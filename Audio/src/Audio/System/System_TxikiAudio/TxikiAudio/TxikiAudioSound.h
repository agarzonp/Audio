#ifndef TXIKI_AUDIO_SOUND_H
#define TXIKI_AUDIO_SOUND_H

#include <memory>

#include "..\..\System_Common\AudioSystemCommon.h"

class TxikiAudioSound : public IAudioSystemSound
{
public:

  static const size_t NUM_CHANNELS = 2;

  size_t numSamples{ 0 };

  std::unique_ptr< short[] > samples;

  size_t sampleIndex{ 0 };

  enum class State
  {
    PLAYING,
    PAUSED,
    STOPPED
  };

  State state{ State::STOPPED };

  float volume{ 1.0f };
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

  bool Pause(bool pause) final
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

#endif // !TXIKI_AUDIO_SOUND_H

