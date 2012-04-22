#pragma once

#include "Init.h"
#include <al.h>
#include <alc.h>
#include <alut.h>



class CSound
{
private:
	ALCdevice*		pDevice;
	ALCcontext*		pContext;
	ALfloat			ListenerPos[3];
	ALfloat			ListenerVel[3];
	ALfloat			ListenerOri[6];
public:
	CSound();
   ~CSound();
	ALboolean CheckALCError();
	ALboolean CheckALError();
	void Play();
};

class remSnd  
{
public:
	ALfloat mVel[3];
	ALfloat mPos[3];
	bool    mLooped;

	// Functions
	bool Open(const std::string &Filename, bool Looped, bool Streamed);
	bool LoadWavFile (const std::string &Filename);
	bool IsStreamed();
	void Play();
	void Close();
	void Update();
	void Move(float X, float Y, float Z);
	void Stop();

	// Construction/destruction
	remSnd();
	virtual ~remSnd();

private:
	// Идентификатор источника
	ALuint      mSourceID;
	// Потоковый ли наш звук?
	bool      mStreamed;

	
};

typedef struct 
{
	unsigned int  ID;
	std::string   Filename;
	unsigned int  Rate;
	unsigned int  Format;
} SndInfo;

//map<ALuint, SndInfo> Buffers;