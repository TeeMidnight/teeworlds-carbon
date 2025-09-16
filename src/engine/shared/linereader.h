/*
 * This file is part of NewTeeworldsCN, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef ENGINE_SHARED_LINEREADER_H
#define ENGINE_SHARED_LINEREADER_H
#include <base/system.h>

// buffered stream for reading lines, should perhaps be something smaller
class CLineReader
{
	char m_aBuffer[4 * 1024 + 1]; // 1 additional byte for null termination
	unsigned m_BufferPos;
	unsigned m_BufferSize;
	unsigned m_BufferMaxSize;
	IOHANDLE m_IO;

public:
	void Init(IOHANDLE IoHandle);
	const char *Get();
};
#endif
