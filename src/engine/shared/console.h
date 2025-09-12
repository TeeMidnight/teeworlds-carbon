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
#ifndef ENGINE_SHARED_CONSOLE_H
#define ENGINE_SHARED_CONSOLE_H

#include <engine/console.h>
#include "memheap.h"
#include <new>

class CConsole : public IConsole
{
	class CCommand : public CCommandInfo
	{
	public:
		CCommand(bool BasicAccess) :
			CCommandInfo(BasicAccess) {}
		CCommand *m_pNext;
		int m_Flags;
		bool m_Temp;
		FCommandCallback m_pfnCallback;
		void *m_pUserData;

		virtual const CCommandInfo *NextCommandInfo(int AccessLevel, int FlagMask) const;

		void SetAccessLevel(int AccessLevel) { m_AccessLevel = clamp(AccessLevel, (int) (ACCESS_LEVEL_ADMIN), (int) (ACCESS_LEVEL_MOD)); }
	};

	class CChain
	{
	public:
		FChainCommandCallback m_pfnChainCallback;
		FCommandCallback m_pfnCallback;
		void *m_pCallbackUserData;
		void *m_pUserData;
	};

	int m_FlagMask;
	bool m_StoreCommands;
	const char *m_apStrokeStr[2];
	CCommand *m_pFirstCommand;

	class CExecFile
	{
	public:
		const char *m_pFilename;
		CExecFile *m_pPrev;
	};

	CExecFile *m_pFirstExec;
	class CConfig *m_pConfig;
	class IStorage *m_pStorage;
	int m_AccessLevel;

	CCommand *m_pRecycleList;
	CHeap m_TempCommands;

	void TraverseChain(FCommandCallback *ppfnCallback, void **ppUserData);

	static void Con_Chain(IResult *pResult, void *pUserData);
	static void Con_Echo(IResult *pResult, void *pUserData);
	static void Con_Exec(IResult *pResult, void *pUserData);
	static void Con_EvalIf(IResult *pResult, void *pUserData);
	static void Con_EvalIfCmd(IResult *pResult, void *pUserData);
	static void ConToggle(IResult *pResult, void *pUser);
	static void ConToggleStroke(IResult *pResult, void *pUser);
	static void ConModCommandAccess(IResult *pResult, void *pUser);
	static void ConModCommandStatus(IResult *pResult, void *pUser);

	void ExecuteFileRecurse(const char *pFilename);
	void ExecuteLineStroked(int Stroke, const char *pStr) override;

	struct
	{
		int m_OutputLevel;
		FPrintCallback m_pfnPrintCallback;
		void *m_pPrintCallbackUserdata;
	} m_aPrintCB[MAX_PRINT_CB];
	int m_NumPrintCB;

	enum
	{
		CONSOLE_MAX_STR_LENGTH = 1024,
		MAX_PARTS = (CONSOLE_MAX_STR_LENGTH + 1) / 2
	};

	class CResult : public IResult
	{
	public:
		char m_aStringStorage[CONSOLE_MAX_STR_LENGTH + 1];
		char *m_pArgsStart;

		const char *m_pCommand;
		const char *m_apArgs[MAX_PARTS];

		CResult();
		CResult &operator=(const CResult &Other);
		void AddArgument(const char *pArg);

		const char *GetString(unsigned Index) override;
		int GetInteger(unsigned Index) override;
		float GetFloat(unsigned Index) override;
	};

	int ParseStart(CResult *pResult, const char *pString, int Length);
	int ParseArgs(CResult *pResult, const char *pFormat);

	/*
	This function will set pFormat to the next parameter (i,s,r,v,?) it contains and
	pNext to the command.
	Descriptions in brackets like [file] will be skipped.
	Returns true on failure.
	Expects pFormat to point at a parameter.
	*/
	bool NextParam(char *pNext, const char *&pFormat);

	class CExecutionQueue
	{
		CHeap m_Queue;

	public:
		struct CQueueEntry
		{
			CQueueEntry *m_pNext;
			CCommand *m_pCommand;
			CResult m_Result;
		} *m_pFirst, *m_pLast;

		void AddEntry()
		{
			CQueueEntry *pEntry = static_cast<CQueueEntry *>(m_Queue.Allocate(sizeof(CQueueEntry)));
			pEntry->m_pNext = 0;
			if(!m_pFirst)
				m_pFirst = pEntry;
			if(m_pLast)
				m_pLast->m_pNext = pEntry;
			m_pLast = pEntry;
			(void) new(&(pEntry->m_Result)) CResult;
		}
		void Reset()
		{
			m_Queue.Reset();
			m_pFirst = m_pLast = 0;
		}
	} m_ExecutionQueue;

	void AddCommandSorted(CCommand *pCommand);
	CCommand *FindCommand(const char *pName, int FlagMask);

	struct CMapListEntryTemp
	{
		CMapListEntryTemp *m_pPrev;
		CMapListEntryTemp *m_pNext;
		char m_aName[TEMPMAP_NAME_LENGTH];
	};

	CHeap *m_pTempMapListHeap;
	CMapListEntryTemp *m_pFirstMapEntry;
	CMapListEntryTemp *m_pLastMapEntry;

public:
	CConsole(int FlagMask);
	~CConsole();

	void Init() override;
	const CCommandInfo *FirstCommandInfo(int AccessLevel, int FlagMask) const override;
	const CCommandInfo *GetCommandInfo(const char *pName, int FlagMask, bool Temp) override;
	int PossibleCommands(const char *pStr, int FlagMask, bool Temp, FPossibleCallback pfnCallback, void *pUser) override;
	int PossibleMaps(const char *pStr, FPossibleCallback pfnCallback, void *pUser) override;

	void ParseArguments(int NumArgs, const char **ppArguments) override;
	void Register(const char *pName, const char *pParams, int Flags, FCommandCallback pfnFunc, void *pUser, const char *pHelp) override;
	void RegisterTemp(const char *pName, const char *pParams, int Flags, const char *pHelp) override;
	void DeregisterTemp(const char *pName) override;
	void DeregisterTempAll() override;
	void RegisterTempMap(const char *pName) override;
	void DeregisterTempMap(const char *pName) override;
	void DeregisterTempMapAll() override;
	void Chain(const char *pName, FChainCommandCallback pfnChainFunc, void *pUser) override;
	void StoreCommands(bool Store) override;

	bool ArgStringIsValid(const char *pFormat) override;
	bool LineIsValid(const char *pStr) override;
	void ExecuteLine(const char *pStr) override;
	void ExecuteLineFlag(const char *pStr, int FlagMask) override;
	bool ExecuteFile(const char *pFilename) override;

	int RegisterPrintCallback(int OutputLevel, FPrintCallback pfnPrintCallback, void *pUserData) override;
	void SetPrintOutputLevel(int Index, int OutputLevel) override;
	void Print(int Level, const char *pFrom, const char *pStr, bool Highlighted = false) override;

	int ParseCommandArgs(const char *pArgs, const char *pFormat, FCommandCallback pfnCallback, void *pContext) override;

	void SetAccessLevel(int AccessLevel) override { m_AccessLevel = clamp(AccessLevel, (int) (ACCESS_LEVEL_ADMIN), (int) (ACCESS_LEVEL_MOD)); }
};

#endif
