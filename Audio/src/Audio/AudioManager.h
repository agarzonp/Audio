#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <cassert>
#include <map>
#include <memory>

#include "System\AudioSystem.h"

class AudioManager
{
public:

	static void Initialise()
	{
    AudioSystem::InitParams initParams;
    initParams.audioSystemType = AudioSystemType::FMOD;
    initParams.audioAssetsPath = "assets/Audio/";

    s_audioSystem.Initialise(initParams);
	}

	static void Deinitialise()
	{
    s_audioSystem.Deinitialise();
	}

	static void Update()
	{
    s_audioSystem.Update();
	}

	static bool LoadSound(const std::string& soundName, AudioSystemSoundMode soundMode = AudioSystemSoundMode_DEFAULT)
	{
    return s_audioSystem.LoadSound(soundName, soundMode);
	}

	static bool UnloadSound(const std::string& soundName)
	{
    return s_audioSystem.UnloadSound(soundName);
	}

	static bool PlaySound(const std::string& soundName)
	{
    return s_audioSystem.PlaySound(soundName);
	}

	static bool StopSound(const std::string& soundName)
	{
		return s_audioSystem.StopSound(soundName);
	}

	static bool PauseSound(const std::string& soundName)
	{
		return s_audioSystem.PauseSound(soundName, true);
	}

	static bool ResumeSound(const std::string& soundName)
	{
		return s_audioSystem.PauseSound(soundName, false);
	}

	static bool SetSoundVolume(const std::string& soundName, float volume)
	{
		return s_audioSystem.SetSoundVolume(soundName, volume);
	}

	static bool SetSoundPitch(const std::string& soundName, float pitch)
	{
    return s_audioSystem.SetSoundPitch(soundName, pitch);
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

		IAudioSystemSound* sound { nullptr };
		AudioSourceDesc desc;

	public:

		void Play()
		{
			if (AudioManager::PlaySound(desc.soundName))
			{
				sound->Set3DAttributes(desc.position, desc.velocity);
				sound->Set3DMinMaxDistance(desc.minDistance, desc.maxDistance);
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
				sound->Set3DAttributes(position, desc.velocity);
			}
		}

		void SetVelocity(const AudioSystemVector& velocity)
		{
			assert(sound);
			if (sound)
			{
				desc.velocity = velocity;
				sound->Set3DAttributes(desc.position, velocity);
			}
		}

		void Set3DMinMaxDistance(float minDistance, float maxDistance)
		{
			assert(sound);
			if (sound)
			{
				desc.minDistance = minDistance;
				desc.maxDistance = maxDistance;

				sound->Set3DMinMaxDistance(minDistance, maxDistance);
			}
		}

		friend class AudioManager;
	};
	

	static void SetListener(const AudioSystemVector& position, const AudioSystemVector& velocity, const AudioSystemVector& forward, const AudioSystemVector& up)
	{
		s_audioSystem.SetListener(position, velocity, forward, up);
	}

	static AudioSource SetAudioSource(const AudioSourceDesc& desc)
	{
		AudioSource audioSource;

    if (LoadSound(desc.soundName, AudioSystemSoundMode_3D))
    {
      auto sound = s_audioSystem.GetSound(desc.soundName);
      audioSource.desc = desc;
      audioSource.sound = sound;
    }

		return audioSource;
	}

	//////////////////////////////////////////////////////////


private:

  static AudioSystem s_audioSystem;
};

AudioSystem AudioManager::s_audioSystem;

#endif
