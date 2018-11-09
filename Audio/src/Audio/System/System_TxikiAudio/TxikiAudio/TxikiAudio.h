#ifndef TXIKI_AUDIO_H
#define TXIKI_AUDIO_H

#include <fstream>
#include <functional>
#include <list>
#include <memory>

#include "portaudio/portaudio.h"

enum class TxikiAudioFileFormat
{
  WAVE
};

enum class TxikiAudioSoundFormat : PaSampleFormat
{
  PCM16 = paInt16
};

enum class TxikiAudioSoundSampleRate
{
	SampleRate_44100Hz = 44100
};

class TxikiAudioSound : public IAudioSystemSound
{
public:

	static const size_t NUM_CHANNELS = 2;

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

	float basePitch{ 1.0f };
		
	bool Release() final
  {
		Stop();

    numSamples = 0;

    samples.reset();

		sampleIndex = 0;

    return true;
  }

	bool Play() final
	{  
    state = State::PLAYING;
    return true; 
  }

	bool Stop() final
	{		
		// reset
		sampleIndex = 0;
		state = State::STOPPED;
		volume = 1.0f;
		pitch = basePitch;

    return true;
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

	bool SetVolume(float v) final
	{
		// set the volume in the range [0.0f, 1.0f]
		volume = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    return true;
	}

	bool SetPitch(float p) final
	{
		// set the volume in the range [0.125f, 8.0f] 
		pitch = p < 0.125f ? 0.125f : (p > 8.0f ? 8.0f : p);
    return true;
	}

	void WriteSound(short* outBuffer, size_t framesPerBuffer)
	{
		if (state != TxikiAudioSound::State::PLAYING)
		{
			// do not write data when not playing
			return;
		}

		if (sampleIndex >= numSamples)
		{
			// no more audio data to write
			Stop();
			return;
		}

		size_t samplesPerBuffer = framesPerBuffer * NUM_CHANNELS;
		size_t audioLength = size_t((float(numSamples - sampleIndex)) / pitch);
		auto length = samplesPerBuffer > audioLength ? audioLength : samplesPerBuffer;

		// copy the samples into the buffer (Note: We are only using PCM16 format!)
		float fsampleIndex = (float)sampleIndex;
		for (size_t i = 0; i < length; i++)
		{
			float value = float(samples[(size_t)fsampleIndex]) * volume;
			outBuffer[i] += (short)value;

			fsampleIndex += pitch;
		}

		sampleIndex = audioLength > 1 ? (size_t)fsampleIndex : numSamples;

		return;
	}

  void Set3DAttributes(const AudioSystemVector& position, const AudioSystemVector& velocity) final
  {
    // TO-DO
  }

  void Set3DMinMaxDistance(float minDistance, float maxDistance) final
  {
    // TO-DO
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

    std::ifstream iFile(soundName.c_str(), std::ios_base::binary);
    if (!iFile.is_open())
    {
      printf("Error: Unable to load file %s\n", soundName.c_str());
      return nullptr;
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
      return nullptr;
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

    size_t numChannels = 0;
    iFile.read(reinterpret_cast<char*>(&numChannels), WavFileFormat.FORMAT.numChannels);
    //printf("WavFileFormat.FORMAT.numChannels: %d\n", numChannels);
    
    size_t sampleRate = 0;
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

		// start reading all the samples
		size_t samplesBufferSize = numChannels == 1 ? numSamples * TxikiAudioSound::NUM_CHANNELS : numSamples;
    std::unique_ptr<short[] > samples(new short[samplesBufferSize]);

		size_t d = numChannels == 1 ? 2 : 1;
    for (size_t i = 0, samplesBufferIndex = 0; i < numSamples; i++, samplesBufferIndex += d)
    {
      short sample = 0; // Note: We are using PCM16 format!
      iFile.read(reinterpret_cast<char*>(&sample), sampleSize);
      //printf("Sample %d: %d\n", i, sample);
      samples[samplesBufferIndex] = sample;
			if (numChannels == 1)
			{
				samples[samplesBufferIndex + 1] = sample;
			}
    }

    std::streampos streamPos = iFile.tellg();
    iFile.seekg(0, iFile.end);
    if (streamPos != iFile.tellg())
    {
      printf("Error: Samples in chunk WavFileFormat.DATA not read properly.\n");
      return nullptr;
    }

    iFile.close();

    // get a new TxikiAudioSound
		return GetTxikiAudioSound(samples, samplesBufferSize, sampleRate);
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

		TxikiAudioSound* GetTxikiAudioSound(std::unique_ptr<short[] >& samples, size_t samplesBufferSize, size_t sampleRate)
		{
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

			// set sound data
			sound->numSamples = samplesBufferSize;
			sound->samples = std::move(samples);
			sound->pitch = float(sampleRate) / float(TxikiAudioSoundSampleRate::SampleRate_44100Hz); // Resample to 44100Hz by modifying the pitch
			sound->basePitch = sound->pitch;

			return sound;
		}

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

