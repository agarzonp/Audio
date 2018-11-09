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

  IAudioSystemSound* LoadSound(const std::string& soundName, AudioSystemSoundMode audioSystemSoundMode) final
  {
    return txikiAudio.LoadSound(soundName);
  }

  bool UnloadSound(IAudioSystemSound* audioSystemSound) final
  {
    TxikiAudioSound* sound = static_cast<TxikiAudioSound*>(audioSystemSound);
    return txikiAudio.UnloadSound(sound);
  }
};

#endif // !AUDIO_SYSTEM_TXIKI_AUDIO

