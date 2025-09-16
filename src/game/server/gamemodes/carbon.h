/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef GAME_SERVER_GAMEMODES_CARBON_H
#define GAME_SERVER_GAMEMODES_CARBON_H

#include <game/server/gamecontroller.h>

class CGameControllerCarbon : public IGameController
{
public:
	CGameControllerCarbon(class CGameContext *pGameServer);
};

#endif // GAME_SERVER_GAMEMODES_CARBON_H
