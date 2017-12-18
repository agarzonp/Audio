#ifndef TXIKI_AUDIO
#define TXIKI_AUDIO

#include <fstream>

class TxikiAudio 
{

  // Wav file format
  struct WavFileFormat_
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

  bool LoadSound(const std::string& soundName)
	{
    std::ifstream iFile(soundName.c_str(), std::ios_base::binary);
    if (!iFile.is_open())
    {
      return false;
    }

    size_t value = 0;

    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.chunkID);
    printf("WavFileFormat.DESCRIPTOR.chunkID: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.chunkSize);
    printf("WavFileFormat.DESCRIPTOR.chunkSize: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DESCRIPTOR.format);
    printf("WavFileFormat.DESCRIPTOR.format: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.subchunk1ID);
    printf("WavFileFormat.FORMAT.subchunk1ID: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.subchunk1Size);
    printf("WavFileFormat.FORMAT.subchunk1Size: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.audioFormat);
    printf("WavFileFormat.FORMAT.audioFormat: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.numChannels);
    printf("WavFileFormat.FORMAT.numChannels: %d\n", value);
    
    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.sampleRate);
    printf("WavFileFormat.FORMAT.sampleRate: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.byteRate);
    printf("WavFileFormat.FORMAT.byteRate: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.blockAlign);
    printf("WavFileFormat.FORMAT.blockAlign: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.FORMAT.bitsPerSample);
    printf("WavFileFormat.FORMAT.bitsPerSample: %d\n", value);
    size_t bitsPerSample = value;

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DATA.subchunk2ID);
    printf("WavFileFormat.DATA.subchunk2ID: %d\n", value);

    value = 0;
    iFile.read(reinterpret_cast<char*>(&value), WavFileFormat.DATA.subchunk2Size);
    printf("WavFileFormat.DATA.subchunk2Size: %d\n", value);
    size_t soundDataSize = value;
    
    size_t bitsPerByte = 8;

    size_t sampleSize = bitsPerSample / bitsPerByte;
    printf("sampleSize : %d\n", sampleSize);

    size_t numSamples = (soundDataSize * bitsPerByte) / bitsPerSample;
    printf("numSamples : %d\n", numSamples);

    for (size_t i = 0; i < numSamples; i++)
    {
      value = 0;
      iFile.read(reinterpret_cast<char*>(&value), sampleSize);
      printf("Sample %d: %d\n", i, value);
    }

    iFile.close();

		return false;
	}

};

#endif // !AUDIO_SYSTEM_TXIKI_AUDIO

