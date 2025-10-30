/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#include <base/system.h>
#include "eventhandler.h"
#include "gamecontext.h"
#include "player.h"

//////////////////////////////////////////////////
// Event handler
//////////////////////////////////////////////////
CEventHandler::CEventHandler()
{
	m_pGameServer = 0;
	Clear();
}

void CEventHandler::SetGameServer(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
}

void CEventHandler::Create(void *pData, int Type, int Size, int64_t Mask)
{
	if(m_NumEvents >= MAX_EVENTS_TOTAL || m_CurrentOffset + Size > MAX_EVENTS_TOTAL * 64)
		return;

	int Offset = m_CurrentOffset;
	mem_copy(m_aSharedData + Offset, pData, Size);
	m_CurrentOffset += Size;

	SEventRef &EventRef = m_aEvents[m_NumEvents++];
	EventRef.m_DataOffset = Offset;
	EventRef.m_Type = Type;
	EventRef.m_Size = Size;
	EventRef.m_X = static_cast<CNetEvent_Common *>(pData)->m_X;
	EventRef.m_Y = static_cast<CNetEvent_Common *>(pData)->m_Y;

	for(int i = 0; i < SERVER_MAX_CLIENTS; i++)
	{
		if(CmaskIsSet(Mask, i))
		{
			int &Num = m_aClientNumEvents[i];
			if(Num < MAX_EVENTS)
				m_aClientEventList[i][Num++] = m_NumEvents - 1;
		}
	}
}

void CEventHandler::Clear()
{
	for(int i = 0; i < SERVER_MAX_CLIENTS; i++)
	{
		m_aClientNumEvents[i] = 0;
	}
	m_CurrentOffset = 0;
	m_NumEvents = 0;
}

void CEventHandler::Snap(int SnappingClient)
{
	if(SnappingClient == -1)
		return;

	for(int i = 0; i < m_aClientNumEvents[SnappingClient]; i++)
	{
		int EventID = m_aClientEventList[SnappingClient][i];
		const SEventRef &EventRef = m_aEvents[EventID];

		if(NetworkClipped(SnappingClient, vec2(EventRef.m_X, EventRef.m_Y), GameServer()))
			continue;

		void *pData = GameServer()->Server()->SnapNewItem(EventRef.m_Type, EventID, EventRef.m_Size);
		if(pData)
			mem_copy(pData, m_aSharedData + EventRef.m_DataOffset, EventRef.m_Size);
	}
}
