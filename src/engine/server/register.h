/*
* This file is part of NewTeeworldsCN, a modified version of Teeworlds.
* This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
* 
* Copyright (C) 2022-2025 Dennis Felsing
* Copyright (C) 2025 NewTeeworldsCN
* 
* This software is provided 'as-is', under the zlib License.
* See license.txt in the root of the distribution for more information.
* If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
*/
#ifndef ENGINE_SERVER_REGISTER_H
#define ENGINE_SERVER_REGISTER_H

class CConfig;
class IConsole;
class IEngine;
class IHttp;
struct CNetChunk;

class IRegister
{
public:
	virtual ~IRegister() {}

	virtual void Update() = 0;
	// Call `OnConfigChange` if you change relevant config variables
	// without going through the console.
	virtual void OnConfigChange() = 0;
	// Returns `true` if the packet was a packet related to registering
	// code and doesn't have to processed furtherly.
	virtual bool OnPacket(const CNetChunk *pPacket) = 0;
	// `pInfo` must be an encoded JSON object.
	virtual void OnNewInfo(const char *pInfo) = 0;
	virtual void OnShutdown() = 0;
};

IRegister *CreateRegister(CConfig *pConfig, IConsole *pConsole, IEngine *pEngine, IHttp *pHttp, int ServerPort, unsigned SixupSecurityToken);

#endif
