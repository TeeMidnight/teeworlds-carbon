/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#include <game/server/botmanager.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include <game/server/gamecontroller.h>

class CGameControllerCarbon : public IGameController
{
public:
	class CQuickRegister
	{
	public:
		CQuickRegister() { GameModeManager()->RegisterGameMode("Carbon", CGameControllerCarbon::CreateController); }
	};

	CGameControllerCarbon(CGameContext *pGameServer);

	bool IsUsingBot() override { return true; }
	static IGameController *CreateController(CGameContext *pGameServer) { return new CGameControllerCarbon(pGameServer); }
};

CGameControllerCarbon::CGameControllerCarbon(CGameContext *pGameServer) :
	IGameController(pGameServer)
{
	m_pGameType = "Carbon";
}

static CGameControllerCarbon::CQuickRegister s_Register;
