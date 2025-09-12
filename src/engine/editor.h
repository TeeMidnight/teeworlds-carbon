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
#ifndef ENGINE_EDITOR_H
#define ENGINE_EDITOR_H
#include "kernel.h"

class IEditor : public IInterface
{
	MACRO_INTERFACE("editor", 0)
public:
	virtual ~IEditor() {}
	virtual void Init() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual bool HasUnsavedData() const = 0;
};

extern IEditor *CreateEditor();
#endif
