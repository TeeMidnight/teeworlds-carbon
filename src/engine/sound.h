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
#ifndef ENGINE_SOUND_H
#define ENGINE_SOUND_H

#include "kernel.h"

class ISound : public IInterface
{
	MACRO_INTERFACE("sound", 0)
public:
	enum
	{
		FLAG_LOOP = 1 << 0,
		FLAG_POS = 1 << 1,
		FLAG_NO_PANNING = 1 << 2,
		FLAG_ALL = FLAG_LOOP | FLAG_POS | FLAG_NO_PANNING,
	};

	enum
	{
		SHAPE_CIRCLE,
		SHAPE_RECTANGLE,
	};

	struct CVoiceShapeCircle
	{
		float m_Radius;
	};

	struct CVoiceShapeRectangle
	{
		float m_Width;
		float m_Height;
	};

	class CSampleHandle
	{
		friend class ISound;
		int m_Id;
		int m_Age;

	public:
		CSampleHandle() :
			m_Id(-1), m_Age(-1)
		{
		}

		bool IsValid() const { return (Id() >= 0) && (Age() >= 0); }
		int Id() const { return m_Id; }
		int Age() const { return m_Age; }

		bool operator==(const CSampleHandle &Other) const { return m_Id == Other.m_Id && m_Age == Other.m_Age; }
	};

	virtual bool IsSoundEnabled() = 0;

	virtual int LoadWV(const char *pFilename) = 0;
	virtual int LoadOpus(const char *pFilename) = 0;
	virtual int LoadWVFromMem(const void *pData, unsigned DataSize, bool FromEditor = false) = 0;
	virtual int LoadOpusFromMem(const void *pData, unsigned DataSize, bool FromEditor = false) = 0;
	virtual void UnloadSample(int SampleID) = 0;

	virtual float GetSampleDuration(int SampleID) = 0; // in s

	virtual void SetChannel(int ChannelID, float Volume, float Panning) = 0;
	virtual void SetListenerPos(float x, float y) = 0;

	virtual void SetVoiceVolume(CSampleHandle Voice, float Volume) = 0;
	virtual void SetVoiceFalloff(CSampleHandle Voice, float Falloff) = 0;
	virtual void SetVoiceLocation(CSampleHandle Voice, float x, float y) = 0;
	virtual void SetVoiceTimeOffset(CSampleHandle Voice, float offset) = 0; // in s

	virtual void SetVoiceCircle(CSampleHandle Voice, float Radius) = 0;
	virtual void SetVoiceRectangle(CSampleHandle Voice, float Width, float Height) = 0;

	virtual CSampleHandle PlayAt(int ChannelID, int SampleID, int Flags, float x, float y) = 0;
	virtual CSampleHandle Play(int ChannelID, int SampleID, int Flags) = 0;
	virtual void Stop(int SampleID) = 0;
	virtual void StopAll() = 0;
	virtual void StopVoice(CSampleHandle Voice) = 0;
	virtual bool IsPlaying(int Sound) = 0;

protected:
	inline CSampleHandle CreateVoiceHandle(int Index, int Age)
	{
		CSampleHandle Voice;
		Voice.m_Id = Index;
		Voice.m_Age = Age;
		return Voice;
	}
};

class IEngineSound : public ISound
{
	MACRO_INTERFACE("enginesound", 0)
public:
	virtual int Init() = 0;
	virtual int Update() = 0;
	virtual void Shutdown() = 0;
};

extern IEngineSound *CreateEngineSound();

#endif
