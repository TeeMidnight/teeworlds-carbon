/*
 * This file is part of NewTeeworldsCN, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef GAME_SERVER_GAMEMODES_CARBONRACE_H
#define GAME_SERVER_GAMEMODES_CARBONRACE_H

#include <game/server/gamecontroller.h>

class CGameControllerCarbonRace : public IGameController
{
public:
	CGameControllerCarbonRace(class CGameContext *pGameServer);

	void Snap(int SnappingClient) override;
	void OnPlayerExtraSnap(class CPlayer *pPlayer, int SnappingClient) override;
	int GetPlayerScore(int ClientID) const override { return -1; }
	bool IsFriendlyFire(class CEntity *pEnt1, class CEntity *pEnt2) const override { return true; }
};

#endif // GAME_SERVER_GAMEMODES_CARBONRACE_H
