/*
 * This file is part of Carbon, a modified version of Teeworlds.
 * This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
 *
 * Copyright (C) 2017 Dennis Felsing
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#include "protocol_ex.h"

#include "config.h"
#include "protocol.h"
#include "uuid_manager.h"

#include <new>

void RegisterUuids(class CUuidManager *pManager)
{
#define UUID(id, name) pManager->RegisterName(id, name);
#include "protocol_ex_msgs.h"
#undef UUID
}

int UnpackMessageID(int *pID, bool *pSys, struct Uuid *pUuid, CUnpacker *pUnpacker, CMsgPacker *pPacker)
{
	*pID = 0;
	*pSys = false;
	mem_zero(pUuid, sizeof(*pUuid));

	int MsgID = pUnpacker->GetInt();

	if(pUnpacker->Error())
	{
		return UNPACKMESSAGE_ERROR;
	}

	*pID = MsgID >> 1;
	*pSys = MsgID & 1;

	if(*pID < 0 || *pID >= OFFSET_UUID)
	{
		return UNPACKMESSAGE_ERROR;
	}

	if(*pID != 0) // NETMSG_EX, NETMSGTYPE_EX
	{
		return UNPACKMESSAGE_OK;
	}

	*pID = g_UuidManager.UnpackUuid(pUnpacker, pUuid);

	if(*pID == UUID_INVALID || *pID == UUID_UNKNOWN)
	{
		return UNPACKMESSAGE_ERROR;
	}

	return UNPACKMESSAGE_OK;
}
