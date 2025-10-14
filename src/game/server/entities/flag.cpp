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
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>

#include "character.h"
#include "flag.h"

CFlag::CFlag(CGameWorld *pGameWorld, int Team, vec2 StandPos) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG, StandPos, ms_PhysSize)
{
	m_Team = Team;
	m_StandPos = StandPos;

	GameWorld()->InsertEntity(this);

	Reset();
}

void CFlag::Reset()
{
	m_pCarrier = 0;
	m_AtStand = true;
	m_Pos = m_StandPos;
	m_Vel = vec2(0, 0);
	m_GrabTick = 0;
}

void CFlag::Grab(CCharacter *pChar)
{
	m_pCarrier = pChar;
	if(m_AtStand)
		m_GrabTick = Server()->Tick();
	m_AtStand = false;
}

void CFlag::Drop()
{
	m_pCarrier = 0;
	m_Vel = vec2(0, 0);
	m_DropTick = Server()->Tick();
}

void CFlag::TickDefered()
{
	if(m_pCarrier)
	{
		// update flag position
		m_Pos = m_pCarrier->GetPos();
	}
	else
	{
		// flag hits death-tile or left the game layer, reset it
		if((GameWorld()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y) & CCollision::COLFLAG_DEATH) || GameLayerClipped(m_Pos))
		{
			Reset();
			GameWorld()->GameController()->OnFlagReturn(this);
		}

		if(!m_AtStand)
		{
			if(Server()->Tick() > m_DropTick + Server()->TickSpeed() * 30)
			{
				Reset();
				GameWorld()->GameController()->OnFlagReturn(this);
			}
			else
			{
				m_Vel.y += GameWorld()->m_Core.m_Tuning.m_Gravity;
				GameWorld()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(ms_PhysSize, ms_PhysSize), 0.5f);
			}
		}
	}
}

void CFlag::TickPaused()
{
	m_DropTick++;
	if(m_GrabTick)
		m_GrabTick++;
}

void CFlag::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Flag *pFlag = (CNetObj_Flag *) Server()->SnapNewItem(NETOBJTYPE_FLAG, m_Team, sizeof(CNetObj_Flag));
	if(!pFlag)
		return;

	pFlag->m_X = round_to_int(m_Pos.x);
	pFlag->m_Y = round_to_int(m_Pos.y);
	pFlag->m_Team = m_Team;
}
