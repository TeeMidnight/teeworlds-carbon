/*
 * This file is part of Carbon, a modified version of Teeworlds.
 * This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
 *
 * Copyright (C) 2014-2018 Dennis Felsing
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#ifndef GAME_CLIENT_COMPONENTS_MAPSOUNDS_H
#define GAME_CLIENT_COMPONENTS_MAPSOUNDS_H

#include <base/tl/array.h>

#include <engine/sound.h>

#include <game/client/component.h>

class CMapSounds : public CComponent
{
	int m_aSounds[64];
	int m_Count;

	struct CSourceQueueEntry
	{
		int m_Sound;
		bool m_HighDetail;
		ISound::CSampleHandle m_Voice;
		CSoundSource *m_pSource;

		bool operator==(const CSourceQueueEntry &Other) const { return (m_Sound == Other.m_Sound) && (m_Voice == Other.m_Voice) && (m_pSource == Other.m_pSource); }
	};

	array<CSourceQueueEntry> m_lSourceQueue;

	void Clear();

public:
	CMapSounds();

	void OnMapLoad() override;
	void OnRender() override;
	void OnStateChange(int NewState, int OldState) override;
};

#endif // GAME_CLIENT_COMPONENTS_MAPSOUNDS_H
