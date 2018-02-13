#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <cassert>
#include <map>
#include <memory>

#include "System\AudioSystem.h"

class AudioManager
{
	static const int SOUND_POOL_SIZE = 100;

public:

	static void Initialise()
	{
		// check that we haven´t initialised it before
		assert(audioSystem == nullptr);
		if (audioSystem)
		{
			return;
		}

		audioSystem = AudioSystem::Create();
		audioSystem->Initialise();

		soundPool.SetSize(SOUND_POOL_SIZE);
	}

	static void Deinitialise()
	{
		if (!audioSystem)
		{
			return;
		}

    // unload all sounds
    for (auto& entry : soundMap)
    {
      if (audioSystem->UnloadSound(*entry.second))
      {
        soundPool.Release(*entry.second);
      }
    }
    soundMap.clear();

    // deinitialise audioSystem
		audioSystem->Deinitialise();
		audioSystem.reset();
	}

	static void Update()
	{
		if (!audioSystem)
		{
			return;
		}

		// aupdate the system
		audioSystem->Update();
	}

	static bool LoadSound(const std::string& soundName, AudioSystemSoundMode soundMode = AudioSystemSoundMode_DEFAULT)
	{
		if (!audioSystem)
		{
			return false;
		}

    if (soundMap.find(soundName) != soundMap.end())
    {
      // sound already loaded
      return true;
    }

    AudioSystemSound* sound = soundPool.GetFree();
    if (!sound)
    {
      printf("Failed to load sound. Error: %s \n", "soundsPool is full. Consider to increase the size of the pool");
      return false;
    }

		static const char* path = "assets/Audio/";
    if (audioSystem->LoadSound(std::string(path) + soundName, soundMode, *sound))
    {
      // insert a new pair in the map
      soundMap.insert(std::make_pair(soundName, sound));
      return true;
    }

		return false;
	}

	static bool UnloadSound(const std::string& soundName)
	{
		if (!audioSystem)
		{
			return false;
		}

		auto soundMapIt = soundMap.find(soundName);
		if (soundMapIt == soundMap.end())
		{
			printf("Failed to unload sound %s. Error: Sound not loaded.", soundName.c_str());
			return false;
		}

		if (audioSystem->UnloadSound(*soundMapIt->second))
		{
			soundMap.erase(soundMapIt);
			return true;
		}

		return false;
	}

	static bool PlaySound(const std::string& soundName)
	{
		if (!audioSystem)
		{
			return false;
		}

    auto soundMapIt = soundMap.find(soundName);
    if (soundMapIt == soundMap.end())
    {
      printf("Failed to play sound %s. Error: Sound not loaded.", soundName.c_str());
      return false;
    }

		return audioSystem->PlaySound(*soundMapIt->second);
	}

	static bool StopSound(const std::string& soundName)
	{
		if (!audioSystem)
		{
			return false;
		}

		auto soundMapIt = soundMap.find(soundName);
		if (soundMapIt == soundMap.end())
		{
			printf("Failed to stop sound %s. Error: Sound not loaded.", soundName.c_str());
			return false;
		}

		return audioSystem->StopSound(*soundMapIt->second);
	}

	static bool PauseSound(const std::string& soundName)
	{
		return PauseSound(soundName, true);
	}

	static bool ResumeSound(const std::string& soundName)
	{
		return PauseSound(soundName, false);
	}

	static bool SetSoundVolume(const std::string& soundName, float volume)
	{
		if (!audioSystem)
		{
			return false;
		}

		auto soundMapIt = soundMap.find(soundName);
		if (soundMapIt == soundMap.end())
		{
			printf("Failed to set volume for sound %s. Error: Sound not loaded.", soundName.c_str());
			return false;
		}

		if (volume > 1.0f) volume = 1.0f;
		if (volume < 0.0f) volume = 0.0f;

		return audioSystem->SetSoundVolume(*soundMapIt->second, volume);
	}

	static bool SetSoundPitch(const std::string& soundName, float pitch)
	{
		if (!audioSystem)
		{
			return false;
		}

		auto soundMapIt = soundMap.find(soundName);
		if (soundMapIt == soundMap.end())
		{
			printf("Failed to set pitch for sound %s. Error: Sound not loaded.", soundName.c_str());
			return false;
		}

		return audioSystem->SetSoundPitch(*soundMapIt->second, pitch);
	}
	
	//////////////////////  3D AUDIO /////////////////////

	struct AudioSourceDesc
	{
		std::string soundName;

		AudioSystemVector position;
		AudioSystemVector velocity;

		float minDistance { -1.0f };
		float maxDistance { -1.0f };
	};

	class AudioSource
	{
	protected:

		AudioSystemSound* sound { nullptr };
		AudioSourceDesc desc;

	public:

		void Play()
		{
			if (AudioManager::PlaySound(desc.soundName))
			{
				sound->GetSoundController()->Set3DAttributes(desc.position, desc.velocity);
				sound->GetSoundController()->Set3DMinMaxDistance(desc.minDistance, desc.maxDistance);
			}
		}

		void Stop()
		{
			AudioManager::StopSound(desc.soundName);
		}

		void SetPosition(const AudioSystemVector& position)
		{
			assert(sound);
			if (sound)
			{
				desc.position = position;
				sound->GetSoundController()->Set3DAttributes(position, desc.velocity);
			}
		}

		void SetVelocity(const AudioSystemVector& velocity)
		{
			assert(sound);
			if (sound)
			{
				desc.velocity = velocity;
				sound->GetSoundController()->Set3DAttributes(desc.position, velocity);
			}
		}

		void Set3DMinMaxDistance(float minDistance, float maxDistance)
		{
			assert(sound);
			if (sound)
			{
				desc.minDistance = minDistance;
				desc.maxDistance = maxDistance;

				sound->GetSoundController()->Set3DMinMaxDistance(minDistance, maxDistance);
			}
		}

		friend class AudioManager;
	};
	

	static void SetListener(const AudioSystemVector& position, const AudioSystemVector& velocity, const AudioSystemVector& forward, const AudioSystemVector& up)
	{
		audioSystem->SetListener(position, velocity, forward, up);
	}

	static AudioSource SetAudioSource(const AudioSourceDesc& desc)
	{
		AudioSource audioSource;

		if (auto sound = Load3DSound(desc.soundName))
		{
			audioSource.desc = desc;
			audioSource.sound = sound;
		}

		return audioSource;
	}

	//////////////////////////////////////////////////////////

protected:

	static AudioSystemSound* Load3DSound(const std::string& soundName)
	{
		if (LoadSound(soundName, AudioSystemSoundMode_3D))
		{
			return soundMap[soundName];
		}

		return nullptr;
	}

	static bool PauseSound(const std::string& soundName, bool pause)
	{
		if (!audioSystem)
		{
			return false;
		}

		auto soundMapIt = soundMap.find(soundName);
		if (soundMapIt == soundMap.end())
		{
			printf("Failed to %s sound %s. Error: Sound not loaded.", pause ? "pause":"resume", soundName.c_str());
			return false;
		}

		return audioSystem->PauseSound(*soundMapIt->second, pause);
	}

private:

	static std::unique_ptr<IAudioSystem> audioSystem;
  static AudioSystemSoundPool soundPool;

  using SoundMap = std::map<std::string, AudioSystemSound*>;
  static SoundMap soundMap;
};

std::unique_ptr<IAudioSystem> AudioManager::audioSystem;
AudioSystemSoundPool AudioManager::soundPool;
AudioManager::SoundMap AudioManager::soundMap;

#endif
