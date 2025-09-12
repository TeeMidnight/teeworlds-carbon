/*
* This file is part of NewTeeworldsCN, a modified version of Teeworlds.
* This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
* 
* Copyright (C) 2023-2025 Dennis Felsing
* Copyright (C) 2025 NewTeeworldsCN
* 
* This software is provided 'as-is', under the zlib License.
* See license.txt in the root of the distribution for more information.
* If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
*/
#ifndef ENGINE_HTTP_H
#define ENGINE_HTTP_H

#include "kernel.h"

class IHttpRequest
{
};

class IHttp : public IInterface
{
	MACRO_INTERFACE("http", 0)

public:
	virtual void Run(IHttpRequest *pRequest) = 0;
};

#endif // ENGINE_HTTP_H From DDNet
