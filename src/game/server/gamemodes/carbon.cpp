/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#include <game/server/botmanager.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "carbon.h"

CGameControllerCarbon::CGameControllerCarbon(CGameContext *pGameServer) :
	IGameController(pGameServer)
{
}
