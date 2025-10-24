/*
 * This file is part of NewTeeworldsCN, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef GAME_SERVER_GAMEMODES_VANILLA_H
#define GAME_SERVER_GAMEMODES_VANILLA_H

#include <engine/shared/protocol.h>
#include <game/server/gamecontroller.h>
#include <unordered_map>

// game
enum
{
    TIMER_INFINITE = -1,
    TIMER_END = 10,
};

enum
{
	TBALANCE_CHECK = -2,
	TBALANCE_OK,
};

class IGameControllerVanilla : public IGameController
{
	// activity
	bool GetPlayersReadyState(int WithoutID = -1);
	void SetPlayersReadyState(bool ReadyState);
	void CheckReadyStates(class CGameWorld *pGameWorld, int WithoutID = -1);

	virtual bool CanBeMovedOnBalance(int ClientID) const;
	void CheckTeamBalance();
	void DoTeamBalance();

	// game
	enum EGameState
	{
		// internal game states
		IGS_WARMUP_GAME,		// warmup started by game because there're not enough players (infinite)
		IGS_WARMUP_USER,		// warmup started by user action via rcon or new match (infinite or timer)

		IGS_START_COUNTDOWN,	// start countown to unpause the game or start match/round (tick timer)

		IGS_GAME_PAUSED,		// game paused (infinite or tick timer)
		IGS_GAME_RUNNING,		// game running (infinite)

		IGS_END_MATCH,			// match is over (tick timer)
		IGS_END_ROUND,			// round is over (tick timer)
 	};

	virtual bool DoWincheckMatch();		// returns true when the match is over
	virtual void DoWincheckRound() {}
	bool HasEnoughPlayers() const { return (IsTeamplay() && m_aTeamSize[TEAM_RED] > 0 && m_aTeamSize[TEAM_BLUE] > 0) || (!IsTeamplay() && m_aTeamSize[TEAM_RED] > 1); }
	void SetGameState(class CGameWorld *pGameWorld, EGameState GameState, int Timer=0);
	void StartMatch();
	void StartRound();

protected:
	// info
	struct CGameInfo
	{
		int m_MatchCurrent;
		int m_MatchNum;
		int m_ScoreLimit;
		int m_TimeLimit;
	};
    int m_aPlayerScore[SERVER_MAX_CLIENTS];

    struct SWorldStatus
    {
        // balancing
        int m_aTeamSize[NUM_TEAMS];
        int m_UnbalancedTick;

        EGameState m_GameState;
        int m_GameStateTimer;
        int m_GameStartTick;

        // game
        int m_MatchCount;
        int m_RoundCount;
        int m_SuddenDeath;
        int m_aTeamscore[NUM_TEAMS];

		CGameInfo m_GameInfo;

        SWorldStatus()
        {
            // balancing
            m_aTeamSize[TEAM_RED] = 0;
            m_aTeamSize[TEAM_BLUE] = 0;
            m_UnbalancedTick = TBALANCE_OK;

            // game
            m_GameState = IGS_GAME_RUNNING;
            m_GameStateTimer = TIMER_INFINITE;
            m_GameStartTick = 0;
            m_MatchCount = 0;
            m_RoundCount = 0;
            m_SuddenDeath = 0;
            m_aTeamscore[TEAM_RED] = 0;
            m_aTeamscore[TEAM_BLUE] = 0;
        }
    };
    std::unordered_map<class CGameWorld *, SWorldStatus> m_uWorldStatus;

	void EndMatch(class CGameWorld *pGameWorld) { SetGameState(pGameWorld, IGS_END_MATCH, TIMER_END); }
	void EndRound(class CGameWorld *pGameWorld) { SetGameState(pGameWorld, IGS_END_ROUND, TIMER_END/2); }

	virtual void SendGameInfo(int ClientID);
	virtual void ResetGame(class CGameWorld *pGameWorld);

    const SWorldStatus *GetWorldStatus(class CGameWorld *pGameWorld) const
    {
		auto IterStatus = m_uWorldStatus.find(pGameWorld);
		if(IterStatus == m_uWorldStatus.end())
			return nullptr;
        return &IterStatus->second;
    }

    SWorldStatus *GetWorldStatus(class CGameWorld *pGameWorld)
    {
		auto IterStatus = m_uWorldStatus.find(pGameWorld);
		if(IterStatus == m_uWorldStatus.end())
			return nullptr;
        return &IterStatus->second;
    }

public:
	IGameControllerVanilla(class CGameContext *pGameServer);
	virtual ~IGameControllerVanilla() {}

	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);

	virtual void OnFlagReturn(class CFlag *pFlag);

	virtual void OnPlayerConnect(class CPlayer *pPlayer);
	virtual void OnPlayerDisconnect(class CPlayer *pPlayer);
	virtual void OnPlayerInfoChange(class CPlayer *pPlayer);
	virtual void OnPlayerReadyChange(class CPlayer *pPlayer);

	virtual void OnReset();

	virtual void DoPause(class CGameWorld *pGameWorld, int Seconds) { SetGameState(pGameWorld, IGS_GAME_PAUSED, Seconds); }
	virtual void DoWarmup(class CGameWorld *pGameWorld, int Seconds)
	{
		SetGameState(pGameWorld, IGS_WARMUP_USER, Seconds);
	}
	virtual void AbortWarmup(class CGameWorld *pGameWorld)
	{
        SWorldStatus *pStatus = GetWorldStatus(pGameWorld);
        if(!pStatus)
            return;
		if((pStatus->m_GameState == IGS_WARMUP_GAME || pStatus->m_GameState == IGS_WARMUP_USER)
			&& pStatus->m_GameStateTimer != TIMER_INFINITE)
		{
			SetGameState(pGameWorld, IGS_GAME_RUNNING);
		}
	}
	virtual void SwapTeamscore();

	// general
	virtual void Snap(int SnappingClient);
	virtual void Tick();

	// info
	virtual void CheckGameInfo();
	virtual bool IsFriendlyFire(int ClientID1, int ClientID2) const;
	virtual bool IsFriendlyTeamFire(int Team1, int Team2) const;
	virtual bool IsGamePaused(class CGameWorld *pGameWorld) const
    {
        const SWorldStatus *pStatus = GetWorldStatus(pGameWorld);
        if(!pStatus)
            return false;
        return pStatus->m_GameState == IGS_GAME_PAUSED || pStatus->m_GameState == IGS_START_COUNTDOWN;
    }
	virtual bool IsGameRunning(class CGameWorld *pGameWorld) const
    {
        const SWorldStatus *pStatus = GetWorldStatus(pGameWorld);
        if(!pStatus)
            return false;
        return pStatus->m_GameState == IGS_GAME_RUNNING;
    }
	virtual bool IsPlayerReadyMode(class CGameWorld *pWorld) const;
	virtual bool IsTeamChangeAllowed(class CGameWorld *pWorld) const;
	virtual bool IsTeamplay() const { return m_GameFlags&GAMEFLAG_TEAMS; }
	virtual bool IsSurvival() const { return m_GameFlags&GAMEFLAG_SURVIVAL; }

	virtual const char *GetGameType() const { return m_pGameType; }
	virtual int GetPlayerScore(int ClientID) const { return m_aPlayerScore[ClientID]; }

	//spawn
	virtual bool GetStartRespawnState() const;

	// team
	virtual bool CanJoinTeam(int Team, int NotThisID) const;
	virtual bool CanChangeTeam(CPlayer *pPplayer, int JoinTeam) const;

	virtual void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg=true);
	virtual void ForceTeamBalance() { if(!(m_GameFlags&GAMEFLAG_SURVIVAL)) DoTeamBalance(); }

	virtual int GetStartTeam();

	virtual void InitWorldConfig(CWorldConfig *pConfig);
};

#endif
