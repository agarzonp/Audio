#ifndef AUDIO_SYSTEM_TXIKI_AUDIO
#define AUDIO_SYSTEM_TXIKI_AUDIO

#include "TxikiAudio/TxikiAudio.h"

class AudioSystemTxikiAudio : public IAudioSystem
{
	TxikiAudio txikiAudio;

public:

	void Initialise() override 
	{
		// TO-DO
	}

	void Deinitialise() override {  /* nothing to do here */ }

	void Update() override 
	{
		// TO-DO
	}

  bool LoadSound(const std::string& soundName, AudioSystemSound& outSound) override
	{
		bool loaded = txikiAudio.LoadSound(soundName);
		if (!loaded)
		{
			printf("Failed to load sound %s \n", soundName.c_str());
		}
		return loaded;
	}

	bool UnloadSound(const AudioSystemSound& audioSystemSound) override
	{
		// TO-DO
		return false;
	}

	bool PlaySound(AudioSystemSound& audioSystemSound) override
	{
		// TO-DO
		return false;
	}

	bool StopSound(const AudioSystemSound& audioSystemSound) override
	{
		// TO-DO
		return false;
	}

	bool PauseSound(const AudioSystemSound& audioSystemSound, bool pause) override
	{
		// TO-DO
		return false;
	}

	bool SetSoundVolume(const AudioSystemSound& audioSystemSound, float volume) override
	{
		// TO-DO
		return false;
	}
};

#endif // !AUDIO_SYSTEM_TXIKI_AUDIO

