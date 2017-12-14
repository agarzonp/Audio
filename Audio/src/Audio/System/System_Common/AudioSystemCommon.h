#ifndef AUDIO_SYSTEM_COMMON_H
#define AUDIO_SYSTEM_COMMON_H

#include <string>
#include <vector>

// A wrapper for the actual system sound
class AudioSystemSound
{
public:
  AudioSystemSound() = default;
  AudioSystemSound(void* sound_) : sound(sound_) {}
  
  void* Get() const { return sound; }

	void SetSoundControl(void* soundControl_) { soundControl = soundControl_; }
	void* GetSoundControl() const { return soundControl; }

protected:
  
	void Release() { sound = nullptr; soundControl = nullptr; }

private:

  void* sound { nullptr };
	void* soundControl{ nullptr };

  // Only the AudioSystemSoundPool is able to release current sound
  friend class AudioSystemSoundPool;
};

// Basic pool of AudioSystemSounds
class AudioSystemSoundPool
{
public:

	void SetSize(size_t size)
	{
		sounds.resize(size);
	}

  AudioSystemSound* GetFree()
  {
    AudioSystemSound* sound = nullptr;

    for (auto& s : sounds)
    {
      if (s.Get() == nullptr)
      {
        sound = &s;
        break;
      }
    }

    return sound;
  }

  void Release(AudioSystemSound& sound)
  {
    sound.Release();
  }

private:

    std::vector<AudioSystemSound> sounds;
};

// Interface for the AudioSystem
class IAudioSystem
{
public:

	virtual void Initialise() = 0;
	virtual void Deinitialise() = 0;

	virtual void Update() = 0;

	virtual bool LoadSound(const std::string& soundName, AudioSystemSound& outSound) = 0;
	virtual bool UnloadSound(const AudioSystemSound& audioSystemSound) = 0;

	virtual bool PlaySound(AudioSystemSound& audioSystemSound) = 0;
	virtual bool StopSound(const AudioSystemSound& audioSystemSound) = 0;
	virtual bool PauseSound(const AudioSystemSound& audioSystemSound, bool pause) = 0;

protected:

  
};

#endif // !AUDIO_SYSTEM_IMPL_H

