#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <memory>

#include "System_Common\AudioSystemCommon.h"
#include "System_FMOD\AudioSystemFMOD.h"

class AudioSystem
{
public:

	static std::unique_ptr<IAudioSystem> Create()
	{
		return std::make_unique<AudioSystemFMOD>();
	}
};

#endif

