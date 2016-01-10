#include <stdio.h>

#include "openal.h"
#include "audio.h"

static ALCdevice *dev;
static ALCcontext *ctx;

bool init_audio()
{
	if (dev) {
		// Already initiated.
		return true;
	}
	else if (!(dev = alcOpenDevice(0))) {
		fprintf(stderr, "failed to open OpenAL device\n");
		return false;
	}

	if (ctx)
	{
		return true;
	}
	else if (!(ctx = alcCreateContext(dev, 0)))
	{
		fprintf(stderr, "failed to create context\n");
		alcCloseDevice(dev);
		return false;
	}

	alcMakeContextCurrent(ctx);

	// Clear error state.
	alGetError();

	return true;
}

void destroy_audio()
{
	alcMakeContextCurrent(0);

	if (ctx) {
		alcDestroyContext(ctx);
	}

	if (dev) {
		alcCloseDevice(dev);
	}
}

void set_audio_listener(const Matrix4x4 &xform)
{
	float pos[3], orient[6];

	pos[0] = xform[0][3];
	pos[1] = xform[1][3];
	pos[2] = xform[2][3];

	orient[0] = xform[0][2];
	orient[1] = xform[1][2];
	orient[2] = -xform[2][2];

	orient[3] = xform[0][1];
	orient[4] = xform[1][1];
	orient[5] = xform[2][1];

	alListenerfv(AL_POSITION, pos);
	alListenerfv(AL_ORIENTATION, orient);
}
