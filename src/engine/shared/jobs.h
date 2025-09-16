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
#ifndef ENGINE_SHARED_JOBS_H
#define ENGINE_SHARED_JOBS_H
typedef int (*JOBFUNC)(void *pData);

class CJobPool;

class CJob
{
	friend class CJobPool;

	CJob *m_pPrev;
	CJob *m_pNext;

	volatile int m_Status;
	volatile int m_Result;

	JOBFUNC m_pfnFunc;
	void *m_pFuncData;

public:
	CJob()
	{
		m_Status = STATE_DONE;
		m_pFuncData = 0;
	}

	enum
	{
		STATE_PENDING = 0,
		STATE_RUNNING,
		STATE_DONE
	};

	int Status() const { return m_Status; }
	int Result() const { return m_Result; }
};

class CJobPool
{
	enum
	{
		MAX_THREADS = 32
	};
	int m_NumThreads;
	void *m_apThreads[MAX_THREADS];
	volatile bool m_Shutdown;

	void *m_Lock;
	CJob *m_pFirstJob;
	CJob *m_pLastJob;

	static void WorkerThread(void *pUser);

public:
	CJobPool();
	~CJobPool();

	int Init(int NumThreads);
	void Shutdown();
	int Add(CJob *pJob, JOBFUNC pfnFunc, void *pData);
};
#endif
