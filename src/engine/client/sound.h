/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
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

	virtual bool IsSoundEnabled() { return m_SoundEnabled != 0; }

	virtual int LoadWV(const char *pFilename);
	virtual int LoadWVFromMem(const void *pData, unsigned DataSize, bool FromEditor);
	virtual int LoadOpus(const char *pFilename);
	virtual int LoadOpusFromMem(const void *pData, unsigned DataSize, bool FromEditor);
	virtual void UnloadSample(int SampleID);

	virtual float GetSampleDuration(int SampleID); // in s

	virtual void SetListenerPos(float x, float y);
	virtual void SetChannel(int ChannelID, float Vol, float Pan);

	virtual void SetVoiceVolume(ISound::CSampleHandle Voice, float Volume);
	virtual void SetVoiceFalloff(ISound::CSampleHandle Voice, float Falloff);
	virtual void SetVoiceLocation(ISound::CSampleHandle Voice, float x, float y);
	virtual void SetVoiceTimeOffset(ISound::CSampleHandle Voice, float offset); // in s

	virtual void SetVoiceCircle(ISound::CSampleHandle Voice, float Radius);
	virtual void SetVoiceRectangle(ISound::CSampleHandle Voice, float Width, float Height);

	ISound::CSampleHandle Play(int ChannelID, int SampleID, int Flags, float x, float y);
	virtual ISound::CSampleHandle PlayAt(int ChannelID, int SampleID, int Flags, float x, float y);
	virtual ISound::CSampleHandle Play(int ChannelID, int SampleID, int Flags);
	virtual void Stop(int SampleID);
	virtual void StopAll();
	virtual void StopVoice(ISound::CSampleHandle Voice);
	virtual bool IsPlaying(int Sound);
};

#endif
