#ifndef TXIKI_AUDIO_SOUND_LOADER_H
#define TXIKI_AUDIO_SOUND_LOADER_H

#include "TxikiAudioEnums.h"
#include "TxikiAudioSound.h"


struct TxikiAudioSoundDesc
{
  TxikiAudioSoundFormat format; 

  std::unique_ptr<short[] > samples;
  size_t samplesBufferSize{ 0 };
  size_t sampleRate{ 0 };
};

class ISoundFileReader
{
public:
  virtual bool Read(std::ifstream& soundFile, TxikiAudioSoundDesc& outTxikiAudioSoundDesc) = 0;
};

class SoundFileReader_WAVE : public ISoundFileReader
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

public:

  bool Read(std::ifstream& soundFile, TxikiAudioSoundDesc& outTxikiAudioSoundDesc) final
  {
    size_t value = 0;

    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.chunkID);
    //printf("WavFileFormat.DESCRIPTOR.chunkID: %d\n", value);

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.chunkSize);
    //printf("WavFileFormat.DESCRIPTOR.chunkSize: %d\n", value);

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.format);
    //printf("WavFileFormat.DESCRIPTOR.format: %d\n", value);

    if (!soundFile.good())
    {
      printf("Error: Chunk WavFileFormat.DESCRIPTOR not read properly.\n");
      return false;
    }

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.subchunk1ID);
    //printf("WavFileFormat.FORMAT.subchunk1ID: %d\n", value);

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.subchunk1Size);
    //printf("WavFileFormat.FORMAT.subchunk1Size: %d\n", value);

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.audioFormat);
    //printf("WavFileFormat.FORMAT.audioFormat: %d\n", value);

    switch (value)
    {
    case 1:
      outTxikiAudioSoundDesc.format = TxikiAudioSoundFormat::PCM16;
      break;
    default:
      outTxikiAudioSoundDesc.format = TxikiAudioSoundFormat::NONE;
      printf("Error: WavFileFormat.FORMAT.audioFormat: %d not supported. Only PCM16 is supported.\n", value);
      return false;
    }

    size_t numChannels = 0;
    soundFile.read(reinterpret_cast<char*>(&numChannels), WavFileFormat.FORMAT.numChannels);
    //printf("WavFileFormat.FORMAT.numChannels: %d\n", numChannels);

    size_t sampleRate = 0;
    soundFile.read(reinterpret_cast<char*>(&sampleRate), WavFileFormat.FORMAT.sampleRate);
    outTxikiAudioSoundDesc.sampleRate = sampleRate;
    //printf("WavFileFormat.FORMAT.sampleRate: %d\n", sampleRate);

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.byteRate);
    //printf("WavFileFormat.FORMAT.byteRate: %d\n", value);

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.blockAlign);
    // printf("WavFileFormat.FORMAT.blockAlign: %d\n", value);

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.bitsPerSample);
    //printf("WavFileFormat.FORMAT.bitsPerSample: %d\n", value);
    size_t bitsPerSample = value;

    switch (outTxikiAudioSoundDesc.format)
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

    if (!soundFile.good())
    {
      printf("Error: Chunk WavFileFormat.FORMAT not read properly.\n");
      return false;
    }

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DATA.subchunk2ID);
    //printf("WavFileFormat.DATA.subchunk2ID: %d\n", value);

    value = 0;
    soundFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DATA.subchunk2Size);
    //printf("WavFileFormat.DATA.subchunk2Size: %d\n", value);

    if (!soundFile.good())
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
    outTxikiAudioSoundDesc.samplesBufferSize = samplesBufferSize;

    std::unique_ptr<short[] > samples(new short[samplesBufferSize]);

    size_t d = numChannels == 1 ? 2 : 1;
    for (size_t i = 0, samplesBufferIndex = 0; i < numSamples; i++, samplesBufferIndex += d)
    {
      short sample = 0; // Note: We are using PCM16 format!
      soundFile.read(reinterpret_cast<char*>(&sample), sampleSize);
      //printf("Sample %d: %d\n", i, sample);
      samples[samplesBufferIndex] = sample;
      if (numChannels == 1)
      {
        samples[samplesBufferIndex + 1] = sample;
      }
    }
    outTxikiAudioSoundDesc.samples = std::move(samples);

    std::streampos streamPos = soundFile.tellg();
    soundFile.seekg(0, soundFile.end);
    if (streamPos != soundFile.tellg())
    {
      soundFile.close();
      printf("Error: Samples in chunk WavFileFormat.DATA not read properly.\n");
      return false;
    }

    return true;
  }
};

class SoundFileReaderFactory
{
  public:
    static std::unique_ptr<ISoundFileReader> NewSoundFileReader(TxikiAudioFileFormat format)
    {
      switch (format)
      {
      case TxikiAudioFileFormat::WAVE:
        return std::make_unique<SoundFileReader_WAVE>();

      case TxikiAudioFileFormat::NONE:
      case TxikiAudioFileFormat::NUM_FILE_FORMATS:
      default:
        return nullptr;
      }

      return nullptr;
    }
};

class TxikiAudioSoundLoader
{
public:

  TxikiAudioSoundLoader()
  {
    soundFileReaders.resize(static_cast<size_t>(TxikiAudioFileFormat::NUM_FILE_FORMATS));
    for (size_t i = 0; i < soundFileReaders.size(); i++)
    {
      soundFileReaders[i] = SoundFileReaderFactory::NewSoundFileReader(TxikiAudioFileFormat(i));
    }
  }

  bool LoadSound(const std::string& soundName, TxikiAudioSound& outSound)
  {
    TxikiAudioFileFormat fileFormat = TxikiAudioFileFormat::NONE;

    // TO-DO: detect file format
    fileFormat = TxikiAudioFileFormat::WAVE;
    if (fileFormat == TxikiAudioFileFormat::NONE)
    {
      printf("Error: Unable to load sound %s. TxikiAudioFileFormat::NONE\n", soundName.c_str());
      return false;
    }
    
    std::ifstream iFile(soundName.c_str(), std::ios_base::binary);
    if (!iFile.is_open())
    {
      printf("Error: Unable to load file %s\n", soundName.c_str());
      return false;
    }

    TxikiAudioSoundDesc soundDesc;

    auto& soundFileReader = soundFileReaders[static_cast<size_t>(fileFormat)];
    if (!soundFileReader->Read(iFile, soundDesc))
    {
      printf("Error: Unable to read file %s with format %d\n", soundName.c_str(), static_cast<int>(fileFormat));
      return false;
    }

    // set sound data
    outSound.numSamples = soundDesc.samplesBufferSize;
    outSound.samples = std::move(soundDesc.samples);
    outSound.pitch = float(soundDesc.sampleRate) / float(TxikiAudioSoundSampleRate::SampleRate_44100Hz); // Resample to 44100Hz by modifying the pitch
    outSound.basePitch = outSound.pitch;

    return true;
  }

private:

  std::vector<std::unique_ptr<ISoundFileReader>> soundFileReaders;
};

#endif // !TXIKI_AUDIO_SOUND_LOADER_H

