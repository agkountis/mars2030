#ifndef AUDIO_MANAGER_H_
#define AUDIO_MANAGER_H_
#include "ovstream.h"
#include "source.h"
#include <list>

#define NUM_STREAMS 4
#define NUM_SOURCES 128

struct StreamPlaylist
{
	std::list<std::string> files;
	std::list<std::string>::iterator it;
	bool loop;
	bool started;
	int stream_idx;

	StreamPlaylist();
};

struct SamplePlaylist
{
	std::list<AudioSample*> samples;
	std::list<AudioSample*>::iterator it;
	bool loop;
	bool started;
	int source_idx;

	SamplePlaylist();
};

class AudioManager
{
private:
	OggVorbisStream *streams[NUM_STREAMS];
	AudioSource *sources[NUM_SOURCES];

	OggVorbisStream *request_stream(int *stream_idx = NULL);
	AudioSource *request_source(int *source_idx = NULL);

	float sources_volume;
	float streams_volume;
	float master_volume;

public:
	AudioManager();
	~AudioManager();

	void set_listener_position(const Vector3 &pos = Vector3(0, 0, 0));
	void set_listener_orientation(const Vector3 &dir, const Vector3 &up = Vector3(0, 1, 0));

	void set_sources_volume(float volume);
	void set_streams_volume(float volume);
	void set_master_volume(float volume);

	OggVorbisStream *get_audio_stream(int idx);
	AudioSource *get_audio_source(int idx);
	AudioSource *get_audio_source(AudioSample *sample);

	void play_stream(const char *fname, float volume, AUDIO_PLAYMODE mode, int *stream_idx = NULL);
	void play_sample(AudioSample *sample, float volume, AUDIO_PLAYMODE mode, const Vector3 &position = Vector3(0, 0, 0), int *src_idx = NULL);

	void play_stream_playlist(StreamPlaylist *playlist, float volume);
	void play_sample_playlist(SamplePlaylist *playlist, float volume);

	void stop_stream(int stream_idx);
	void stop_source(int source_idx);

	void stop_streams();
	void stop_sources();

	void pause_streams(); //TODO
	void pause_sources();

	void resume_streams(); //TODO
	void resume_sources(); 

	void regulate_volumes();

	void cleanup();
};

#endif //AUDIO_MANAGER_H_
