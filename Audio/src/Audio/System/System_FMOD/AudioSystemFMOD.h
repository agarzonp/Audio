#ifndef AUDIO_SYSTEM_FMOD
#define AUDIO_SYSTEM_FMOD

#include "FMOD/fmod.hpp"
#include "FMOD/fmod_errors.h"

#include <vector>

class AudioSystemFMOD : public IAudioSystem
{
	FMOD::System* system { nullptr };

public:

	void Initialise() override
	{
		FMOD_RESULT result = FMOD::System_Create(&system);
		if (result != FMOD_OK)
		{
			printf("Failed to create FMOD system. Error: %s \n", FMOD_ErrorString(result));
			return;
		}

		// TO-DO: init config data should be passed as an argument of the function
		int maxChannels = 50;
		FMOD_INITFLAGS flags = FMOD_INIT_NORMAL;
		void* extraDriverData = nullptr;
		system->init(maxChannels, flags, extraDriverData);
	}

	void Deinitialise() override
	{
		if (!system)
		{
			return;
		}
		
		system->release();
	}

	void Update() override
	{
		if (!system)
		{
			return;
		}

		system->update();
	}

  bool LoadSound(const std::string& soundName, AudioSystemSound& outSound) override
	{
		if (!system)
		{
			return false;
		}

 
    // create the sound
		FMOD::Sound* sound = nullptr;
		FMOD_RESULT result = system->createSound(soundName.c_str(), FMOD_DEFAULT, nullptr, &sound);
    if (result != FMOD_OK)
    {
      printf("Failed to load sound. Error: %s \n", FMOD_ErrorString(result));
      return false;
    }

    outSound = AudioSystemSound(sound);

		return true;
	}

	bool UnloadSound(const AudioSystemSound& audioSystemSound) override
	{
    FMOD::Sound* sound = static_cast<FMOD::Sound*> (audioSystemSound.Get());
    if (sound)
    {
      sound->release();
      return true;
    }

		return false;
	}

	bool PlaySound(const AudioSystemSound& audioSystemSound) override
	{
		if (!system)
		{
			return false;
		}

    FMOD::Sound* sound = static_cast<FMOD::Sound*> (audioSystemSound.Get());
    if (!sound)
    {
      return false;
    }

    FMOD::Channel* channel = nullptr;
    bool paused = false;
    FMOD_RESULT result = system->playSound(sound, nullptr, paused, &channel);
    if (result != FMOD_OK)
      printf("Failed to play sound. Error: %s \n", FMOD_ErrorString(result));
		
		return (result == FMOD_OK);
	}
};

#endif // !AUDIO_SYSTEM_FMOD

