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

CGameControllerCarbonRace::CGameControllerCarbonRace(CGameContext *pGameServer) :
	IGameController(pGameServer)
{
	m_pGameType = "CarbonRace";
	m_GameFlags = GAMEFLAG_RACE;
}

void CGameControllerCarbonRace::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameDataRace *pGameData = static_cast<CNetObj_GameDataRace *>(Server()->SnapNewItem(NETOBJTYPE_GAMEDATARACE, 0, sizeof(CNetObj_GameDataRace)));
	if(!pGameData)
		return;

	pGameData->m_BestTime = 0;
	pGameData->m_Precision = 3;
	pGameData->m_RaceFlags = RACEFLAG_KEEP_WANTED_WEAPON;
}

void CGameControllerCarbonRace::OnPlayerExtraSnap(CPlayer *pPlayer, int SnappingClient)
{
	CNetObj_PlayerInfoRace *pRaceInfo = static_cast<CNetObj_PlayerInfoRace *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFORACE, pPlayer->GetCID(), sizeof(CNetObj_PlayerInfoRace)));
	if(!pRaceInfo)
		return;
	pRaceInfo->m_RaceStartTick = -1;
}
