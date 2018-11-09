#ifndef AUDIO_SYSTEM_SOUND_FMOD_H
#define AUDIO_SYSTEM_SOUND_FMOD_H

class AudioSystemSoundFMOD : public IAudioSystemSound
{
public:

  AudioSystemSoundFMOD(FMOD::Sound* s) : sound(s) {}

  bool Play() final
  {
    bool paused = false;
    FMOD_RESULT result = s_system->playSound(sound, nullptr, paused, &channel);
    if (result != FMOD_OK)
    {
      printf("Failed to play sound. Error: %s \n", FMOD_ErrorString(result));
    }

    return (result == FMOD_OK);
  }

  bool Stop() final
  {
    if (!channel)
    {
      printf("Failed to %s. Sound has null channel.\n", "Stop");
      return false;
    }

    FMOD_RESULT result = channel->stop();
    if (result != FMOD_OK)
      printf("Failed to stop sound. Error: %s \n", FMOD_ErrorString(result));

    return (result == FMOD_OK);
  }

  bool Pause(bool pause) final
  {
    if (!channel)
    {
      printf("Failed to %s. Sound has null channel.\n", "Pause");
      return false;
    }

    FMOD_RESULT result = channel->setPaused(pause);
    if (result != FMOD_OK)
      printf("Failed to %s sound. Error: %s \n", pause ? "pause" : "resume", FMOD_ErrorString(result));

    return (result == FMOD_OK);
  }

  bool Release() final
  {
    if (sound)
    {
      sound->release();
      return true;
    }

    return false;
  }

  bool SetVolume(float volume) final
  {
    if (!channel)
    {
      printf("Failed to %s. Sound has null channel.\n", "SetVolume");
      return false;
    }

    FMOD_RESULT result = channel->setVolume(volume);
    if (result != FMOD_OK)
      printf("Failed to set volume for sound. Error: %s \n", FMOD_ErrorString(result));

    return (result == FMOD_OK);
  }

  bool SetPitch(float pitch) final
  {
    if (!channel)
    {
      printf("Failed to %s. Sound has null channel.\n", "SetPitch");
      return false;
    }

    FMOD_RESULT result = channel->setPitch(pitch);
    if (result != FMOD_OK)
      printf("Failed to set pitch for sound. Error: %s \n", FMOD_ErrorString(result));

    return (result == FMOD_OK);
  }

  void Set3DAttributes(const AudioSystemVector& position, const AudioSystemVector& velocity) final
  {
    if (!channel)
    {
      printf("Failed to %s. Sound has null channel.\n", "Set3DAttributes");
      return;
    }

    const FMOD_VECTOR* pos = reinterpret_cast<const FMOD_VECTOR*> (&position);
    const FMOD_VECTOR* vel = reinterpret_cast<const FMOD_VECTOR*> (&velocity);
    channel->set3DAttributes(pos, vel);
  }

  void Set3DMinMaxDistance(float minDistance, float maxDistance) final
  {
    if (!channel)
    {
      printf("Failed to %s. Sound has null channel.\n", "Set3DMinMaxDistance");
      return;
    }
  }

private:

  static FMOD::System* s_system;

  FMOD::Sound* sound{ nullptr };
  FMOD::Channel* channel{ nullptr };

  friend class AudioSystemFMOD;
};

FMOD::System* AudioSystemSoundFMOD::s_system = nullptr;

#endif // !AUDIO_SYSTEM_SOUND_FMOD_H

