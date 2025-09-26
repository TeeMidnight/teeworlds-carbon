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
#ifndef ENGINE_SERVER_H
#define ENGINE_SERVER_H

#include <base/types.h>

#include "kernel.h"
#include "message.h"

class IServer : public IInterface
{
	MACRO_INTERFACE("server", 0)
protected:
	int m_CurrentGameTick;
	int m_TickSpeed;

public:
	/*
		Structure: CClientInfo
	*/
	struct CClientInfo
	{
		const char *m_pName;
		int m_Latency;
	};

	int Tick() const { return m_CurrentGameTick; }
	int TickSpeed() const { return m_TickSpeed; }

	virtual const char *ClientLanguage(int ClientID) const = 0;
	virtual const char *ClientName(int ClientID) const = 0;
	virtual const char *ClientClan(int ClientID) const = 0;
	virtual int ClientCountry(int ClientID) const = 0;
	virtual int ClientScore(int ClientID) const = 0;
	virtual bool ClientIngame(int ClientID) const = 0;
	virtual Uuid GetClientMapID(int ClientID) const = 0;
	virtual int GetClientInfo(int ClientID, CClientInfo *pInfo) const = 0;
	virtual void GetClientAddr(int ClientID, char *pAddrStr, int Size) const = 0;
	virtual int GetClientVersion(int ClientID) const = 0;
	virtual int GetCarbonClientVersion(int ClientID) const = 0;
	virtual int GetDDNetClientVersion(int ClientID) const = 0;

	virtual int SendMsg(CMsgPacker *pMsg, int Flags, int ClientID) = 0;

	template<class T>
	int SendPackMsg(T *pMsg, int Flags, int ClientID)
	{
		CMsgPacker Packer(pMsg->MsgID(), false);
		if(pMsg->Pack(&Packer))
			return -1;
		return SendMsg(&Packer, Flags, ClientID);
	}

	virtual void SetClientLanguage(int ClientID, char const *pLanguage) = 0;
	virtual void SetClientName(int ClientID, char const *pName) = 0;
	virtual void SetClientClan(int ClientID, char const *pClan) = 0;
	virtual void SetClientCountry(int ClientID, int Country) = 0;
	virtual void SetClientScore(int ClientID, int Score) = 0;

	virtual int SnapNewID() = 0;
	virtual void SnapFreeID(int ID) = 0;
	virtual void *SnapNewItem(int Type, int ID, int Size) = 0;

	virtual void SnapSetStaticsize(int ItemType, int Size) = 0;

	enum
	{
		RCON_CID_SERV = -1,
		RCON_CID_VOTE = -2,
	};
	virtual void SetRconCID(int ClientID) = 0;
	virtual bool IsAuthed(int ClientID) const = 0;
	virtual bool IsBanned(int ClientID) = 0;
	virtual void Kick(int ClientID, const char *pReason) = 0;

	virtual void DemoRecorder_HandleAutoStart() = 0;
	virtual bool DemoRecorder_IsRecording() = 0;

	virtual void ExpireServerInfo() = 0;

	virtual const char *Localize(const char *pCode, const char *pStr, const char *pContext = "") GNUC_ATTRIBUTE((format_arg(3))) = 0;
	virtual const char *Localize(int ClientID, const char *pStr, const char *pContext = "") GNUC_ATTRIBUTE((format_arg(3))) = 0;

	virtual int GetLanguagesInfo(struct SLanguageInfo **ppInfo) = 0;

	virtual void SwitchClientMap(int ClientID, Uuid MapID) = 0;
	virtual void RequestNewMap(int ClientID, const char *pMapName, unsigned ModeID) = 0;

	virtual Uuid GetBaseMapUuid() const = 0;
	virtual const char *GetMapName(Uuid MapID) = 0;
	virtual unsigned GetMapModeID(Uuid MapID) = 0;
};

class IGameServer : public IInterface
{
	MACRO_INTERFACE("gameserver", 0)
protected:
public:
	virtual void OnInit() = 0;
	virtual void OnConsoleInit() = 0;
	virtual void OnShutdown() = 0;

	virtual void OnTick() = 0;
	virtual void OnPreSnap() = 0;
	virtual void OnSnap(int ClientID) = 0;
	virtual void OnPostSnap() = 0;

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID) = 0;

	virtual void OnClientConnected(int ClientID, bool AsSpec) = 0;
	virtual void OnClientEnter(int ClientID) = 0;
	virtual void OnClientDrop(int ClientID, const char *pReason) = 0;
	virtual void OnClientDirectInput(int ClientID, void *pInput) = 0;
	virtual void OnClientPredictedInput(int ClientID, void *pInput) = 0;

	virtual bool IsClientBot(int ClientID) const = 0;
	virtual bool IsClientReady(int ClientID) const = 0;
	virtual bool IsClientPlayer(int ClientID) const = 0;
	virtual bool IsClientSpectator(int ClientID) const = 0;

	virtual const char *GameType() const = 0;
	virtual const char *Version() const = 0;
	virtual const char *NetVersion() const = 0;
	virtual const char *NetVersionHashUsed() const = 0;
	virtual const char *NetVersionHashReal() const = 0;

	virtual bool TimeScore() const { return false; }
	/**
	 * Used to report custom player info to master servers.
	 *
	 * @param pJsonWriter A pointer to a CJsonStringWriter which the custom data will be added to.
	 * @param i The client id.
	 */
	virtual void OnUpdatePlayerServerInfo(class CJsonStringWriter *pJSonWriter, int Id) = 0;

	virtual bool CheckWorldExists(Uuid WorldID) = 0;
	virtual void LoadNewWorld(Uuid WorldID) = 0;
	virtual void SwitchPlayerWorld(int ClientID, Uuid WorldID) = 0;
};

extern IGameServer *CreateGameServer();
#endif
