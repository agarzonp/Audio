#pragma once

typedef size_t AudioSystemSoundMode;

#define AudioSystemSoundMode_DEFAULT 0x00000000
#define AudioSystemSoundMode_2D      0x00000001
#define AudioSystemSoundMode_3D      0x00000002

struct AudioSystemVector
{
	float x;
	float y;
	float z;
};