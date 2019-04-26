#ifndef TXIKI_AUDIO_ENUMS_H
#define TXIKI_AUDIO_ENUMS_H

enum class TxikiAudioFileFormat
{
  NONE = -1,

  WAVE,

  NUM_FILE_FORMATS
};

enum class TxikiAudioSoundFormat : PaSampleFormat
{
  NONE,

  PCM16 = paInt16
};

enum class TxikiAudioSoundSampleRate
{
  NONE,

  SampleRate_44100Hz = 44100
};

#endif // !TXIKI_AUDIO_ENUMS_H

