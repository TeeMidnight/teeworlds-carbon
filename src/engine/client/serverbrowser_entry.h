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
#ifndef ENGINE_CLIENT_SERVERBROWSER_ENTRY_H
#define ENGINE_CLIENT_SERVERBROWSER_ENTRY_H

class CServerEntry
{
public:
	enum
	{
		STATE_INVALID = 0,
		STATE_PENDING,
		STATE_READY,
	};

	NETADDR m_Addr;
	int64_t m_RequestTime;
	int m_InfoState;
	int m_CurrentToken; // the token is to keep server refresh separated from each other
	int m_TrackID;
	class CServerInfo m_Info;

	CServerEntry *m_pNextIp; // ip hashed list

	CServerEntry *m_pPrevReq; // request list
	CServerEntry *m_pNextReq;
};

#endif
