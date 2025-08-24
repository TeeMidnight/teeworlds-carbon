/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMECONTEXT_H
#define GAME_SERVER_GAMECONTEXT_H

#include <engine/console.h>
#include <engine/server.h>

#include <game/commands.h>
#include <game/layers.h>
#include <game/voting.h>

#include "eventhandler.h"
#include "gamemenu.h"
#include "gameworld.h"

#include <vector>
/*
	Tick
		Game Context (CGameContext::tick)
			Game World (GAMEWORLD::tick)
				Reset world if requested (GAMEWORLD::reset)
				All entities in the world (ENTITY::tick)
				All entities in the world (ENTITY::tick_defered)
				Remove entities marked for deletion (GAMEWORLD::remove_entities)
			Game Controller (GAMECONTROLLER::tick)
			All players (CPlayer::tick)


	Snap
		Game Context (CGameContext::snap)
			Game World (GAMEWORLD::snap)
				All entities in the world (ENTITY::snap)
			Game Controller (GAMECONTROLLER::snap)
			Events handler (EVENT_HANDLER::snap)
			All players (CPlayer::snap)

*/
class CGameContext : public IGameServer
{
	IServer *m_pServer;
	class CConfig *m_pConfig;
	class IConsole *m_pConsole;
	class IStorage *m_pStorage;
	CLayers m_Layers;
	CCollision m_Collision;
	CNetObjHandler m_NetObjHandler;
	CTuningParams m_Tuning;

	class CBotManager *m_pBotManager;
	class CGameController *m_pController;

	CGameMenu *m_pGameMenu;

