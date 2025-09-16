/*
 * This file is part of Carbon, a modified version of Teeworlds.
 * This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
 *
 * Copyright (C) 2007-2014 Magnus Auvinen
 * Copyright (C) 2014-2018 Dennis Felsing
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef ENGINE_CLIENT_SOUND_H
#define ENGINE_CLIENT_SOUND_H

#include <engine/sound.h>

class CSound : public IEngineSound
{
	int m_SoundEnabled;

public:
	CConfig *m_pConfig;
	IEngineGraphics *m_pGraphics;
	IStorage *m_pStorage;

	int Init() override;

	int Update() override;
	void Shutdown() override;
	int AllocID();

	static void RateConvert(int SampleID);

	static int DecodeWV(int SampleID, const void *pData, unsigned DataSize);
	static int DecodeOpus(int SampleID, const void *pData, unsigned DataSize);

	bool IsSoundEnabled() override { return m_SoundEnabled != 0; }

	int LoadWV(const char *pFilename) override;
	int LoadWVFromMem(const void *pData, unsigned DataSize, bool FromEditor) override;
	int LoadOpus(const char *pFilename) override;
	int LoadOpusFromMem(const void *pData, unsigned DataSize, bool FromEditor) override;
	void UnloadSample(int SampleID) override;

	float GetSampleDuration(int SampleID) override; // in s

	void SetListenerPos(float x, float y) override;
	void SetChannel(int ChannelID, float Vol, float Pan) override;

	void SetVoiceVolume(ISound::CSampleHandle Voice, float Volume) override;
	void SetVoiceFalloff(ISound::CSampleHandle Voice, float Falloff) override;
	void SetVoiceLocation(ISound::CSampleHandle Voice, float x, float y) override;
	void SetVoiceTimeOffset(ISound::CSampleHandle Voice, float offset) override; // in s

	void SetVoiceCircle(ISound::CSampleHandle Voice, float Radius) override;
	void SetVoiceRectangle(ISound::CSampleHandle Voice, float Width, float Height) override;

	ISound::CSampleHandle Play(int ChannelID, int SampleID, int Flags, float x, float y);
	ISound::CSampleHandle PlayAt(int ChannelID, int SampleID, int Flags, float x, float y) override;
	ISound::CSampleHandle Play(int ChannelID, int SampleID, int Flags) override;
	void Stop(int SampleID) override;
	void StopAll() override;
	void StopVoice(ISound::CSampleHandle Voice) override;
	bool IsPlaying(int Sound) override;
};

#endif
