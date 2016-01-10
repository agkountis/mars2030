#ifndef STREAM_H_
#define STREAM_H_

#include <pthread.h>

#define AUDIO_BUFFER_MSEC		1000
// TODO should the sampling rate be hardcoded?
#define AUDIO_BUFFER_SAMPLES	(AUDIO_BUFFER_MSEC * 44100 / 1000)
// TODO unhardcode the channels number
#define AUDIO_BUFFER_BYTES		(AUDIO_BUFFER_SAMPLES * 2 * 2)

enum AUDIO_PLAYMODE
{
	AUDIO_PLAYMODE_ONCE,
	AUDIO_PLAYMODE_LOOP
};

struct AudioStreamBuffer {
	char samples[AUDIO_BUFFER_BYTES];

	int num_samples;
	int channels;
	int sample_rate;
};

class AudioStream {
private:
	pthread_t play_thread;
	pthread_mutex_t mutex;

	float init_volume;
	float volume;
	bool done, loop;
	unsigned int poll_interval;
	unsigned int alsrc;

	virtual bool more_samples(AudioStreamBuffer *buf) = 0;

public:
	void poll_loop();

	AudioStream();
	virtual ~AudioStream();

	void set_volume(float vol);
	float get_volume() const;

	void set_playback_volume(float vol);
	float get_playback_volume();

	virtual void play(AUDIO_PLAYMODE mode);
	virtual void stop();

	virtual void rewind() = 0;
	virtual bool is_playing() const;
	virtual bool is_looping() const;

	// TODO
	virtual int freq_count(int bin) const;
	// frequency range in hertz
	virtual int freq_count(int range_start, int range_end) const;
};

#endif  // AUDIO_STREAM_H_