	static void ConTuneParam(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneReset(IConsole::IResult *pResult, void *pUserData);
	static void ConTunes(IConsole::IResult *pResult, void *pUserData);
	static void ConSay(IConsole::IResult *pResult, void *pUserData);
	static void ConBroadcast(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTeamAll(IConsole::IResult *pResult, void *pUserData);
	static void ConAddVote(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveVote(IConsole::IResult *pResult, void *pUserData);
	static void ConClearVotes(IConsole::IResult *pResult, void *pUserData);
	static void ConVote(IConsole::IResult *pResult, void *pUserData);
	static void ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainSettingUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	static void NewCommandHook(const CCommandManager::CCommand *pCommand, void *pContext);
	static void RemoveCommandHook(const CCommandManager::CCommand *pCommand, void *pContext);

	static bool MenuServerVote(int ClientID, SCallVoteStatus &VoteStatus, class CGameMenu *pMenu, void *pUserData);

	CGameContext(int Resetting);
	void Construct(int Resetting);

	bool m_Resetting;

public:
	IServer *Server() const { return m_pServer; }
	class CConfig *Config() const { return m_pConfig; }
	class IConsole *Console() const { return m_pConsole; }
	class IStorage *Storage() const { return m_pStorage; }
	CCollision *Collision() { return &m_Collision; }
	CTuningParams *Tuning() { return &m_Tuning; }

	class CBotManager *BotManager() const { return m_pBotManager; }
	class CGameController *GameController() const { return m_pController; }
	CGameMenu *GameMenu() const { return m_pGameMenu; }

	CGameContext();
	~CGameContext();

	void Clear();

	CEventHandler m_Events;
	class CPlayer *m_apPlayers[SERVER_MAX_CLIENTS];

	CGameWorld m_World;
	CCommandManager m_CommandManager;

	CCommandManager *CommandManager() { return &m_CommandManager; }

	// helper functions
	class CCharacter *GetPlayerChar(int ClientID);

	// voting
	void StartVote(const char *pDesc, const char *pCommand, const char *pReason);
	void EndVote(int Type, bool Force);
	void AbortVoteOnDisconnect(int ClientID);
	void AbortVoteOnTeamChange(int ClientID);

	int m_VoteCreator;
	int m_VoteType;
	int64 m_VoteCloseTime;
	int64 m_VoteCancelTime;
	bool m_VoteUpdate;
	int m_VotePos;
	char m_aVoteDescription[VOTE_DESC_LENGTH];
	char m_aVoteCommand[VOTE_CMD_LENGTH];
	char m_aVoteReason[VOTE_REASON_LENGTH];
	int m_VoteClientID;
	int m_NumVoteOptions;
	int m_VoteEnforce;
	enum
	{
		VOTE_TIME = 25,
		VOTE_CANCEL_TIME = 10,

		MIN_SKINCHANGE_CLIENTVERSION = 0x0703,
		MIN_RACE_CLIENTVERSION = 0x0704,
	};
	class CHeap *m_pVoteOptionHeap;
	CVoteOptionServer *m_pVoteOptionFirst;
	CVoteOptionServer *m_pVoteOptionLast;

	// helper functions
	void CreateDamage(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self, int64 Mask = -1LL);
	void CreateExplosion(vec2 Pos, CEntity *pFrom, int Weapon, int MaxDamage, int64 Mask = -1LL);
	void CreateHammerHit(vec2 Pos, int64 Mask = -1LL);
	void CreatePlayerSpawn(vec2 Pos, int64 Mask = -1LL);
	void CreateDeath(vec2 Pos, int Who, int64 Mask = -1LL);
	void CreateSound(vec2 Pos, int Sound, int64 Mask = -1LL);

	// ----- send functions -----
	void SendChat(int ChatterClientID, int Mode, int To, const char *pText);
	void SendBroadcast(const char *pText, int ClientID);
	void SendEmoticon(int ClientID, int Emoticon);
	void SendWeaponPickup(int ClientID, int Weapon);
	void SendMotd(int ClientID);
	void SendSettings(int ClientID);
	void SendSkinChange(int ClientID, int TargetID);
	void SendTuningParams(int ClientID);
	void SendSoundTarget(int ClientID, int SoundID);
	void SendReadyToEnter(CPlayer *pPlayer);

	void SendGameMsg(int GameMsgID, int ClientID);
	void SendGameMsg(int GameMsgID, int ParaI1, int ClientID);
	void SendGameMsg(int GameMsgID, int ParaI1, int ParaI2, int ParaI3, int ClientID);

	void SendChatCommand(const CCommandManager::CCommand *pCommand, int ClientID);
	void SendChatCommands(int ClientID);
	void SendRemoveChatCommand(const CCommandManager::CCommand *pCommand, int ClientID);

	void SendForceVote(int Type, const char *pDescription, const char *pReason);
	void SendVoteSet(int Type, int ToClientID);
	void SendVoteStatus(int ClientID, int Total, int Yes, int No);
	void SendVoteClearOptions(int ClientID);

	void OnGameMenuInit();

	// engine events
	void OnInit() override;
	void OnConsoleInit() override;
	void OnShutdown() override;

	void OnTick() override;
	void OnPreSnap() override;
	void OnSnap(int ClientID) override;
	void OnPostSnap() override;

	void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID) override;

	void OnClientConnected(int ClientID, bool AsSpec) override { OnClientConnected(ClientID, false, AsSpec); }
	void OnClientConnected(int ClientID, bool Dummy, bool AsSpec);
	void OnClientTeamChange(int ClientID);
	void OnClientEnter(int ClientID) override;
	void OnClientDrop(int ClientID, const char *pReason) override;
	void OnClientDirectInput(int ClientID, void *pInput) override;
	void OnClientPredictedInput(int ClientID, void *pInput) override;

	bool IsClientBot(int ClientID) const override;
	bool IsClientReady(int ClientID) const override;
	bool IsClientPlayer(int ClientID) const override;
	bool IsClientSpectator(int ClientID) const override;

	const char *GameType() const override;
	const char *Version() const override;
	const char *NetVersion() const override;
	const char *NetVersionHashUsed() const override;
	const char *NetVersionHashReal() const override;

	void OnUpdatePlayerServerInfo(class CJsonStringWriter *pJSonWriter, int Id) override;
};

inline int64 CmaskAll() { return -1; }
inline int64 CmaskOne(int ClientID) { return (int64) 1 << ClientID; }
inline int64 CmaskAllExceptOne(int ClientID) { return CmaskAll() ^ CmaskOne(ClientID); }
inline bool CmaskIsSet(int64 Mask, int ClientID) { return (Mask & CmaskOne(ClientID)) != 0; }

int NetworkClipped(int SnappingClient, vec2 CheckPos, CGameContext *pGameServer);

#endif // GAME_SERVER_GAMECONTEXT_H
