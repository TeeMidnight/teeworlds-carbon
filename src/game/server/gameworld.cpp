/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */

#include <generated/server_data.h>

#include "botmanager.h"
#include "entities/character.h"
#include "entity.h"
#include "gamecontext.h"
#include "gamecontroller.h"
#include "gameworld.h"
#include "player.h"

CEventHandler *CGameWorld::EventHandler() { return &GameServer()->m_Events; }
//////////////////////////////////////////////////
// game world
//////////////////////////////////////////////////
CGameWorld::CGameWorld()
{
	m_pGameServer = nullptr;
	m_pConfig = nullptr;
	m_pServer = nullptr;

	// spawn
	m_aNumSpawnPoints[0] = 0;
	m_aNumSpawnPoints[1] = 0;
	m_aNumSpawnPoints[2] = 0;

	m_Paused = false;
	m_ResetRequested = false;
	for(int i = 0; i < NUM_ENTTYPES; i++)
		m_apFirstEntityTypes[i] = nullptr;

	m_pBotManager = nullptr;
}

CGameWorld::~CGameWorld()
{
	// delete all entities
	for(int i = 0; i < NUM_ENTTYPES; i++)
		while(m_apFirstEntityTypes[i])
			delete m_apFirstEntityTypes[i];

	if(m_pBotManager)
		delete m_pBotManager;
}

void CGameWorld::SetCollision(std::shared_ptr<CCollision> pCollision)
{
	m_pCollision = pCollision;
}

void CGameWorld::SetGameServer(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
	m_pConfig = m_pGameServer->Config();
	m_pServer = m_pGameServer->Server();
}

void CGameWorld::SetGameController(IGameController *pGameController)
{
	m_pGameController = pGameController;
	if(m_pBotManager)
		delete m_pBotManager;
	m_pBotManager = nullptr;
	if(pGameController->IsUsingBot())
	{
		m_pBotManager = new CBotManager(this);
	}
}

CEntity *CGameWorld::FindFirst(int Type)
{
	return Type < 0 || Type >= NUM_ENTTYPES ? 0 : m_apFirstEntityTypes[Type];
}

int CGameWorld::FindEntities(vec2 Pos, float Radius, CEntity **ppEnts, int Max, int Type)
{
	if(Type < 0 || Type >= NUM_ENTTYPES)
		return 0;

	int Num = 0;
	for(CEntity *pEnt = m_apFirstEntityTypes[Type]; pEnt; pEnt = pEnt->m_pNextTypeEntity)
	{
		if(distance(pEnt->m_Pos, Pos) < Radius + pEnt->m_ProximityRadius)
		{
			if(ppEnts)
				ppEnts[Num] = pEnt;
			Num++;
			if(Num == Max)
				break;
		}
	}

	return Num;
}

int CGameWorld::FindEntities(vec2 Pos, float Radius, CEntity **ppEnts, int Max, EEntityFlag Flag)
{
	int Num = 0;
	for(int i = 0; i < NUM_ENTTYPES; i++)
	{
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; pEnt = pEnt->TypeNext())
		{
			if((pEnt->GetObjFlag() & Flag) && distance(pEnt->m_Pos, Pos) < Radius + pEnt->m_ProximityRadius)
			{
				if(ppEnts)
					ppEnts[Num] = pEnt;
				Num++;
				if(Num == Max)
					break;
			}
		}
	}

	return Num;
}

void CGameWorld::InsertEntity(CEntity *pEnt)
{
#ifdef CONF_DEBUG
	for(CEntity *pCur = m_apFirstEntityTypes[pEnt->m_ObjType]; pCur; pCur = pCur->m_pNextTypeEntity)
		dbg_assert(pCur != pEnt, "err");
#endif

	// insert it
	if(m_apFirstEntityTypes[pEnt->m_ObjType])
		m_apFirstEntityTypes[pEnt->m_ObjType]->m_pPrevTypeEntity = pEnt;
	pEnt->m_pNextTypeEntity = m_apFirstEntityTypes[pEnt->m_ObjType];
	pEnt->m_pPrevTypeEntity = 0x0;
	m_apFirstEntityTypes[pEnt->m_ObjType] = pEnt;
}

void CGameWorld::DestroyEntity(CEntity *pEnt)
{
	pEnt->MarkForDestroy();
}

void CGameWorld::RemoveEntity(CEntity *pEnt)
{
	// not in the list
	if(!pEnt->m_pNextTypeEntity && !pEnt->m_pPrevTypeEntity && m_apFirstEntityTypes[pEnt->m_ObjType] != pEnt)
		return;

	// remove
	if(pEnt->m_pPrevTypeEntity)
		pEnt->m_pPrevTypeEntity->m_pNextTypeEntity = pEnt->m_pNextTypeEntity;
	else
		m_apFirstEntityTypes[pEnt->m_ObjType] = pEnt->m_pNextTypeEntity;
	if(pEnt->m_pNextTypeEntity)
		pEnt->m_pNextTypeEntity->m_pPrevTypeEntity = pEnt->m_pPrevTypeEntity;

	// keep list traversing valid
	if(m_pNextTraverseEntity == pEnt)
		m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;

	pEnt->m_pNextTypeEntity = 0;
	pEnt->m_pPrevTypeEntity = 0;
}

