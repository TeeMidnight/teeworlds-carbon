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
#ifndef GAME_CLIENT_COMPONENTS_SOUNDS_H
#define GAME_CLIENT_COMPONENTS_SOUNDS_H

#include <engine/shared/jobs.h>
#include <engine/sound.h>
#include <game/client/component.h>

class CSounds : public CComponent
{
	enum
	{
		QUEUE_SIZE = 32,
	};
	struct QueueEntry
	{
		int m_Channel;
		int m_SetId;
	} m_aQueue[QUEUE_SIZE];
	int m_QueuePos;
	int64_t m_QueueWaitTime;
	class CJob m_SoundJob;
	bool m_WaitForSoundJob;

	int GetSampleId(int SetId);

public:
	// sound channels
	enum
	{
		CHN_GUI = 0,
		CHN_MUSIC,
		CHN_WORLD,
		CHN_GLOBAL,
		CHN_MAPSOUND,
	};

	int GetInitAmount() const override;
	void OnInit() override;
	void OnReset() override;
	void OnStateChange(int NewState, int OldState) override;
	void OnRender() override;

	void ClearQueue();
	void Enqueue(int Channel, int SetId);
	void Play(int Channel, int SetId, float Vol);
	void PlayAt(int Channel, int SetId, float Vol, vec2 Pos);
	void Stop(int SetId);
	bool IsPlaying(int SetId);

	void UpdateSoundVolume();

	ISound::CSampleHandle PlaySample(int Channel, int SampleId, float Vol, int Flags = 0);
	ISound::CSampleHandle PlaySampleAt(int Channel, int SampleId, float Vol, vec2 Pos, int Flags = 0);
};

#endif
