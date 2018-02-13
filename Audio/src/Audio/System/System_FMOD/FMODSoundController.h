#ifndef FMOD_SOUND_CONTROLLER_H
#define FMOD_SOUND_CONTROLLER_H

class FMODSoundController : public AudioSystemSoundController
{
public:

	FMODSoundController(void* controller) : AudioSystemSoundController(controller) {}

	bool Stop() const override
	{
		FMOD::Channel* channel = static_cast<FMOD::Channel*> (controller);
		if (!channel)
		{
			return false;
		}

		FMOD_RESULT result = channel->stop();
		if (result != FMOD_OK)
			printf("Failed to stop sound. Error: %s \n", FMOD_ErrorString(result));

		return (result == FMOD_OK);
	}

	bool Pause(bool pause) const 
	{ 
		FMOD::Channel* channel = static_cast<FMOD::Channel*> (controller);
		if (!channel)
		{
			return false;
		}

		FMOD_RESULT result = channel->setPaused(pause);
		if (result != FMOD_OK)
			printf("Failed to %s sound. Error: %s \n", pause ? "pause" : "resume", FMOD_ErrorString(result));

		return (result == FMOD_OK);
	}

	bool SetVolume(float volume) const 
	{ 
		FMOD::Channel* channel = static_cast<FMOD::Channel*> (controller);
		if (!channel)
		{
			return false;
		}

		FMOD_RESULT result = channel->setVolume(volume);
		if (result != FMOD_OK)
			printf("Failed to set volume for sound. Error: %s \n", FMOD_ErrorString(result));

		return (result == FMOD_OK);
	}

	bool SetPitch(float pitch) const 
	{ 
		FMOD::Channel* channel = static_cast<FMOD::Channel*> (controller);
		if (!channel)
		{
			return false;
		}

		FMOD_RESULT result = channel->setPitch(pitch);
		if (result != FMOD_OK)
			printf("Failed to set pitch for sound. Error: %s \n", FMOD_ErrorString(result));

		return (result == FMOD_OK);
	}

	void Set3DAttributes(const AudioSystemVector& position, const AudioSystemVector& velocity) override
	{
		AudioSystemSoundController::Set3DAttributes(position, velocity);

		FMOD::Channel* channel = static_cast<FMOD::Channel*> (controller);
		if (channel)
		{
			const FMOD_VECTOR* pos = reinterpret_cast<const FMOD_VECTOR*> (&this->position);
			const FMOD_VECTOR* vel = reinterpret_cast<const FMOD_VECTOR*> (&this->velocity);
			channel->set3DAttributes(pos, vel);
		}
	}

	void Set3DMinMaxDistance(float minDistance, float maxDistance) override
	{
		FMOD::Channel* channel = static_cast<FMOD::Channel*> (controller);
		if (channel)
		{
			channel->set3DMinMaxDistance(minDistance, maxDistance);
		}
	}
};
#endif // !FMOD_SOUND_CONTROLLER_H


