/*
 * This file is part of NewTeeworldsCN, a modified version of Teeworlds.
 * This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
 *
 * Copyright (C) 2021-2025 Dennis Felsing
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef ENGINE_SHARED_SERVERINFO_H
#define ENGINE_SHARED_SERVERINFO_H

#include "protocol.h"

#include <engine/map.h>
#include <engine/serverbrowser.h>

typedef struct _json_value json_value;
class CServerInfo;

class CServerInfo2
{
public:
	enum EClientScoreKind
	{
		CLIENT_SCORE_KIND_UNSPECIFIED,
		CLIENT_SCORE_KIND_POINTS,
		CLIENT_SCORE_KIND_TIME
	};

	class CClient
	{
	public:
		char m_aName[MAX_NAME_LENGTH];
		char m_aClan[MAX_CLAN_LENGTH];
		int m_Country;
		int m_Score;
		bool m_IsPlayer;
	};

	CClient m_aClients[MAX_CLIENTS];
	int m_MaxClients;
	int m_NumClients; // Indirectly serialized.
	int m_MaxPlayers;
	int m_NumPlayers; // Not serialized.
	EClientScoreKind m_ClientScoreKind;
	bool m_Passworded;
	char m_aGameType[16];
	char m_aName[64];
	char m_aMapName[128];
	char m_aVersion[32];

	bool operator==(const CServerInfo2 &Other) const;
	bool operator!=(const CServerInfo2 &Other) const { return !(*this == Other); }
	static bool FromJson(CServerInfo2 *pOut, const json_value *pJson);
	static bool FromJsonRaw(CServerInfo2 *pOut, const json_value *pJson);
	bool Validate() const;

	operator CServerInfo() const;
};

bool ParseCrc(unsigned int *pResult, const char *pString);

#endif // ENGINE_SHARED_SERVERINFO_H
