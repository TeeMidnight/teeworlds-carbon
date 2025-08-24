/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_EVENTHANDLER_H
#define GAME_SERVER_EVENTHANDLER_H

#include <engine/shared/protocol.h>
//
class CEventHandler
{
	static const int MAX_EVENTS = 32;
	static const int MAX_DATASIZE = 32 * 64;

	// extra 1 is server demo snap.
	int m_aaTypes[SERVER_MAX_CLIENTS][MAX_EVENTS];
	int m_aaOffsets[SERVER_MAX_CLIENTS][MAX_EVENTS];
	int m_aaSizes[SERVER_MAX_CLIENTS][MAX_EVENTS];
	char m_aaData[SERVER_MAX_CLIENTS][MAX_DATASIZE];

	class CGameContext *m_pGameServer;

	int m_aCurrentOffset[SERVER_MAX_CLIENTS];
	int m_aNumEvents[SERVER_MAX_CLIENTS];

	void Create(void *pData, int Type, int Size, int ClientID);

public:
	CGameContext *GameServer() const { return m_pGameServer; }
	void SetGameServer(CGameContext *pGameServer);

	CEventHandler();
	void Create(void *pData, int Type, int Size, int64 Mask = -1);
	void Clear();
	void Snap(int SnappingClient);
};

#endif
