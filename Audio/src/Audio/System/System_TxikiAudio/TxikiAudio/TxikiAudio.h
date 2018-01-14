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
  unsigned int sampleRate { 44100 };
  unsigned int numChannels { 2 };
  unsigned int numSamples{ 0 };

  std::unique_ptr< short[] > samples;

	size_t sampleIndex { 0 };

  void Release()
  {
    sampleRate = 44100;
    numChannels = 2;
    numSamples = 0;

    samples.reset();
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
  PaStream* stream { nullptr };

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
    // close the stream and release the sound
    if (stream)
    {
      Pa_CloseStream(stream);
    }
    sound.Release();

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
    int numInputChannels = 0;
    int numOutputChannels = sound->numChannels;
    PaSampleFormat sampleFormat = paInt16; // for the moment we only allow TxikiAudioSoundFormat::PCM16
    double sampleRate = sound->sampleRate;
    unsigned long bufferLength = paFramesPerBufferUnspecified; // PortAudio will pick the best possible buffer size
    PaStreamCallback* callback = &TxikiAudio::WriteSoundCallback;
    
    // Open default stream to play the audio
    PaError result = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels, paInt16, sampleRate, bufferLength, callback, sound);
    if (result != paNoError)
    {
      printf("TxikiAudio unable to PlaySound. PortAudio error: %s\n", Pa_GetErrorText(result));
      return false;
    }

    // Start audio stream
    result = Pa_StartStream(stream);
    if (result != paNoError)
    {
      printf("TxikiAudio unable to PlaySound. PortAudio error: %s\n", Pa_GetErrorText(result));
      return false;
    }

    return true;
  }

  bool StopSound(TxikiAudioSound* sound)
  {
    // abort current stream
    PaError result = Pa_AbortStream(stream);
    if (result != paNoError)
    {
      printf("TxikiAudio unable to StopSound. PortAudio error: %s\n", Pa_GetErrorText(result));
      return false;
    }

    // close the stream
    result = Pa_CloseStream(stream);
    if (result != paNoError)
    {
      printf("TxikiAudio unable to PlaySound. PortAudio error: %s\n", Pa_GetErrorText(result));
      return false;
    }

    // release current sound
    sound->Release();

    return true;
  }

  private:

    static int WriteSoundCallback(const void *inputBuffer, void *outputBuffer, unsigned long bufferLength, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
    {
			// reset buffer (Note: We are using PCM16 format!) 
			short* outBuffer = static_cast<short*>(outputBuffer);
			std::memset(outputBuffer, 0, bufferLength * 2); 

			TxikiAudioSound* sound = static_cast<TxikiAudioSound*>(userData);
			if (sound->sampleIndex >= sound->numSamples)
			{
				// no more audio data to write
				return 0;
			}

			auto audioLength = sound->numSamples - sound->sampleIndex;
			auto emptyBufferFrames = bufferLength - audioLength;
			auto length = bufferLength > audioLength ? audioLength : bufferLength;
			
			// copy the samples into the buffer (Note: We are using PCM16 format!)
			#define USE_MEMCPY_FIXED 0
			#if USE_MEMCPY_FIXED
			std::memcpy(outputBuffer, sound->samples.get() + (sizeof(sound->samples[0]) * sound->sampleIndex), length * 2);
			sound->sampleIndex += length;
			#else
			// copy the samples into the buffer
			for (size_t i = 0; i < length; i++, sound->sampleIndex++)
			{

				outBuffer[i] = sound->samples[sound->sampleIndex];
			}
			#endif // USE_MEMCPY_FIXED

      return 0;
    }


};

#endif // !AUDIO_SYSTEM_TXIKI_AUDIO

