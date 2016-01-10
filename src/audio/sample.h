#ifndef SAMPLE_H_
#define SAMPLE_H_

#include "dataset.h"

class AudioSample {
private:
	unsigned int albuffer;

	void destroy();

public:
	AudioSample();
	~AudioSample();

	bool load(const char *fname);

	friend class AudioSource;
};

typedef DataSet<AudioSample*> SampleSet;

AudioSample *load_audio_sample(const char *fname);
void destroy_audio_sample(AudioSample *s);

#endif	// SAMPLE_H_
