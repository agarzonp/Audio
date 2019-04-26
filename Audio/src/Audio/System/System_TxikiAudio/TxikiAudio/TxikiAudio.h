#ifndef TXIKI_AUDIO_H
#define TXIKI_AUDIO_H

#include <fstream>
#include <functional>
#include <list>
#include <memory>

#include "portaudio/portaudio.h"

#include "TxikiAudioEnums.h"
#include "TxikiAudioSound.h"
#include "TxikiAudioSoundLoader.h"



class TxikiAudio 
{
  // sound loader
  TxikiAudioSoundLoader soundLoader;

	// sounds
	std::list<TxikiAudioSound> sounds;

	// handle to PortAudio stream
	PaStream* stream_PCM16{ nullptr }; 

  bool initialised{ false };

public:

  bool Init()
  {
    assert(!initialised);
    if (initialised)
    {
      return false;
    }

    // initialise portaudio
    auto result = Pa_Initialize();
    if (result != paNoError)
    {
      printf("Unable to initialise TxikiAudio. PortAudio error: %s\n", Pa_GetErrorText(result));
      return false;
    }

    StartStream();

    initialised = true;
    return true;
  }

  bool Terminate()
  {
    // release the sounds
		for (auto& sound : sounds)
		{
			sound.Release();
		}
    
		// close the stream
		if (stream_PCM16)
		{
			Pa_CloseStream(stream_PCM16);
			stream_PCM16 = nullptr;
		}

    // terminate portaudio
    auto result = Pa_Terminate();
    if (result != paNoError)
    {
      printf("Unable to terminate TxikiAudio. PortAudio error: %s\n", Pa_GetErrorText(result));
      return false;
    }

    initialised = false;
    return true;
  }

  TxikiAudioSound* LoadSound(const std::string& soundName)
  {
    if (!initialised)
    {
      printf("Error: Unable to load sound %s. TxikiAudio not initialised\n", soundName.c_str());
      return nullptr;
    }

    TxikiAudioSound* sound = nullptr;

    // reuse a not used sound
    for (auto& s : sounds)
    {
      if (!s.samples)
      {
        sound = &s;
        break;
      }
    }

    // create a new sound if all sounds are in use
    if (!sound)
    {
      sounds.emplace_back(TxikiAudioSound());
      sound = &sounds.back();
    }

    if (soundLoader.LoadSound(soundName, *sound))
    {
      return sound;
    }

    return nullptr;
	}
  
  bool UnloadSound(TxikiAudioSound* sound)
  {
    if (!sound)
    {
      return false;
    }

    sound->Release();
    return true;
  }

	protected:

		void WriteSounds(void* outputBuffer, size_t framesPerBuffer)
		{
			// reset buffer (Note: We are using PCM16 format!) 
			short* outBuffer = static_cast<short*>(outputBuffer);
			std::memset(outputBuffer, 0, sizeof(short) * framesPerBuffer * TxikiAudioSound::NUM_CHANNELS);

			// write sounds
			for (auto& sound : sounds)
			{
				sound.WriteSound(outBuffer, framesPerBuffer);
			}
		}

  private:

    static int WriteSoundCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
    {
			TxikiAudio* txikiAudio = static_cast<TxikiAudio*>(userData);
			txikiAudio->WriteSounds(outputBuffer, framesPerBuffer);

			return 0;
    }

		bool StartStream()
		{
			if (!stream_PCM16)
			{
				int inputChannels = 0;
				int outputChannels = TxikiAudioSound::NUM_CHANNELS;
				PaSampleFormat sampleFormat = static_cast<PaSampleFormat>(TxikiAudioSoundFormat::PCM16);
				size_t sampleRate = (size_t)TxikiAudioSoundSampleRate::SampleRate_44100Hz;
				auto framesPerBuffer = paFramesPerBufferUnspecified; // PortAudio will pick the best possible buffer size

				// Open default stream to play the audio
				PaError result = Pa_OpenDefaultStream(&stream_PCM16, inputChannels, outputChannels, sampleFormat, sampleRate, framesPerBuffer, WriteSoundCallback, this);
				if (result != paNoError)
				{
					printf("TxikiAudio unable to open PortAudio stream. PortAudio error: %s\n", Pa_GetErrorText(result));
					return false;
				}

				// Start audio stream
				result = Pa_StartStream(stream_PCM16);
				if (result != paNoError)
				{
					printf("TxikiAudio unable to PlaySound. PortAudio error: %s\n", Pa_GetErrorText(result));
					return false;
				}
			}

			return true;
		}
};

#endif // !TXIKI_AUDIO_H

