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
#ifndef GAME_CLIENT_COMPONENTS_EMOTICON_H
#define GAME_CLIENT_COMPONENTS_EMOTICON_H
#include <base/vmath.h>
#include <game/client/component.h>

class CEmoticon : public CComponent
{
	void DrawCircle(float x, float y, float r, int Segments);

	bool m_WasActive;
	bool m_Active;

	vec2 m_SelectorMouse;
	int m_SelectedEmote;

	static void ConKeyEmoticon(IConsole::IResult *pResult, void *pUserData);
	static void ConEmote(IConsole::IResult *pResult, void *pUserData);

public:
	CEmoticon();

	void OnReset() override;
	void OnConsoleInit() override;
	void OnRender() override;
	void OnRelease() override;
	bool OnCursorMove(float x, float y, int CursorType) override;

	void SendEmote(int Emoticon);
};

#endif
