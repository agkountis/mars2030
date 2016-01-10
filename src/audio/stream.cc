#include <stdio.h>
#include <assert.h>
#include "openal.h"
#include "stream.h"
#include "logger.h"
#include "timer.h"

static ALenum alformat(AudioStreamBuffer *buf)
{
	return buf->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
}

AudioStream::AudioStream()
{
	alsrc = 0;
	poll_interval = 250;
	done = true;
	loop = false;
	init_volume = 1.0;
	volume = 1.0;

	pthread_mutex_init(&mutex, 0);
}

AudioStream::~AudioStream()
{
	stop();
}

void AudioStream::set_volume(float vol)
{
	init_volume = vol;
}

float AudioStream::get_volume() const
{
	return init_volume;
}

void AudioStream::set_playback_volume(float vol)
{
	volume = vol;
}

float AudioStream::get_playback_volume()
{
	return volume;
}

static void *thread_func(void *arg)
{
	AudioStream *astr = (AudioStream*)arg;
	astr->poll_loop();
	return 0;
}

void AudioStream::play(AUDIO_PLAYMODE mode)
{
	loop = (mode == AUDIO_PLAYMODE_LOOP);
	done = false;

	if(pthread_create(&play_thread, 0, thread_func, this) != 0) {
		error_log("failed to create music playback thread\n");
	}
}

void AudioStream::stop()
{
	pthread_mutex_lock(&mutex);

	if(alsrc) {
		done = true;
		alSourceStop(alsrc);
		printf("waiting for the music thread to stop\n");
		pthread_mutex_unlock(&mutex);
		pthread_join(play_thread, 0);
	} else {
		pthread_mutex_unlock(&mutex);
	}
}

// thread function
void AudioStream::poll_loop()
{
	static const int num_buffers = 3;
	unsigned int albuf[num_buffers];

	pthread_mutex_lock(&mutex);
	alGenSources(1, &alsrc);
	alSourcei(alsrc, AL_LOOPING, AL_FALSE);
    alSourcef(alsrc, AL_GAIN, init_volume);
	alGenBuffers(num_buffers, albuf);
	AudioStreamBuffer *buf = new AudioStreamBuffer;

	for(int i=0; i<num_buffers; i++) {
		if(more_samples(buf)) {
			int bufsz = buf->num_samples * buf->channels * 2;       // 2 is for 16bit samples
			alBufferData(albuf[i], alformat(buf), buf->samples, bufsz, buf->sample_rate);

			if(alGetError()) {
				fprintf(stderr, "failed to load sample data into OpenAL buffer\n");
			}

			alSourceQueueBuffers(alsrc, 1, albuf + i);

			if(alGetError()) {
				fprintf(stderr, "failed to start streaming audio buffers\n");
			}
		} else {
			break;
		}
	}

	// start playback
	alSourcePlay(alsrc);
	while(!done) {
		/* find out how many (if any) of the queued buffers are
		* done, and free to be reused.
		*/
		alSourcef(alsrc, AL_GAIN, volume);
		int num_buf_done;
		alGetSourcei(alsrc, AL_BUFFERS_PROCESSED, &num_buf_done);
		for(int i=0; i<num_buf_done; i++) {
			int err;
			// unqueue a buffer...
			unsigned int buf_id;
			alSourceUnqueueBuffers(alsrc, 1, &buf_id);

			if((err = alGetError())) {
				fprintf(stderr, "failed to unqueue used buffer (error: %x)\n", err);
				num_buf_done = i;
				break;
			}

			int looping;

			alGetSourcei(alsrc, AL_LOOPING, &looping);
			assert(looping == AL_FALSE);
			int cur_buf;
			alGetSourcei(alsrc, AL_BUFFER, &cur_buf);
			if((unsigned int)cur_buf == buf_id) {
				continue;
			}

			// if there are more data, fill it up and requeue it
			if(more_samples(buf)) {
				int bufsz = buf->num_samples * buf->channels * 2;       // 2 is for 16bit samples
				alBufferData(buf_id, alformat(buf), buf->samples, bufsz, buf->sample_rate);
				if((err = alGetError())) {
					fprintf(stderr, "failed to load sample data into OpenAL buffer (error: %x)\n", err);
				}

				alSourceQueueBuffers(alsrc, 1, &buf_id);
				if(alGetError()) {
					fprintf(stderr, "failed to start streaming audio buffers\n");
				}
			} else {
				// no more data...
				if(loop) {
					rewind();
				} else {
					done = true;
				}
			}
		}

		if(num_buf_done) {
			// make sure playback didn't stop
			int state;
			alGetSourcei(alsrc, AL_SOURCE_STATE, &state);
			if(state != AL_PLAYING) {
				alSourcePlay(alsrc);
			}
		}

		pthread_mutex_unlock(&mutex);
		sleep_msec(poll_interval);
		pthread_mutex_lock(&mutex);
	}


	// done with the data, wait for the source to stop playing before cleanup
	int state;

	while(alGetSourcei(alsrc, AL_SOURCE_STATE, &state), state == AL_PLAYING) {
		sched_yield();
	}

	alDeleteBuffers(num_buffers, albuf);
	alDeleteSources(1, &alsrc);
	alsrc = 0;
	pthread_mutex_unlock(&mutex);

	delete buf;
}

bool AudioStream::is_playing() const
{
	return !done;
}

bool AudioStream::is_looping() const
{
	return loop;
}

int AudioStream::freq_count(int bin) const
{
	return 0;	// TODO
}

// frequency range in hertz
int AudioStream::freq_count(int range_start, int range_end) const
{
	return 0;	// TODO
}
