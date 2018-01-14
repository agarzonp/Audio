#ifndef TXIKI_AUDIO
#define TXIKI_AUDIO

#include <fstream>
#include <functional>
#include <memory>
#include "portaudio/portaudio.h"

enum class TxikiAudioFileFormat
{
  WAVE
};

enum class TxikiAudioSoundFormat
{
  PCM16
};

struct TxikiAudioSound
{
  size_t sampleRate { 44100 };
	size_t numChannels { 2 };
	size_t numSamples{ 0 };

  std::unique_ptr< short[] > samples;

	size_t sampleIndex { 0 };

	enum class State
	{
		PLAYING,
		PAUSED,
		STOPPED
	};

	State state{ State::STOPPED };

	float volume { 1.0f };
	float pitch{ 1.0f };
		
	void Release()
  {
		Stop();

    sampleRate = 44100;
    numChannels = 2;
    numSamples = 0;

    samples.reset();

		sampleIndex = 0;
  }

	void Play()
	{
		state = State::PLAYING;
	}

	void Stop()
	{		
		// reset
		sampleIndex = 0;
		state = State::STOPPED;
		volume = 1.0f;
		pitch = 1.0f;
	}

	bool Pause(bool pause)
	{
		if (state == State::STOPPED)
		{
			return false;
		}

		state = pause ? State::PAUSED : State::PLAYING;
		return true;
	}

	void SetVolume(float v)
	{
		// set the volume in the range [0.0f, 1.0f]
		volume = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
	}

	void SetPitch(float p)
	{
		// set the volume in the range [0.125f, 8.0f] 
		pitch = p < 0.125f ? 0.125f : (p > 8.0f ? 8.0f : p);
	}

	void WriteSound(short* outBuffer, size_t bufferLength)
	{
		if (sampleIndex >= numSamples)
		{
			// no more audio data to write
			Stop();
			return;
		}

		if (state != TxikiAudioSound::State::PLAYING)
		{
			// do not write data when not playing
			return;
		}

		size_t audioLength = size_t((float(numSamples - sampleIndex)) / pitch);
		auto length = bufferLength > audioLength ? audioLength : bufferLength;

		// copy the samples into the buffer (Note: We are only using PCM16 format!)
		float fsampleIndex = (float)sampleIndex;
		for (size_t i = 0; i < length; i++)
		{
			float value = float(samples[(size_t)fsampleIndex]) * volume;
			outBuffer[i] = (short)value;

			fsampleIndex += pitch;
		}

		sampleIndex = audioLength > 1 ? (size_t)fsampleIndex : numSamples;

		return;
	}
};

class TxikiAudio 
{
  // Wave file format
  struct WaveFileFormat_
  {
    // 1st chunk
    // DESCRIPTOR: RIFF file type
    struct DESCRIPTOR_
    {
      std::streamsize chunkID{ 4 }; // "FFIR" = 0x46464952 = 1179011410 ("RIFF" in Big Endian)
      std::streamsize chunkSize{ 4 };
      std::streamsize format{ 4 }; // "EVAW" = 0x45564157 = 1163280727 ("WAVE" in Big Endian)
    }DESCRIPTOR;
    

    // 2nd chunk
    // FORMAT: data format
    struct FORMAT_
    {
      std::streamsize subchunk1ID{ 4 }; // "tmf" = 0x20746D66 = 544501094 ("fmt" in Big Endian)
      std::streamsize subchunk1Size{ 4 }; // 16 = audioFormat(2) + numChannels(2) + sampleRate(4) + byteRate(4) + blockAlign(2) + bitsPerSample(2)
      std::streamsize audioFormat{ 2 }; // PCM = 0x0001 = 1
      std::streamsize numChannels{ 2 };
      std::streamsize sampleRate{ 4 };
      std::streamsize byteRate{ 4 };
      std::streamsize blockAlign{ 2 };
      std::streamsize bitsPerSample{ 2 };
    }FORMAT;
   

