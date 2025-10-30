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
#include <engine/console.h>
#include "editor.h"

CLayerGame::CLayerGame(int w, int h) :
	CLayerTiles(w, h)
{
	str_copy(m_aName, "Game", sizeof(m_aName));
	m_Game = 1;
}

CLayerGame::~CLayerGame()
{
}

int CLayerGame::RenderProperties(CUIRect *pToolbox)
{
	int r = CLayerTiles::RenderProperties(pToolbox);
	m_Image = -1;
	return r;
}
