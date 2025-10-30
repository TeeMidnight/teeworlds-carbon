/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */

#include "entity.h"
#include "gamecontext.h"
#include "player.h"

CEntity::CEntity(CGameWorld *pGameWorld, int ObjType, vec2 Pos, int ProximityRadius)
{
	m_pGameWorld = pGameWorld;

	m_pPrevTypeEntity = 0;
	m_pNextTypeEntity = 0;

	m_ID = Server()->SnapNewID();
	m_ObjType = ObjType;

	m_ProximityRadius = ProximityRadius;

	m_MarkedForDestroy = false;
	m_Pos = Pos;
}

CEntity::~CEntity()
{
	GameWorld()->RemoveEntity(this);
	Server()->SnapFreeID(m_ID);
}

int CEntity::NetworkClipped(int SnappingClient)
{
	return NetworkClipped(SnappingClient, m_Pos);
}

int CEntity::NetworkClipped(int SnappingClient, vec2 CheckPos)
{
	return ::NetworkClipped(SnappingClient, CheckPos, GameServer());
}

bool CEntity::GameLayerClipped(vec2 CheckPos)
{
	int rx = round_to_int(CheckPos.x) / 32;
	int ry = round_to_int(CheckPos.y) / 32;
	return (rx < -200 || rx >= GameWorld()->Collision()->GetWidth() + 200) || (ry < -200 || ry >= GameWorld()->Collision()->GetHeight() + 200);
}

COwnerComponent::COwnerComponent(CEntity *pThis)
{
	m_pOwner = nullptr;
	m_pThis = pThis;
}

CHealthComponent::CHealthComponent(CEntity *pThis)
{
	m_pThis = pThis;
}

CHealthComponent::~CHealthComponent()
{
}

void CHealthComponent::Die(CEntity *pKiller, int Weapon)
{
	m_Alive = false;
	m_pThis->GameWorld()->RemoveEntity(m_pThis);
}

bool CHealthComponent::IsFriendlyDamage(CEntity *pFrom)
{
	if(!pFrom)
		return false;
	return true;
}

bool CHealthComponent::TakeDamage(vec2 Force, vec2 Source, int Dmg, CEntity *pFrom, int Weapon)
{
	if(IsFriendlyDamage(pFrom))
		return false;

	if(pFrom == m_pThis)
		Dmg = maximum(1, Dmg / 2);

	if(Dmg)
	{
		if(m_Armor)
		{
			if(Dmg > 1)
			{
				m_Health--;
				Dmg--;
			}

			if(Dmg > m_Armor)
			{
				Dmg -= m_Armor;
				m_Armor = 0;
			}
			else
			{
				m_Armor -= Dmg;
				Dmg = 0;
			}
		}

		m_Health -= Dmg;
	}

	// check for death
	if(m_Health <= 0)
	{
		Die(pFrom, Weapon);
		return false;
	}
	return true;
}
