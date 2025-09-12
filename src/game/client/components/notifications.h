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
#ifndef GAME_CLIENT_COMPONENTS_NOTIFICATIONS_H
#define GAME_CLIENT_COMPONENTS_NOTIFICATIONS_H
#include <game/client/component.h>

class CNotifications : public CComponent
{
	float m_SoundToggleTime;

	void OnConsoleInit() override;
	void RenderSoundNotification();

	static void Con_SndToggle(IConsole::IResult *pResult, void *pUserData);

public:
	CNotifications();
	void OnRender() override;
};

#endif
