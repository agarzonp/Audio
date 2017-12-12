#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <cassert>
#include <map>
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

    if (audioSystem->LoadSound(soundName, *sound))
    {
      // insert a new pair in the map
      soundMap.insert(std::make_pair(soundName, sound));
      return true;
    }

		return false;
	}
	bool PlaySound(const std::string& soundName)
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
