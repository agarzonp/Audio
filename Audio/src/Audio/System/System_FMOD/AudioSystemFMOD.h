#ifndef AUDIO_SYSTEM_FMOD
#define AUDIO_SYSTEM_FMOD

#include "FMOD/fmod.hpp"
#include "FMOD/fmod_errors.h"

#include "FMODSoundController.h"

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

		// set listener
		const FMOD_VECTOR* listenerPos = reinterpret_cast<const FMOD_VECTOR*> (&listener.GetPosition());
		const FMOD_VECTOR* listenerVelocity = reinterpret_cast<const FMOD_VECTOR*> (&listener.GetVelocity());
		const FMOD_VECTOR* listenerForward = reinterpret_cast<const FMOD_VECTOR*> (&listener.GetForward());
		const FMOD_VECTOR* listenerUp = reinterpret_cast<const FMOD_VECTOR*> (&listener.GetUp());
		system->set3DListenerAttributes(0, listenerPos, listenerVelocity, listenerForward, listenerUp);

		// update the system
		system->update();
	}

  bool LoadSound(const std::string& soundName, AudioSystemSoundMode audioSystemSoundMode, AudioSystemSound& outSound) override
	{
		if (!system)
		{
			return false;
		}

		// get sound mode
		FMOD_MODE soundMode = FMOD_DEFAULT;
		if (audioSystemSoundMode & AudioSystemSoundMode_2D)
		{
			soundMode |= FMOD_2D;
		}
		else if (audioSystemSoundMode & AudioSystemSoundMode_3D)
		{
			soundMode |= FMOD_3D;
		}

    // create the sound
		FMOD::Sound* sound = nullptr;
		FMOD_RESULT result = system->createSound(soundName.c_str(), soundMode, nullptr, &sound);
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
			std::unique_ptr<AudioSystemSoundController> controller = std::make_unique<FMODSoundController>(channel);
			audioSystemSound.SetSoundController(controller);
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

		return audioSystemSound.GetSoundController()->Stop();
	}

	bool PauseSound(const AudioSystemSound& audioSystemSound, bool pause) override
	{
		if (!system)
		{
			return false;
		}

		return audioSystemSound.GetSoundController()->Pause(pause);
	}

	bool SetSoundVolume(const AudioSystemSound& audioSystemSound, float volume) override
	{
		if (!system)
		{
			return false;
		}

		return audioSystemSound.GetSoundController()->SetVolume(volume);
	}

	bool SetSoundPitch(const AudioSystemSound& audioSystemSound, float pitch) override
	{
		if (!system)
		{
			return false;
		}

		return audioSystemSound.GetSoundController()->SetPitch(pitch);
	}
};

#endif // !AUDIO_SYSTEM_FMOD

