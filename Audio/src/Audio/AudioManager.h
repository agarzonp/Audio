#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <cassert>
#include <memory>

#include "System\AudioSystem.h"

class AudioManager
{
public:

	void Initialise()
	{
		// check that we haven´t initialised it before
		assert(audioSystem == nullptr);
		if (audioSystem)
		{
			return;
		}

		audioSystem = AudioSystem::Create();

		audioSystem->Initialise();
	}

	void Deinitialise()
	{
		if (!audioSystem)
		{
			return;
		}

		audioSystem->Deinitialise();
		audioSystem.reset();
	}

	void Update()
	{
		if (!audioSystem)
		{
			return;
		}

		audioSystem->Update();
	}

	bool LoadSound(const std::string& soundName)
	{
		if (!audioSystem)
		{
			return false;
		}

		return audioSystem->LoadSound(soundName);
	}
	bool PlaySound(const std::string& soundName)
	{
		if (!audioSystem)
		{
			return false;
		}

		return audioSystem->PlaySound(soundName);
	}

private:

	static std::unique_ptr<IAudioSystem> audioSystem;
};

std::unique_ptr<IAudioSystem> AudioManager::audioSystem;

#endif
