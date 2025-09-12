/*
* This file is part of NewTeeworldsCN, a modified version of Teeworlds.
* This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
* 
* Copyright (C) 2020-2025 Dennis Felsing
* Copyright (C) 2025 NewTeeworldsCN
* 
* This software is provided 'as-is', under the zlib License.
* See license.txt in the root of the distribution for more information.
* If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
*/
#ifndef GAME_SERVER_TEEINFO_H
#define GAME_SERVER_TEEINFO_H

#include <engine/shared/protocol.h>
#include <generated/protocol.h>

struct STeeInfo
{
	char m_aaSkinPartNames[NUM_SKINPARTS][MAX_SKIN_ARRAY_SIZE];
	int m_aUseCustomColors[NUM_SKINPARTS];
	int m_aSkinPartColors[NUM_SKINPARTS];
};

STeeInfo GenerateRandomSkin();

#endif // GAME_SERVER_TEEINFO_H
