#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <memory>

#include "System_Common\AudioSystemCommon.h"
#include "System_FMOD\AudioSystemFMOD.h"
#include "System_TxikiAudio\AudioSystemTxikiAudio.h"


// AudioSystemType
enum class AudioSystemType
{
  FMOD,
  TXIKI_AUDIO
};

// AudioSystem
//
class AudioSystem
{
public:

  struct InitParams
  {
    AudioSystemType audioSystemType{ AudioSystemType::FMOD };
    size_t soundPoolSise;
    const char* audioAssetsPath;
  };


  void Initialise(const InitParams& params)
  {
    // init system
    InitSystem(params.audioSystemType);
    
    // set sound pool size
    soundPool.SetSize(params.soundPoolSise);

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
      if (system->UnloadSound(*entry.second))
      {
        soundPool.Release(*entry.second);
      }
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

    AudioSystemSound* sound = soundPool.GetFree();
    if (!sound)
    {
      printf("Failed to load sound. Error: %s \n", "soundsPool is full. Consider to increase the size of the pool");
      return false;
    }

    std::string soundPath = audioAssetsPath + soundName;
    if (system->LoadSound(soundPath, soundMode, *sound))
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

    if (system->UnloadSound(*soundMapIt->second))
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

    return system->PlaySound(*soundMapIt->second);
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

    return system->StopSound(*soundMapIt->second);
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

    return system->PauseSound(*soundMapIt->second, pause);
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

    return system->SetSoundVolume(*soundMapIt->second, volume);
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

    return system->SetSoundPitch(*soundMapIt->second, pitch);
  }

  void SetListener(const AudioSystemVector& position, const AudioSystemVector& velocity, const AudioSystemVector& forward, const AudioSystemVector& up)
  {
    if (system)
    {
      system->SetListener(position, velocity, forward, up);
    }
  }

private:

  AudioSystemSound* GetSound(const std::string& soundName)
  {
    auto soundMapIt = soundMap.find(soundName);
    return soundMapIt != soundMap.end() ? soundMapIt->second : nullptr;
  }

  void InitSystem(AudioSystemType type)
  {
    // check that we haven't initialised it before
    assert(system == nullptr);
    if (system)
    {
      return;
    }

    switch (type)
    {
    case AudioSystemType::FMOD:
      system = std::make_unique<AudioSystemFMOD>();
      break;
    case AudioSystemType::TXIKI_AUDIO:
      system = std::make_unique<AudioSystemTxikiAudio>();
      break;
    }

    system->Initialise();
  }

private:

  std::string audioAssetsPath;

  std::unique_ptr<IAudioSystem> system;
  AudioSystemSoundPool soundPool;

  using SoundMap = std::map<std::string, AudioSystemSound*>;
  SoundMap soundMap;

  friend class AudioManager;
};

#endif

