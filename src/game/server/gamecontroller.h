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
	int m_RealPlayerNum;

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

	// game
	int m_GameStartTick;
protected:
	class CGameContext *GameServer() const { return m_pGameServer; }
	CConfig *Config() const { return m_pConfig; }
	IServer *Server() const { return m_pServer; }

	// info
	int m_GameFlags;
	const char *m_pGameType;

	virtual void ResetGame(class CGameWorld *pGameWorld);
	virtual void SendGameInfo(int ClientID);

	// team
	int ClampTeam(int Team) const;

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
	virtual bool IsFriendlyTeamFire(class CGameWorld *pWorld, int Team1, int Team2) const;
	virtual bool IsPlayerReadyMode(class CGameWorld *pWorld) const;
	virtual bool IsTeamChangeAllowed(class CGameWorld *pWorld) const;
	virtual bool IsTeamplay() const { return false; }
	virtual bool IsSurvival() const { return false; }

	const char *GetGameType() const { return m_pGameType; }
	unsigned ModeHash() const;

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

	virtual void HandleCharacterTiles(class CCharacter *pChr, vec2 LastPos, vec2 NewPos) {};
	virtual void RegisterChatCommands(CCommandManager *pManager);

	virtual void InitWorldConfig(CWorldConfig *pConfig);
};

class CGameModeManager
{
	typedef IGameController *(*FCreateGameController)(class CGameContext *pGameServer);

	std::unordered_map<unsigned, FCreateGameController> m_upfnCreate;
	std::unordered_map<unsigned, IGameController *> m_upControllers;

public:
	CGameModeManager();
	~CGameModeManager();

	void RegisterGameMode(const char *pGamemode, FCreateGameController pfnCreate);
	void OnInit(class CGameContext *pGameServer);
	void OnPostSnap();
	void OnShutdown();
	void OnTick();
	IGameController *Get(const char *pGamemode);
	IGameController *Get(unsigned ModeHash);
};

extern CGameModeManager *GameModeManager();

template<typename ControllerClass>
class CGameModeRegister
{
public:
    CGameModeRegister(const char *pModeName)
    {
        GameModeManager()->RegisterGameMode(pModeName, CreateController);
    }

	static IGameController *CreateController(CGameContext *pGameServer) { return new ControllerClass(pGameServer); }
};

#endif
