#include <stdio.h>
#include <assert.h>
#include "datapath.h"
#include "logger.h"
#include "ovstream.h"

OggVorbisStream::OggVorbisStream()
{
	vfopen = false;

	pthread_mutex_init(&vflock, 0);
}

OggVorbisStream::~OggVorbisStream()
{
	close();
}

bool OggVorbisStream::open(const char *fname)
{
	close();

	pthread_mutex_lock(&vflock);

	std::string fpath = datafile_path(fname);
	info_log("opening ogg/vorbis stream: %s -> %s\n", (fname && *fname) ? fname : "<not found>", (!fpath.empty()) ? fpath.c_str() : "<not found>");

	if(fpath.empty() || ov_fopen(fpath.c_str(), &vf) != 0) {
		error_log("failed to open ogg/vorbis stream: %s\n", fname ? fname : "<not found>");
		pthread_mutex_unlock(&vflock);
		return false;
	}

	vfopen = true;
	pthread_mutex_unlock(&vflock);
	return true;
}

void OggVorbisStream::close()
{
	pthread_mutex_lock(&vflock);
	if(vfopen) {
		ov_clear(&vf);
		vfopen = false;
	}
	pthread_mutex_unlock(&vflock);
}

void OggVorbisStream::play(AUDIO_PLAYMODE mode)
{
	if (vfopen)
	{
		AudioStream::play(mode);
	} else {
		error_log("failed to play audio track.\n");
	}
}

void OggVorbisStream::rewind()
{
	pthread_mutex_lock(&vflock);
	if(vfopen) {
		ov_raw_seek(&vf, 0);
	}
	pthread_mutex_unlock(&vflock);
}

bool OggVorbisStream::more_samples(AudioStreamBuffer *buf)
{
	pthread_mutex_lock(&vflock);

	vorbis_info *vinfo = ov_info(&vf, -1);
	buf->channels = vinfo->channels;
	buf->sample_rate = vinfo->rate;
	assert(buf->channels == 2);
	assert(buf->sample_rate == 44100);

	long bufsz = AUDIO_BUFFER_BYTES;
	long total_read = 0;
	while(total_read < bufsz) {
		int bitstream;
		long rd = ov_read(&vf, buf->samples + total_read, bufsz - total_read, 0, 2, 1, &bitstream);

		if(!rd) {
			bufsz = total_read;
		} else {
			total_read += rd;
		}
	}

	if(!total_read) {
		buf->num_samples = 0;
		pthread_mutex_unlock(&vflock);
		return false;
	}

	buf->num_samples = bufsz / vinfo->channels / 2;
	pthread_mutex_unlock(&vflock);
	return true;
}
