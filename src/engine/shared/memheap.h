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
#ifndef ENGINE_SHARED_MEMHEAP_H
#define ENGINE_SHARED_MEMHEAP_H
class CHeap
{
	struct CChunk
	{
		char *m_pMemory;
		char *m_pCurrent;
		char *m_pEnd;
		CChunk *m_pNext;
	};

	enum
	{
		// how large each chunk should be
		CHUNK_SIZE = 1024 * 64,
	};

	CChunk *m_pCurrent;

	void Clear();
	void NewChunk();
	void *AllocateFromChunk(unsigned int Size);

public:
	CHeap();
	~CHeap();
	void Reset();
	void *Allocate(unsigned int Size);
	const char *StoreString(const char *pSrc);
};
#endif
