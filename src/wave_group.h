#ifndef WAVE_GROUP_H_
#define WAVE_GROUP_H_
#include "wave.h"
#include "sample.h"
#include "source.h"

class WaveGroup
{
private:
	std::list<Wave*> waves;
	AudioSample *sample;
	AudioSource source;
    bool playback_started;

	void advance_waves(long time);

public:
	WaveGroup();
	~WaveGroup();

	void add_wave(Wave *wave);
	void assign_audio_sample(AudioSample *sample);
	void assign_audio_sample(const char *name);

	const std::list<Enemy*> &get_enemy_list() const;

	void remove_defeated_enemies();
	void update(float dt, long time);
	void render(unsigned int render_mask, long time = 0);

	bool is_cleared() const;
};


#endif
