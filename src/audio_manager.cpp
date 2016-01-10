#include "audio_manager.h"
#include "game.h"

StreamPlaylist::StreamPlaylist()
{
	loop = false;
	started = false;
}

SamplePlaylist::SamplePlaylist()
{
	loop = false;
}

AudioManager::AudioManager()
{
	for (unsigned int i = 0; i < NUM_STREAMS; i++){
		streams[i] = NULL;
	}

	for (unsigned int i = 0; i < NUM_SOURCES; i++){
		sources[i] = NULL;
	}

	sources_volume = 1.0;
	streams_volume = 1.0;
	master_volume = 1.0;
}

AudioManager::~AudioManager()
{
	cleanup();
}

/*Private Functions-------------*/
OggVorbisStream* AudioManager::request_stream(int *stream_idx)
{
	for (unsigned int i = 0; i < NUM_STREAMS; i++){
		if (streams[i] && !streams[i]->is_playing()){
			streams[i]->close();
			if (stream_idx)
				*stream_idx = i;
			return streams[i];
		}
	}

	for (unsigned int i = 0; i < NUM_STREAMS; i++){
		if (!streams[i]){
			streams[i] = new OggVorbisStream;
			if (stream_idx)
				*stream_idx = i;
			return streams[i];
		}
	}

	std::cerr << "All available streams are allocated and currenlty playing! Cannot open a new stream!" << std::endl;
	return NULL;
}

AudioSource* AudioManager::request_source(int *source_idx)
{
	for (unsigned int i = 0; i < NUM_SOURCES; i++){
		if (sources[i] && !sources[i]->is_playing()){
			if (source_idx)
				*source_idx = i;
			return sources[i];
		}
	}

	for (unsigned int i = 0; i < NUM_SOURCES; i++){
		if (!sources[i]){
			sources[i] = new AudioSource;
			if (source_idx)
				*source_idx = i;
			return sources[i];
		}
	}

	std::cerr << "Maximum number of sources allocated and are playing a sample! Cannot play the sample specified!" << std::endl;
	return NULL;
}
/*------------------------------*/

void AudioManager::set_listener_position(const Vector3 &pos)
{
	float values[3] = { pos.x, pos.y, pos.z };
	alListenerfv(AL_POSITION, values);
}

void AudioManager::set_listener_orientation(const Vector3 &dir, const Vector3 &up)
{
	float values[6] = { dir.x, dir.y, dir.z, up.x, up.y, up.z };
	alListenerfv(AL_ORIENTATION, values);
}

void AudioManager::set_sources_volume(float volume)
{
	if (volume < 0.0)
		volume = 0.0;

	if (volume > 1.0)
		volume = 1.0;

	sources_volume = volume;
}

void AudioManager::set_streams_volume(float volume)
{
	if (volume < 0.0)
		volume = 0.0;

	if (volume > 1.0)
		volume = 1.0;

	streams_volume = volume;
}

void AudioManager::set_master_volume(float volume)
{
	if (volume < 0.0)
		volume = 0.0;

	if (volume > 1.0)
		volume = 1.0;

	master_volume = volume;
}

OggVorbisStream* AudioManager::get_audio_stream(int idx)
{
	if (idx < NUM_STREAMS && idx >= 0){
		if (streams[idx])
			return streams[idx];
	}
	else{
		std::cerr << "Index out of array bounds! Max available streams ->" << NUM_STREAMS << std::endl;
	}
	return NULL;
}

AudioSource* AudioManager::get_audio_source(int idx)
{
	if (idx < NUM_SOURCES && idx >= 0){
		if (sources[idx])
			return sources[idx];
	}

	return NULL;
}

AudioSource* AudioManager::get_audio_source(AudioSample *sample)
{
	for (unsigned int i = 0; i < NUM_SOURCES; i++){
		const AudioSample *s = NULL;
		if (sources[i])
			s = sources[i]->get_sample();

		if (s == sample){
			return sources[i];
		}
	}

	return NULL;
}

void AudioManager::play_stream(const char *fname, float volume, AUDIO_PLAYMODE mode, int *stream_idx)
{
	OggVorbisStream *stream = request_stream(stream_idx);
	if (stream){
		stream->open(fname);
		stream->set_volume(volume);
		stream->set_playback_volume(volume);
		stream->play(mode);
	}
}

