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
#ifndef ENGINE_SHARED_UUID_MANAGER_H
#define ENGINE_SHARED_UUID_MANAGER_H

#include <base/tl/array.h>
#include <base/uuid.h>

struct CName
{
	Uuid m_Uuid;
	const char *m_pName;
};

class CPacker;
class CUnpacker;

class CUuidManager
{
	array<CName> m_aNames;

public:
	void RegisterName(int ID, const char *pName);
	Uuid GetUuid(int ID) const;
	const char *GetName(int ID) const;
	int LookupUuid(Uuid Uuid) const;

	int UnpackUuid(CUnpacker *pUnpacker) const;
	int UnpackUuid(CUnpacker *pUnpacker, Uuid *pOut) const;
	void PackUuid(int ID, CPacker *pPacker) const;

	void DebugDump() const;
};

extern CUuidManager g_UuidManager;

#endif // ENGINE_SHARED_UUID_MANAGER_H
