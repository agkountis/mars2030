#ifndef SOURCE_H_
#define SOURCE_H_

#include "vmath/vmath.h"
#include "sample.h"
#include "openal.h"

class AudioSource {
private:
	unsigned int alsrc;
	const AudioSample *sample;

	float volume;

public:
	AudioSource();
	~AudioSource();

	void set_sample(const AudioSample *sample);
	const AudioSample *get_sample() const;

	void set_position(const Vector3 &pos, bool viewspace = false);
	Vector3 get_position() const;

	void set_volume(float vol);
	float get_volume() const;

	void set_playback_volume(float vol);
	float get_playback_volume() const;

	void set_looping(bool state);

	void set_reference_dist(float rdist);
	float get_reference_dist() const;

	void set_max_dist(float dist);
	float get_max_dist() const;

	bool is_playing() const;
	bool is_paused() const;
	void play();
	void stop();
	void pause();
};

#endif	// SOURCE_H_
