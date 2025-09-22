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
#ifndef GAME_SERVER_EVENTHANDLER_H
#define GAME_SERVER_EVENTHANDLER_H

#include <engine/shared/protocol.h>
//
class CEventHandler
{
	struct SEventRef
	{
		int m_DataOffset;
		int m_Type;
		int m_Size;
		int m_X;
		int m_Y;
	};
	static const int MAX_EVENTS = 32;
	static const int MAX_EVENTS_TOTAL = 256;
	char m_aSharedData[MAX_EVENTS_TOTAL * 64];
	int m_CurrentOffset;

	SEventRef m_aEvents[MAX_EVENTS_TOTAL];
	int m_NumEvents;

	int m_aClientEventList[SERVER_MAX_CLIENTS][MAX_EVENTS];
	int m_aClientNumEvents[SERVER_MAX_CLIENTS];

	class CGameContext *m_pGameServer;

public:
	CGameContext *GameServer() const { return m_pGameServer; }
	void SetGameServer(CGameContext *pGameServer);

	CEventHandler();
	void Create(void *pData, int Type, int Size, int64_t Mask = -1);
	void Clear();
	void Snap(int SnappingClient);
};

#endif
