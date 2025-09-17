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
#ifndef ENGINE_MESSAGE_H
#define ENGINE_MESSAGE_H

#include <base/uuid.h>
#include <engine/shared/packer.h>

class CMsgPacker : public CPacker
{
public:
	CMsgPacker(int Type, bool System = false)
	{
		Reset();
		if(Type < 0 || Type > 0x3FFFFFFF)
		{
			m_Error = true;
			return;
		}
		AddInt((Type << 1) | (System ? 1 : 0));
	}
	CMsgPacker(const Uuid &MsgID, bool System = false)
	{
		Reset();
		AddInt(System ? 1 : 0);
		AddRaw(&MsgID, sizeof(MsgID));
	}

	CMsgPacker(const char *pUuidStr, bool System = false) :
		CMsgPacker(CalculateUuid(pUuidStr), System)
	{
	}
};

class CMsgUnpacker : public CUnpacker
{
	int m_Type;
	bool m_System;
	const Uuid *m_pUuid;

public:
	CMsgUnpacker() = default;

	CMsgUnpacker(const void *pData, int Size)
	{
		Reset(pData, Size);
		const int Msg = GetInt();
		if(Msg < 0)
			m_Error = true;
		if(m_Error)
		{
			m_System = false;
			m_Type = 0;
			m_pUuid = nullptr;
			return;
		}
		m_System = Msg & 1;
		m_Type = Msg >> 1;
		m_pUuid = nullptr;
		if(m_Type == 0)
		{
			m_pUuid = (const Uuid *) GetRaw(sizeof(*m_pUuid));
			if(m_Error)
			{
				m_pUuid = nullptr;
				m_Error = false;
				Reset(pData, Size);
				GetInt();
				m_System = Msg & 1;
				m_Type = Msg >> 1;
				return;
			}
		}
	}

	int Type() const { return m_Type; }
	bool System() const { return m_System; }
	const Uuid *MsgUuid() const { return m_pUuid; }
};

#endif
