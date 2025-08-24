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
	int m_aaTypes[MAX_CLIENTS + 1][MAX_EVENTS];
	int m_aaOffsets[MAX_CLIENTS + 1][MAX_EVENTS];
	int m_aaSizes[MAX_CLIENTS + 1][MAX_EVENTS];
	char m_aaData[MAX_CLIENTS + 1][MAX_DATASIZE];

	class CGameContext *m_pGameServer;

	int m_aCurrentOffset[MAX_CLIENTS + 1];
	int m_aNumEvents[MAX_CLIENTS + 1];

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