void AudioManager::play_sample(AudioSample *sample, float volume, AUDIO_PLAYMODE mode, const Vector3 &position, int *src_idx)
{
	AudioSource *source = request_source(src_idx);
	if (source){
		source->set_sample(sample);
		source->set_volume(volume);
		source->set_playback_volume(volume);
		source->set_looping((bool)mode);
		source->set_position(position);
		source->play();
	}
}

void AudioManager::play_stream_playlist(StreamPlaylist *playlist, float volume)
{
	OggVorbisStream *stream;

	if (!playlist->started){
		stream = request_stream(&playlist->stream_idx);
		if (stream){
			playlist->it = playlist->files.begin();
			stream->open((*playlist->it).c_str());
			stream->set_volume(volume);
			stream->set_playback_volume(volume);
			stream->play(AUDIO_PLAYMODE_ONCE);
			playlist->it++;
			playlist->started = true;
		}
	}
	else {
		stream = get_audio_stream(playlist->stream_idx);
		if (stream && !stream->is_playing()){
			if (!(playlist->it == playlist->files.end())){
				stream->close();
				stream->open((*playlist->it).c_str());
				stream->set_volume(volume);
				stream->set_playback_volume(volume);
				stream->play(AUDIO_PLAYMODE_ONCE);
				playlist->it++;
			}
		}
	}

	if (playlist->loop && playlist->it == playlist->files.end())
		playlist->it = playlist->files.begin();
}

void AudioManager::play_sample_playlist(SamplePlaylist *playlist, float volume)
{
	AudioSource *src;

	if (!playlist->started){
		src = request_source(&playlist->source_idx);
		if (src){
			playlist->it = playlist->samples.begin();
			src->set_sample((*playlist->it));
			src->set_volume(volume);
			src->set_playback_volume(volume);
			src->play();
			playlist->it++;
			playlist->started = true;
		}
	}
	else{
		src = get_audio_source(playlist->source_idx);
		if (src && !src->is_playing()){
			if (!(playlist->it == playlist->samples.end())){
				src->set_sample((*playlist->it));
				src->set_volume(volume);
				src->set_playback_volume(volume);
				src->play();
				playlist->it++;
			}
		}
	}

	if (playlist->loop && playlist->it == playlist->samples.end())
		playlist->it = playlist->samples.begin();
}

void AudioManager::stop_stream(int stream_idx)
{
	if (stream_idx < 0)
		return;

	AudioStream *strm = get_audio_stream(stream_idx);
	if (strm && strm->is_playing())
		strm->stop();

}

void AudioManager::stop_source(int source_idx)
{
	if (source_idx < 0)
		return;

	AudioSource *src = get_audio_source(source_idx);
	if (src && src->is_playing())
		src->stop();
}

void AudioManager::stop_streams()
{
	for (unsigned int i = 0; i < NUM_STREAMS; i++){
		if (streams[i] && streams[i]->is_playing())
			streams[i]->stop();
	}
}

void AudioManager::stop_sources()
{
	for (unsigned int i = 0; i < NUM_SOURCES; i++){
		if (sources[i] && sources[i]->is_playing())
			sources[i]->stop();
	}
}

void AudioManager::pause_streams()
{

}

void AudioManager::pause_sources()
{
	for (unsigned int i = 0; i < NUM_SOURCES; i++){
		if (sources[i] && sources[i]->is_playing())
			sources[i]->pause();
	}
}

void AudioManager::resume_streams()
{
	//TODO
}

void AudioManager::resume_sources()
{
	for (unsigned int i = 0; i < NUM_SOURCES; i++){
		if (sources[i] && !sources[i]->is_playing())
			sources[i]->play();
	}
}


void AudioManager::regulate_volumes()
{
    for (unsigned int i = 0; i < NUM_SOURCES; i++){
		if (sources[i]) {
			float vol = sources[i]->get_volume() * sources_volume * master_volume;
			sources[i]->set_playback_volume(vol);
		}
	}

	for (unsigned int i = 0; i < NUM_STREAMS; i++){
		if (streams[i]) {
			streams[i]->set_playback_volume(streams[i]->get_volume() * streams_volume * master_volume);
		}
	}
}

void AudioManager::cleanup()
{
	for (unsigned int i = 0; i < NUM_STREAMS; i++){
		if (streams[i]){
			streams[i]->stop();
			streams[i]->close();
			delete streams[i];
		}
	}

	for (unsigned int i = 0; i < NUM_SOURCES; i++){
		if (sources[i]){
			sources[i]->stop();
			delete sources[i];
		}
	}
}
