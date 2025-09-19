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
#ifndef GAME_SERVER_GAMECONTROLLER_H
#define GAME_SERVER_GAMECONTROLLER_H

#include <base/tl/array.h>
#include <base/vmath.h>

#include <game/commands.h>

#include <generated/protocol.h>

/*
	Class: Game Controller
		Controls the main game logic. Keeping track of team and player score,
		winning conditions and specific game logic.
*/
class IGameController
{
	friend class CBotManager;

	class CGameContext *m_pGameServer;
	class CConfig *m_pConfig;
	class IServer *m_pServer;

	// activity
	void DoActivityCheck();
	bool GetPlayersReadyState(int WithoutID = -1);
	void SetPlayersReadyState(bool ReadyState);

	// balancing
	enum
	{
		TBALANCE_CHECK = -2,
		TBALANCE_OK,
	};
	int m_RealPlayerNum;
	void ResetGame();

	// spawn
	struct CSpawnEval
	{
		CSpawnEval()
		{
			m_Got = false;
			m_FriendlyTeam = -1;
			m_Pos = vec2(100, 100);
		}

		class CGameWorld *m_pWorld;
		vec2 m_Pos;
		bool m_Got;
		bool m_RandomSpawn;
		int m_FriendlyTeam;
		float m_Score;
	};

	float EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos) const;
	void EvaluateSpawnType(CSpawnEval *pEval, int Type) const;

	// team
	int ClampTeam(int Team) const;

protected:
	class CGameContext *GameServer() const { return m_pGameServer; }
	CConfig *Config() const { return m_pConfig; }
	IServer *Server() const { return m_pServer; }

	// game
	int m_GameStartTick;

	// info
	int m_GameFlags;
	const char *m_pGameType;

	void SendGameInfo(int ClientID);

public:
	virtual bool IsUsingBot() { return false; }

	IGameController(class CGameContext *pGameServer);
	virtual ~IGameController();

	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void OnCharacterSpawn(class CCharacter *pChr);
	virtual void OnFlagReturn(class CFlag *pFlag);
	virtual bool OnEntity(class CGameWorld *pGameWorld, int Index, vec2 Pos);
	virtual bool OnExtraTile(class CGameWorld *pGameWorld, int Index, vec2 Pos);

	virtual void OnPlayerConnect(class CPlayer *pPlayer);
	virtual void OnPlayerDisconnect(class CPlayer *pPlayer);
	virtual void OnPlayerInfoChange(class CPlayer *pPlayer);
	virtual void OnPlayerReadyChange(class CPlayer *pPlayer);

	virtual void OnPlayerSendEmoticon(int Emoticon, class CPlayer *pPlayer);

	virtual void OnReset();

	// general
	virtual void Snap(int SnappingClient);
	virtual void PostSnap();
	virtual void Tick();

	// info
	virtual bool IsFriendlyFire(class CEntity *pEnt1, class CEntity *pEnt2) const;
	virtual bool IsFriendlyTeamFire(int Team1, int Team2) const;
	virtual bool IsPlayerReadyMode() const;
	virtual bool IsTeamChangeAllowed() const;
	virtual bool IsTeamplay() const { return false; }
	virtual bool IsSurvival() const { return false; }

	const char *GetGameType() const { return m_pGameType; }

	// spawn
	virtual bool CanSpawn(class CGameWorld *pWorld, int Team, vec2 *pPos) const;
	virtual bool GetStartRespawnState() const;

	// team
	virtual bool CanJoinTeam(int Team, int NotThisID) const;
	virtual bool CanChangeTeam(CPlayer *pPplayer, int JoinTeam) const;

	virtual void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg = true);

	int GetRealPlayerNum() const { return m_RealPlayerNum; }
	virtual int GetStartTeam();
	virtual void OnPlayerExtraSnap(class CPlayer *pPlayer, int SnappingClient) {}
	virtual int GetPlayerScore(int ClientID) const { return 0; }

	// static void Com_Example(IConsole::IResult *pResult, void *pContext);
	virtual void RegisterChatCommands(CCommandManager *pManager);
};

#endif
