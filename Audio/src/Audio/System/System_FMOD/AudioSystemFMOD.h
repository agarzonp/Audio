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

	bool PlaySound(AudioSystemSound& audioSystemSound) override
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
    if (result == FMOD_OK)
		{
			audioSystemSound.SetSoundControl(channel);
		}
		else
		{
			printf("Failed to play sound. Error: %s \n", FMOD_ErrorString(result));
		}
      
		return (result == FMOD_OK);
	}

	bool StopSound(const AudioSystemSound& audioSystemSound) override
	{
		if (!system)
		{
			return false;
		}

		FMOD::Channel* channel = static_cast<FMOD::Channel*> (audioSystemSound.GetSoundControl());
		if (!channel)
		{
			return false;
		}

		FMOD_RESULT result = channel->stop();
		if (result != FMOD_OK)
			printf("Failed to stop sound. Error: %s \n", FMOD_ErrorString(result));

		return (result == FMOD_OK);
	}

	bool PauseSound(const AudioSystemSound& audioSystemSound, bool pause) override
	{
		if (!system)
		{
			return false;
		}

		FMOD::Channel* channel = static_cast<FMOD::Channel*> (audioSystemSound.GetSoundControl());
		if (!channel)
		{
			return false;
		}

		FMOD_RESULT result = channel->setPaused(pause);
		if (result != FMOD_OK)
			printf("Failed to %s sound. Error: %s \n", pause ? "pause" : "resume",  FMOD_ErrorString(result));

		return (result == FMOD_OK);
	}

	bool SetSoundVolume(const AudioSystemSound& audioSystemSound, float volume) override
	{
		if (!system)
		{
			return false;
		}

		FMOD::Channel* channel = static_cast<FMOD::Channel*> (audioSystemSound.GetSoundControl());
		if (!channel)
		{
			return false;
		}

		FMOD_RESULT result = channel->setVolume(volume);
		if (result != FMOD_OK)
			printf("Failed to set volume for sound. Error: %s \n", FMOD_ErrorString(result));

		return (result == FMOD_OK);
	}

	bool SetSoundPitch(const AudioSystemSound& audioSystemSound, float pitch) override
	{
		FMOD::Channel* channel = static_cast<FMOD::Channel*> (audioSystemSound.GetSoundControl());
		if (!channel)
		{
			return false;
		}

		FMOD_RESULT result = channel->setPitch(pitch);
		if (result != FMOD_OK)
			printf("Failed to set pitch for sound. Error: %s \n", FMOD_ErrorString(result));

		return (result == FMOD_OK);
	}
};

#endif // !AUDIO_SYSTEM_FMOD

