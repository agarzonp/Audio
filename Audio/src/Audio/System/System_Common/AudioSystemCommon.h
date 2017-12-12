#ifndef AUDIO_SYSTEM_COMMON_H
#define AUDIO_SYSTEM_COMMON_H

#include <string>

class IAudioSystem
{
public:

	virtual void Initialise() = 0;
	virtual void Deinitialise() = 0;

	virtual void Update() = 0;

	virtual bool LoadSound(const std::string& soundName) = 0;
	virtual bool UnloadSound(const std::string& soundName) = 0;

	virtual bool PlaySound(const std::string& soundName) = 0;
};

#endif // !AUDIO_SYSTEM_IMPL_H

