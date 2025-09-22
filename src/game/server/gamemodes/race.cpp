/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#include <engine/server.h>

#include <game/server/botmanager.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "race.h"

enum
{
	TILE_START = 33,
	TILE_FINISH = 34,

	COLFLAG_START = 1 << 3,
	COLFLAG_FINISH = 1 << 4
};

CGameControllerCarbonRace::CGameControllerCarbonRace(CGameContext *pGameServer) :
	IGameController(pGameServer)
{
	m_pGameType = "CarbonRace";
	m_GameFlags = GAMEFLAG_RACE;
}

void CGameControllerCarbonRace::HandleCharacterTiles(CCharacter *pChr, vec2 LastPos, vec2 NewPos)
{
	static const vec2 ColBox(CCharacterCore::PHYS_SIZE, CCharacterCore::PHYS_SIZE);
	if(!pChr)
		return;

	int Flag = pChr->GameWorld()->Collision()->TestBoxMoveAt(LastPos, NewPos, ColBox);
	if(Flag & COLFLAG_START)
	{
	}

	if(Flag & COLFLAG_FINISH)
	{
	}
}

void CGameControllerCarbonRace::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameDataRace *pGameData = static_cast<CNetObj_GameDataRace *>(Server()->SnapNewItem(NETOBJTYPE_GAMEDATARACE, 0, sizeof(CNetObj_GameDataRace)));
	if(!pGameData)
		return;

	pGameData->m_BestTime = -1;
	pGameData->m_Precision = 2;
	pGameData->m_RaceFlags = RACEFLAG_KEEP_WANTED_WEAPON;
}

void CGameControllerCarbonRace::OnPlayerExtraSnap(CPlayer *pPlayer, int SnappingClient)
{
	CNetObj_PlayerInfoRace *pRaceInfo = static_cast<CNetObj_PlayerInfoRace *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFORACE, pPlayer->GetCID(), sizeof(CNetObj_PlayerInfoRace)));
	if(!pRaceInfo)
		return;
	pRaceInfo->m_RaceStartTick = -1;
}

bool CGameControllerCarbonRace::OnExtraTile(CGameWorld *pWorld, int Index, vec2 Pos)
{
	int Flag = -1;
	switch(Index)
	{
	case TILE_START: Flag = COLFLAG_START; break;
	case TILE_FINISH: Flag = COLFLAG_FINISH; break;
	}
	if(Flag == -1)
		return false;
	pWorld->Collision()->SetFlagFor(Pos, Flag);
	return true;
}
