#ifndef AUDIO_SYSTEM_COMMON_H
#define AUDIO_SYSTEM_COMMON_H

#include <string>
#include <vector>

#include "AudioSystemDefines.h"

// Interface for AudioSystemSound
class IAudioSystemSound
{
public:

  virtual bool Play() = 0;
  virtual bool Stop() = 0;
  virtual bool Pause(bool pause) = 0;
  virtual bool Release() = 0;

  virtual bool SetVolume(float volume) = 0;
  virtual bool SetPitch(float pitch) = 0;

  virtual void Set3DAttributes(const AudioSystemVector& position, const AudioSystemVector& velocity) = 0;
  virtual void Set3DMinMaxDistance(float minDistance, float maxDistance) = 0;
};

// AudioSystemListener
class AudioSystemListener
{
	AudioSystemVector position;
	AudioSystemVector velocity;
	AudioSystemVector forward;
	AudioSystemVector up;

public:

	void Set(const AudioSystemVector& position_, const AudioSystemVector& velocity_, const AudioSystemVector& forward_, const AudioSystemVector& up_)
	{
		position = position_;
		velocity = velocity_;
		forward = forward_;
		up = up_;
	}

	const AudioSystemVector& GetPosition() { return position; }
	const AudioSystemVector& GetVelocity() { return velocity; }
	const AudioSystemVector& GetForward() { return forward; }
	const AudioSystemVector& GetUp() { return up; }
};

// Interface for the AudioSystem
class IAudioSystem
{
public:

	virtual void Initialise() = 0;
	virtual void Deinitialise() = 0;

	virtual void Update() = 0;

  virtual IAudioSystemSound* LoadSound(const std::string& soundName, AudioSystemSoundMode audioSystemSoundMode) = 0;
  virtual bool UnloadSound(IAudioSystemSound* audioSystemSound) = 0;

	void SetListener(const AudioSystemVector& position, const AudioSystemVector& velocity, const AudioSystemVector& forward, const AudioSystemVector& up)
	{
		listener.Set(position, velocity, forward, up);
	}

protected:

	AudioSystemListener listener;
};

#endif // !AUDIO_SYSTEM_IMPL_H

