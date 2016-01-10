#include "openal.h"
#include "source.h"

AudioSource::AudioSource()
{
	sample = 0;

	alGenSources(1, &alsrc);
	alSourcei(alsrc, AL_LOOPING, AL_FALSE);

    volume = 1.0;
}

AudioSource::~AudioSource()
{
	if(alsrc) {
		if(is_playing()) {
			stop();
		}
		alDeleteSources(1, &alsrc);
	}
}

void AudioSource::set_sample(const AudioSample *sample)
{
	stop();

	if(sample) {
		if(!sample->albuffer) {
			fprintf(stderr, "%s: trying to attach null buffer!\n", __FUNCTION__);
			return;
		}
		alSourcei(alsrc, AL_BUFFER, sample->albuffer);
	}
	this->sample = sample;
}

const AudioSample *AudioSource::get_sample() const
{
	return sample;
}

void AudioSource::set_position(const Vector3 &pos, bool viewspace)
{
	alSourcei(alsrc, AL_SOURCE_RELATIVE, viewspace ? AL_TRUE : AL_FALSE);
	alSource3f(alsrc, AL_POSITION, pos.x, pos.y, pos.z);
}

Vector3 AudioSource::get_position() const
{
	float pos[3];
	alGetSourcefv(alsrc, AL_POSITION, pos);
	return Vector3(pos[0], pos[1], pos[2]);
}

void AudioSource::set_volume(float vol)
{
	volume = vol;
}

float AudioSource::get_volume() const
{
	return volume;
}

void AudioSource::set_playback_volume(float vol)
{
	alSourcef(alsrc, AL_GAIN, vol);
}

float AudioSource::get_playback_volume() const
{
	float vol;
	alGetSourcef(alsrc, AL_GAIN, &vol);
	return vol;
}

void AudioSource::set_looping(bool state)
{
	alSourcei(alsrc, AL_LOOPING, state);
}

void AudioSource::set_reference_dist(float rdist)
{
	alSourcef(alsrc, AL_REFERENCE_DISTANCE, rdist);
}

float AudioSource::get_reference_dist() const
{
	float res;
	alGetSourcef(alsrc, AL_REFERENCE_DISTANCE, &res);
	return res;
}

void AudioSource::set_max_dist(float dist)
{
	alSourcef(alsrc, AL_MAX_DISTANCE, dist);
}

float AudioSource::get_max_dist() const
{
	float res;
	alGetSourcef(alsrc, AL_MAX_DISTANCE, &res);
	return res;
}

bool AudioSource::is_playing() const
{
	int state;
	alGetSourcei(alsrc, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

bool AudioSource::is_paused() const
{
	int state;
	alGetSourcei(alsrc, AL_SOURCE_STATE, &state);
	return state == AL_PAUSED;
}

void AudioSource::play()
{
	if(sample) {
		alSourcePlay(alsrc);
	}
}

void AudioSource::stop()
{
	if(sample) {
		alSourceStop(alsrc);
	}
}

void AudioSource::pause()
{
	if (sample) {
		alSourcePause(alsrc);
	}
}