    // 3rd chunk
    // DATA: sound data
    struct DATA_
    {
      std::streamsize subchunk2ID{ 4 }; //  "atad" = 0x61746164 = 1635017060 ("data in Big Endian")
      std::streamsize subchunk2Size{ 4 };
      // std::streamsize data { X }
    }DATA;
   
  } WavFileFormat;

  TxikiAudioSound sound;

	PaStream* stream_1outputChannel_PCM16_sampleRate11025{ nullptr }; // handle to PortAudio stream

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

    initialised = true;
    return true;
  }

  bool Terminate()
  {
    // release the sound
    sound.Release();

		// close the stream
		if (stream_1outputChannel_PCM16_sampleRate11025)
		{
			Pa_CloseStream(stream_1outputChannel_PCM16_sampleRate11025);
			stream_1outputChannel_PCM16_sampleRate11025 = nullptr;
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

  bool LoadSound(const std::string& soundName, TxikiAudioSound*& outSound)
	{
    if (!initialised)
    {
      return false;
    }

    std::ifstream iFile(soundName.c_str(), std::ios_base::binary);
    if (!iFile.is_open())
    {
      printf("Error: Unable to load file %s\n", soundName.c_str());
      return false;
    }

    size_t value = 0;

    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.chunkID);
    //printf("WavFileFormat.DESCRIPTOR.chunkID: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.chunkSize);
    //printf("WavFileFormat.DESCRIPTOR.chunkSize: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.format);
    //printf("WavFileFormat.DESCRIPTOR.format: %d\n", value);

    if (!iFile.good())
    {
      printf("Error: Chunk WavFileFormat.DESCRIPTOR not read properly.\n");
      return false;
    }

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.subchunk1ID);
    //printf("WavFileFormat.FORMAT.subchunk1ID: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.subchunk1Size);
    //printf("WavFileFormat.FORMAT.subchunk1Size: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.audioFormat);
    //printf("WavFileFormat.FORMAT.audioFormat: %d\n", value);

    TxikiAudioSoundFormat audioFormat;
    switch (value)
    {
    case 1:
      audioFormat = TxikiAudioSoundFormat::PCM16;
      break;
    default:
      printf("Error: WavFileFormat.FORMAT.audioFormat: %d not supported. Only PCM16 is supported.\n", value);
      return false;
    }

    unsigned int numChannels = 0;
    iFile.read(reinterpret_cast<char*>(&numChannels), WavFileFormat.FORMAT.numChannels);
    //printf("WavFileFormat.FORMAT.numChannels: %d\n", numChannels);
    
    unsigned int sampleRate = 0;
    iFile.read(reinterpret_cast<char*>(&sampleRate), WavFileFormat.FORMAT.sampleRate);
    //printf("WavFileFormat.FORMAT.sampleRate: %d\n", sampleRate);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.byteRate);
    //printf("WavFileFormat.FORMAT.byteRate: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.blockAlign);
   // printf("WavFileFormat.FORMAT.blockAlign: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.bitsPerSample);
    //printf("WavFileFormat.FORMAT.bitsPerSample: %d\n", value);
    size_t bitsPerSample = value;
    
    switch (audioFormat)
    {
    case TxikiAudioSoundFormat::PCM16:
      if (bitsPerSample != 16)
      {
        printf("Error: WavFileFormat.FORMAT.bitsPerSample is %d with format PCM16\n", bitsPerSample);
        return false;
      }
      break;
    default:
      printf("Error: audioFormat not handled when reading WavFileFormat.FORMAT.bitsPerSample %d\n", bitsPerSample);
      return false;
    }

    if (!iFile.good())
    {
      printf("Error: Chunk WavFileFormat.FORMAT not read properly.\n");
      return false;
    }

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DATA.subchunk2ID);
    //printf("WavFileFormat.DATA.subchunk2ID: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DATA.subchunk2Size);
    //printf("WavFileFormat.DATA.subchunk2Size: %d\n", value);

    if (!iFile.good())
    {
      printf("Error: Chunk WavFileFormat.DATA not read properly.\n");
      return false;
    }

    size_t soundDataSize = value;
    size_t bitsPerByte = 8;

    size_t sampleSize = bitsPerSample / bitsPerByte;
    //printf("sampleSize : %d\n", sampleSize);

    size_t numSamples = (soundDataSize * bitsPerByte) / bitsPerSample;
    //printf("numSamples : %d\n", numSamples);

    std::unique_ptr<short[] > samples(new short[numSamples]);

    // start reading all the samples
    for (size_t i = 0; i < numSamples; i++)
    {
      short sample = 0;
      iFile.read(reinterpret_cast<char*>(&sample), sampleSize);
      //printf("Sample %d: %d\n", i, sample);
      samples[i] = sample;
    }

    std::streampos streamPos = iFile.tellg();
    iFile.seekg(0, iFile.end);
    if (streamPos != iFile.tellg())
    {
      printf("Error: Samples in chunk WavFileFormat.DATA not read properly.\n");
      return false;
    }

    iFile.close();

    // fill sound data
    sound.sampleRate = sampleRate;
    sound.numChannels = numChannels;
    sound.numSamples = numSamples;
    sound.samples = std::move(samples);
    outSound = &sound;

		return true;
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

  bool PlaySound(TxikiAudioSound* sound)
  {
		if (StartStream(sound))
		{
			sound->Play();
			return true;
		}
		
		return false;
  }

  bool StopSound(TxikiAudioSound* sound)
  {
		sound->Stop();
		return true;
  }

	bool PauseSound(TxikiAudioSound* sound, bool pause)
	{
		return sound->Pause(pause);
	}

	bool SetVolume(TxikiAudioSound* sound, float volume)
	{
		sound->SetVolume(volume);
		return true;
	}

	bool SetPitch(TxikiAudioSound* sound, float pitch)
	{
		sound->SetPitch(pitch);
		return true;
	}

	protected:

		void WriteSounds(void* outputBuffer, size_t bufferLength)
		{
			// reset buffer (Note: We are using PCM16 format!) 
			short* outBuffer = static_cast<short*>(outputBuffer);
			std::memset(outputBuffer, 0, bufferLength * 2);

			sound.WriteSound(outBuffer, bufferLength);
		}

  private:

    static int WriteSoundCallback(const void *inputBuffer, void *outputBuffer, unsigned long bufferLength, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
    {
			TxikiAudio* txikiAudio = static_cast<TxikiAudio*>(userData);
			txikiAudio->WriteSounds(outputBuffer, bufferLength);

			return 0;
    }

		bool StartStream(TxikiAudioSound* sound)
		{
			if (sound->numChannels != 1 || sound->sampleRate != 11025)
			{
				// FIXME: Make this more generic
				printf("Only 1 output channel and 11025 sample rate is supported, Sound numChannels (%d) sampleRate(%d)", sound->numChannels, sound->sampleRate);
				return false;
			}

			if (!stream_1outputChannel_PCM16_sampleRate11025)
			{
				// FIXME: for the moment we only allow this kind of configuration
				int numInputChannels = 0;
				int numOutputChannels = 1;
				PaSampleFormat sampleFormat = paInt16;
				size_t sampleRate = 11025;
				auto bufferLength = paFramesPerBufferUnspecified; // PortAudio will pick the best possible buffer size

				// Open default stream to play the audio
				PaError result = Pa_OpenDefaultStream(&stream_1outputChannel_PCM16_sampleRate11025, numInputChannels, numOutputChannels, sampleFormat, sampleRate, bufferLength, WriteSoundCallback, this);
				if (result != paNoError)
				{
					printf("TxikiAudio unable to open PortAudio stream. PortAudio error: %s\n", Pa_GetErrorText(result));
					return false;
				}

				// Start audio stream
				result = Pa_StartStream(stream_1outputChannel_PCM16_sampleRate11025);
				if (result != paNoError)
				{
					printf("TxikiAudio unable to PlaySound. PortAudio error: %s\n", Pa_GetErrorText(result));
					return false;
				}
			}

			return true;
		}
};

#endif // !AUDIO_SYSTEM_TXIKI_AUDIO