//
void CGameWorld::Snap(int SnappingClient)
{
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt;)
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			pEnt->Snap(SnappingClient);
			pEnt = m_pNextTraverseEntity;
		}
}

void CGameWorld::PostSnap()
{
	if(BotManager())
		BotManager()->PostSnap();

	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt;)
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			pEnt->PostSnap();
			pEnt = m_pNextTraverseEntity;
		}
}

void CGameWorld::Reset()
{
	// reset all entities
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt;)
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			pEnt->Reset();
			pEnt = m_pNextTraverseEntity;
		}
	RemoveEntities();

	GameController()->OnReset();
	RemoveEntities();

	m_ResetRequested = false;
}

void CGameWorld::RemoveEntities()
{
	// destroy objects marked for destruction
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt;)
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			if(pEnt->IsMarkedForDestroy())
			{
				RemoveEntity(pEnt);
				pEnt->Destroy();
			}
			pEnt = m_pNextTraverseEntity;
		}
}

void CGameWorld::Tick()
{
	if(m_ResetRequested)
		Reset();

	if(BotManager())
		BotManager()->Tick();

	if(m_Paused)
	{
		// update all objects
		for(int i = 0; i < NUM_ENTTYPES; i++)
			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt;)
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->TickPaused();
				pEnt = m_pNextTraverseEntity;
			}
	}
	else
	{
		// update all objects
		for(int i = 0; i < NUM_ENTTYPES; i++)
			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt;)
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->Tick();
				pEnt = m_pNextTraverseEntity;
			}

		for(int i = 0; i < NUM_ENTTYPES; i++)
			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt;)
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->TickDefered();
				pEnt = m_pNextTraverseEntity;
			}
	}

	RemoveEntities();
}

// TODO: should be more general
CEntity *CGameWorld::IntersectEntity(vec2 Pos0, vec2 Pos1, float Radius, int Type, vec2 &NewPos, CEntity *pNotThis)
{
	// Find other players
	float ClosestLen = distance(Pos0, Pos1) * 100.0f;
	CEntity *pClosest = 0;

	CEntity *p = (CEntity *) FindFirst(Type);
	for(; p; p = (CEntity *) p->TypeNext())
	{
		if(p == pNotThis)
			continue;

		vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, p->m_Pos);
		float Len = distance(p->m_Pos, IntersectPos);
		if(Len < p->m_ProximityRadius + Radius)
		{
			Len = distance(Pos0, IntersectPos);
			if(Len < ClosestLen)
			{
				NewPos = IntersectPos;
				ClosestLen = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}

CEntity *CGameWorld::IntersectEntity(vec2 Pos0, vec2 Pos1, float Radius, EEntityFlag Flag, vec2 &NewPos, CEntity *pNotThis)
{
	// Find other players
	float ClosestLen = distance(Pos0, Pos1) * 100.0f;
	CEntity *pClosest = 0;

	for(int i = 0; i < NUM_ENTTYPES; i++)
	{
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; pEnt = pEnt->TypeNext())
		{
			if(!(pEnt->GetObjFlag() & Flag) || pEnt == pNotThis)
				continue;

			vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, pEnt->m_Pos);
			float Len = distance(pEnt->m_Pos, IntersectPos);
			if(Len < pEnt->m_ProximityRadius + Radius)
			{
				Len = distance(Pos0, IntersectPos);
				if(Len < ClosestLen)
				{
					NewPos = IntersectPos;
					ClosestLen = Len;
					pClosest = pEnt;
				}
			}
		}
	}

	return pClosest;
}

