#ifndef AUDIO_SYSTEM_TXIKI_AUDIO
#define AUDIO_SYSTEM_TXIKI_AUDIO

#include "TxikiAudio/TxikiAudio.h"

class AudioSystemTxikiAudio : public IAudioSystem
{
	TxikiAudio txikiAudio;

public:

	void Initialise() override 
	{
    txikiAudio.Init();
	}

	void Deinitialise() override 
  {  
    txikiAudio.Terminate();
  }

	void Update() override 
	{
		// TO-DO
	}

  bool LoadSound(const std::string& soundName, AudioSystemSound& outSound) override
	{
    TxikiAudioSound* sound = nullptr;
		bool loaded = txikiAudio.LoadSound(soundName, sound);
		if (!loaded)
		{
			printf("Failed to load sound %s \n", soundName.c_str());
		}

    outSound = AudioSystemSound(sound);

		return loaded;
	}

	bool UnloadSound(const AudioSystemSound& audioSystemSound) override
	{
    TxikiAudioSound* sound = static_cast<TxikiAudioSound*>(audioSystemSound.Get());
    return txikiAudio.UnloadSound(sound);
	}

	bool PlaySound(AudioSystemSound& audioSystemSound) override
	{
    TxikiAudioSound* sound = static_cast<TxikiAudioSound*>(audioSystemSound.Get());
    return txikiAudio.PlaySound(sound);
	}

	bool StopSound(const AudioSystemSound& audioSystemSound) override
	{
    TxikiAudioSound* sound = static_cast<TxikiAudioSound*>(audioSystemSound.Get());
    return txikiAudio.StopSound(sound);
	}

	bool PauseSound(const AudioSystemSound& audioSystemSound, bool pause) override
	{
		TxikiAudioSound* sound = static_cast<TxikiAudioSound*>(audioSystemSound.Get());
		return txikiAudio.PauseSound(sound, pause);
	}

	bool SetSoundVolume(const AudioSystemSound& audioSystemSound, float volume) override
	{
		TxikiAudioSound* sound = static_cast<TxikiAudioSound*>(audioSystemSound.Get());
		return txikiAudio.SetVolume(sound, volume);
	}

	bool SetSoundPitch(const AudioSystemSound& audioSystemSound, float pitch) override
	{
		TxikiAudioSound* sound = static_cast<TxikiAudioSound*>(audioSystemSound.Get());
		return txikiAudio.SetPitch(sound, pitch);
	}
};

#endif // !AUDIO_SYSTEM_TXIKI_AUDIO

