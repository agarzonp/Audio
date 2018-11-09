#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <memory>

#include "AudioSystemFactory.h"

// AudioSystem
//
class AudioSystem
{
public:

  struct InitParams
  {
    AudioSystemType audioSystemType{ AudioSystemType::FMOD };
    const char* audioAssetsPath;
  };


  void Initialise(const InitParams& params)
  {
    // init system
    system = AudioSystemFactory::NewSystem(params.audioSystemType);
    system->Initialise();

    // set audio assets path
    audioAssetsPath = params.audioAssetsPath;
  }

  void Deinitialise()
  {
    if (!system)
    {
      return;
    }

    // unload all sounds
    for (auto& entry : soundMap)
    {
      system->UnloadSound(entry.second);
    }
    soundMap.clear();

    // deinitialise system
    system->Deinitialise();
    system.reset();
  }

  void Update()
  {
    if (system)
    {
      system->Update();
    }
  }

  bool LoadSound(const std::string& soundName, AudioSystemSoundMode soundMode = AudioSystemSoundMode_DEFAULT)
  {
    if (!system)
    {
      return false;
    }

    if (soundMap.find(soundName) != soundMap.end())
    {
      // sound already loaded
      return true;
    }

    std::string soundPath = audioAssetsPath + soundName;
    if (auto sound = system->LoadSound(soundPath, soundMode))
    {
      // insert a new pair in the map
      soundMap.insert(std::make_pair(soundName, sound));
      return true;
    }

    printf("Failed to load sound %s\n", soundPath.c_str());

    return false;
  }

  bool UnloadSound(const std::string& soundName)
  {
    if (!system)
    {
      return false;
    }

    auto soundMapIt = soundMap.find(soundName);
    if (soundMapIt == soundMap.end())
    {
      printf("Failed to unload sound %s. Error: Sound not loaded.", soundName.c_str());
      return false;
    }

    if (system->UnloadSound(soundMapIt->second))
    {
      soundMap.erase(soundMapIt);
      return true;
    }

    return false;
  }

  bool PlaySound(const std::string& soundName)
  {
    if (!system)
    {
      return false;
    }

    auto soundMapIt = soundMap.find(soundName);
    if (soundMapIt == soundMap.end())
    {
      printf("Failed to play sound %s. Error: Sound not loaded.", soundName.c_str());
      return false;
    }

    return soundMapIt->second->Play();
  }

  bool StopSound(const std::string& soundName)
  {
    if (!system)
    {
      return false;
    }

    auto soundMapIt = soundMap.find(soundName);
    if (soundMapIt == soundMap.end())
    {
      printf("Failed to stop sound %s. Error: Sound not loaded.", soundName.c_str());
      return false;
    }

    return soundMapIt->second->Stop();
  }

  bool PauseSound(const std::string& soundName, bool pause)
  {
    if (!system)
    {
      return false;
    }

    auto soundMapIt = soundMap.find(soundName);
    if (soundMapIt == soundMap.end())
    {
      printf("Failed to %s sound %s. Error: Sound not loaded.", pause ? "pause" : "resume", soundName.c_str());
      return false;
    }

    return soundMapIt->second->Pause(pause);
  }

  bool SetSoundVolume(const std::string& soundName, float volume)
  {
    if (!system)
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

    return soundMapIt->second->SetVolume(volume);
  }

  bool SetSoundPitch(const std::string& soundName, float pitch)
  {
    if (!system)
    {
      return false;
    }

    auto soundMapIt = soundMap.find(soundName);
    if (soundMapIt == soundMap.end())
    {
      printf("Failed to set pitch for sound %s. Error: Sound not loaded.", soundName.c_str());
      return false;
    }

    return soundMapIt->second->SetPitch(pitch);
  }

  void SetListener(const AudioSystemVector& position, const AudioSystemVector& velocity, const AudioSystemVector& forward, const AudioSystemVector& up)
  {
    if (system)
    {
      system->SetListener(position, velocity, forward, up);
    }
  }

private:

  IAudioSystemSound* GetSound(const std::string& soundName)
  {
    auto soundMapIt = soundMap.find(soundName);
    return soundMapIt != soundMap.end() ? soundMapIt->second : nullptr;
  }

private:

  std::string audioAssetsPath;

  std::unique_ptr<IAudioSystem> system;

  using SoundMap = std::map<std::string, IAudioSystemSound*>;
  SoundMap soundMap;

  friend class AudioManager;
};

#endif

