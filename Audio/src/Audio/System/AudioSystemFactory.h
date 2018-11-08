#ifndef AUDIO_SYSTEM_FACTORY_H
#define AUDIO_SYSTEM_FACTORY_H

#include "System_Common\AudioSystemCommon.h"
#include "System_FMOD\AudioSystemFMOD.h"
#include "System_TxikiAudio\AudioSystemTxikiAudio.h"

// AudioSystemType
enum class AudioSystemType
{
  FMOD,
  TXIKI_AUDIO
};

// AudioSystemFactory
class AudioSystemFactory
{
public:

  static std::unique_ptr<IAudioSystem> NewSystem(AudioSystemType type)
  {
    std::unique_ptr<IAudioSystem> system;

    switch (type)
    {
    case AudioSystemType::FMOD:
      system = std::make_unique<AudioSystemFMOD>();
      break;
    case AudioSystemType::TXIKI_AUDIO:
      system = std::make_unique<AudioSystemTxikiAudio>();
      break;
    }

    return system;
  }
};

#endif