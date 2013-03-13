#pragma once

#include <al.h>
#include <alc.h>
#include <alut.h>
#include "Help.h"


class CSound
{
private:
	ALCdevice*		pDevice;
	ALCcontext*		pContext;
	ALfloat			ListenerPos[3];
	ALfloat			ListenerVel[3];
	ALfloat			ListenerOri[6];
	ALuint			source[3];
	unsigned int	buffer[3];
public:
	CSound();
   ~CSound();
	ALboolean CheckALCError();
	ALboolean CheckALError();
	void Play(int i);
};

