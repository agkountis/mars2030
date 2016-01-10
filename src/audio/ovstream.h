#ifndef OVSTREAM_H_
#define OVSTREAM_H_

#include <pthread.h>
#include <vorbis/vorbisfile.h>
#include "stream.h"

class OggVorbisStream : public AudioStream {
private:
	OggVorbis_File vf;
	bool vfopen;
	pthread_mutex_t vflock;

	virtual bool more_samples(AudioStreamBuffer *buf);

public:
	OggVorbisStream();
	virtual ~OggVorbisStream();

	bool open(const char *fname);
	void close();

	virtual void play(AUDIO_PLAYMODE mode);
	virtual void rewind();
};

#endif  // OVSTREAM_H_