CEntity *CGameWorld::ClosestEntity(vec2 Pos, float Radius, int Type, CEntity *pNotThis)
{
	// Find other players
	float ClosestRange = Radius * 2;
	CEntity *pClosest = 0;

	CEntity *p = FindFirst(Type);
	for(; p; p = p->TypeNext())
	{
		if(p == pNotThis)
			continue;

		float Len = distance(Pos, p->m_Pos);
		if(Len < p->m_ProximityRadius + Radius)
		{
			if(Len < ClosestRange)
			{
				ClosestRange = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}

CEntity *CGameWorld::ClosestEntity(vec2 Pos, float Radius, EEntityFlag Flag, CEntity *pNotThis)
{
	// Find other players
	float ClosestRange = Radius * 2;
	CEntity *pClosest = 0;

	for(int i = 0; i < NUM_ENTTYPES; i++)
	{
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; pEnt = pEnt->TypeNext())
		{
			if(!(pEnt->GetObjFlag() & Flag) || pEnt == pNotThis)
				continue;

			float Len = distance(Pos, pEnt->m_Pos);
			if(Len < pEnt->m_ProximityRadius + Radius)
			{
				if(Len < ClosestRange)
				{
					ClosestRange = Len;
					pClosest = pEnt;
				}
			}
		}
	}

	return pClosest;
}

int64_t CGameWorld::CmaskAllInWorld()
{
	int64_t Mask = 0LL;
	for(auto &pPlayer : GameServer()->m_apPlayers)
	{
		if(pPlayer && pPlayer->GameWorld() == this)
		{
			Mask |= CmaskOne(pPlayer->GetCID());
		}
	}
	return Mask;
}

int64_t CGameWorld::CmaskAllInWorldExceptOne(int ClientID)
{
	return CmaskAllInWorld() ^ CmaskOne(ClientID);
}

void CGameWorld::CreateDamage(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self, int64_t Mask)
{
	float f = angle(Source);
	CNetEvent_Damage Event;
	Event.m_X = (int) Pos.x;
	Event.m_Y = (int) Pos.y;
	Event.m_ClientID = Id;
	Event.m_Angle = (int) (f * 256.0f);
	Event.m_HealthAmount = HealthAmount;
	Event.m_ArmorAmount = ArmorAmount;
	Event.m_Self = Self;
	EventHandler()->Create(&Event, NETEVENTTYPE_DAMAGE, sizeof(CNetEvent_Damage), Mask);
}

void CGameWorld::CreateHammerHit(vec2 Pos, int64_t Mask)
{
	// create the event
	CNetEvent_HammerHit Event;
	Event.m_X = (int) Pos.x;
	Event.m_Y = (int) Pos.y;
	EventHandler()->Create(&Event, NETEVENTTYPE_HAMMERHIT, sizeof(CNetEvent_HammerHit), Mask);
}

void CGameWorld::CreateExplosion(vec2 Pos, CEntity *pFrom, int Weapon, int MaxDamage, int64_t Mask)
{
	// create the event
	CNetEvent_Explosion Event;
	Event.m_X = (int) Pos.x;
	Event.m_Y = (int) Pos.y;
	EventHandler()->Create(&Event, NETEVENTTYPE_EXPLOSION, sizeof(CNetEvent_Explosion), Mask);

	// deal damage
	CBaseHealthEntity *apEnts[MAX_CHECK_ENTITY];
	float Radius = g_pData->m_Explosion.m_Radius;
	float InnerRadius = 48.0f;
	float MaxForce = g_pData->m_Explosion.m_MaxForce;
	int Num = FindEntities(Pos, Radius, (CEntity **) apEnts, MAX_CHECK_ENTITY, EEntityFlag::ENTFLAG_DAMAGE);
	for(int i = 0; i < Num; i++)
	{
		vec2 Diff = apEnts[i]->GetPos() - Pos;
		vec2 Force(0, MaxForce);
		float l = length(Diff);
		if(l)
			Force = normalize(Diff) * MaxForce;
		float Factor = 1 - clamp((l - InnerRadius) / (Radius - InnerRadius), 0.0f, 1.0f);
		if((int) (Factor * MaxDamage))
			apEnts[i]->TakeDamage(Force * Factor, Diff * -1, (int) (Factor * MaxDamage), pFrom, Weapon);
	}
}

void CGameWorld::CreatePlayerSpawn(vec2 Pos, int64_t Mask)
{
	CNetEvent_Spawn Event;
	Event.m_X = (int) Pos.x;
	Event.m_Y = (int) Pos.y;
	EventHandler()->Create(&Event, NETEVENTTYPE_SPAWN, sizeof(CNetEvent_Spawn), Mask);
}

void CGameWorld::CreateDeath(vec2 Pos, int ClientID, int64_t Mask)
{
	CNetEvent_Death Event;
	Event.m_X = (int) Pos.x;
	Event.m_Y = (int) Pos.y;
	Event.m_ClientID = ClientID;
	EventHandler()->Create(&Event, NETEVENTTYPE_DEATH, sizeof(CNetEvent_Death), Mask);
}

void CGameWorld::CreateSound(vec2 Pos, int Sound, int64_t Mask)
{
	if(Sound < 0)
		return;
	CNetEvent_SoundWorld Event;
	Event.m_X = (int) Pos.x;
	Event.m_Y = (int) Pos.y;
	Event.m_SoundID = Sound;
	EventHandler()->Create(&Event, NETEVENTTYPE_SOUNDWORLD, sizeof(CNetEvent_SoundWorld), Mask);
}

void CGameWorld::CreateDamage(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self)
{
	CreateDamage(Pos, Id, Source, HealthAmount, ArmorAmount, Self, CmaskAllInWorld());
}

void CGameWorld::CreateExplosion(vec2 Pos, CEntity *pFrom, int Weapon, int MaxDamage)
{
	CreateExplosion(Pos, pFrom, Weapon, MaxDamage, CmaskAllInWorld());
}

void CGameWorld::CreateHammerHit(vec2 Pos)
{
	CreateHammerHit(Pos, CmaskAllInWorld());
}

void CGameWorld::CreatePlayerSpawn(vec2 Pos)
{
	CreatePlayerSpawn(Pos, CmaskAllInWorld());
}

void CGameWorld::CreateDeath(vec2 Pos, int Who)
{
	CreateDeath(Pos, Who, CmaskAllInWorld());
}

void CGameWorld::CreateSound(vec2 Pos, int Sound)
{
	CreateSound(Pos, Sound, CmaskAllInWorld());
}
