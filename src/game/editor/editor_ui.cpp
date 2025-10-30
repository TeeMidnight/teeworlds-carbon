/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */

#include <game/client/components/menus.h>
#include <game/client/lineinput.h>
#include <game/client/localization.h>
#include <game/client/render.h>
#include <game/client/ui.h>
#include "editor.h"

vec4 CEditor::GetButtonColor(const void *pID, int Checked)
{
	if(Checked < 0)
		return vec4(0, 0, 0, 0.5f);

	if(Checked > 0)
	{
		if(UI()->HotItem() == pID)
			return vec4(1, 0, 0, 0.75f);
		return vec4(1, 0, 0, 0.5f);
	}

	if(UI()->HotItem() == pID)
		return vec4(1, 1, 1, 0.75f);
	return vec4(1, 1, 1, 0.5f);
}

int CEditor::DoButton_Editor_Common(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	if(UI()->MouseInside(pRect))
	{
		if(Flags & BUTTON_CONTEXT)
			ms_pUiGotContext = pID;
		if(m_pTooltip)
			m_pTooltip = pToolTip;
	}

	if(UI()->HotItem() == pID && pToolTip)
		m_pTooltip = (const char *) pToolTip;

	if(UI()->DoButtonLogic(pID, pRect, 0))
		return 1;
	if(UI()->DoButtonLogic(pID, pRect, 1))
		return 2;
	return 0;
}

int CEditor::DoButton_Editor(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	pRect->Draw(GetButtonColor(pID, Checked), 3.0f);
	UI()->DoLabel(pRect, pText, 10.0f, TEXTALIGN_MC);
	return DoButton_Editor_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}

int CEditor::DoButton_Image(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip, bool Used)
{
	// darken the button if not used
	vec4 ButtonColor = GetButtonColor(pID, Checked);
	if(!Used)
		ButtonColor *= vec4(0.5f, 0.5f, 0.5f, 1.0f);

	const float FontSize = clamp(8.0f * pRect->w / TextRender()->TextWidth(10.0f, pText, -1), 6.0f, 10.0f);
	pRect->Draw(ButtonColor, 3.0f);
	UI()->DoLabel(pRect, pText, FontSize, TEXTALIGN_MC);
	return DoButton_Editor_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}

int CEditor::DoButton_Menu(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	pRect->Draw(vec4(0.5f, 0.5f, 0.5f, 1.0f), 3.0f, CUIRect::CORNER_T);

	CUIRect Label = *pRect;
	Label.VMargin(5.0f, &Label);
	UI()->DoLabel(&Label, pText, 10.0f, TEXTALIGN_ML);
	return DoButton_Editor_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}

int CEditor::DoButton_MenuItem(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	if(UI()->HotItem() == pID || Checked)
		pRect->Draw(GetButtonColor(pID, Checked), 3.0f);

	CUIRect Label = *pRect;
	Label.VMargin(5.0f, &Label);
	UI()->DoLabel(&Label, pText, 10.0f, TEXTALIGN_ML);
	return DoButton_Editor_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}

int CEditor::DoButton_Tab(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	pRect->Draw(GetButtonColor(pID, Checked), 5.0f, CUIRect::CORNER_T);
	UI()->DoLabel(pRect, pText, 10.0f, TEXTALIGN_MC);
	return DoButton_Editor_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}

int CEditor::DoButton_Ex(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip, int Corners, float FontSize)
{
	pRect->Draw(GetButtonColor(pID, Checked), 3.0f, Corners);
	UI()->DoLabel(pRect, pText, FontSize, TEXTALIGN_MC);
	return DoButton_Editor_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}

int CEditor::DoButton_ButtonInc(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	pRect->Draw(GetButtonColor(pID, Checked), 3.0f, CUIRect::CORNER_R);
	UI()->DoLabel(pRect, pText ? pText : "+", 10.0f, TEXTALIGN_MC);
	return DoButton_Editor_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}

int CEditor::DoButton_ButtonDec(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	pRect->Draw(GetButtonColor(pID, Checked), 3.0f, CUIRect::CORNER_L);
	UI()->DoLabel(pRect, pText ? pText : "-", 10.0f, TEXTALIGN_MC);
	return DoButton_Editor_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}
