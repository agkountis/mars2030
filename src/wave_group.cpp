#include "wave_group.h"
#include "game.h"

WaveGroup::WaveGroup()
{
	sample = NULL;
	playback_started = false;
}

WaveGroup::~WaveGroup()
{
    if(!waves.empty()) {
        std::list<Wave*>::iterator it = waves.begin();

        while(it != waves.end()) {
            delete (*it);
            it = waves.erase(it);
        }
    }
}


/*Private Functions--------------------------------*/
void WaveGroup::advance_waves(long time)
{
	std::list<Wave*>::iterator it = waves.begin();

	if ((*it)->is_cleared() && (*it)->has_spawn_started()){
		delete (*it);
		it = waves.erase(it);

		if (!waves.empty())
			(*it)->set_activation_time(time);
	}
}
/*-------------------------------------------------*/

void WaveGroup::add_wave(Wave *wave)
{
	waves.push_back(wave);
}

void WaveGroup::assign_audio_sample(AudioSample *sample)
{
	this->sample = sample;
}

const std::list<Enemy*>& WaveGroup::get_enemy_list() const
{
	return (*waves.begin())->get_enemy_list();
}

void WaveGroup::assign_audio_sample(const char *name)
{
	unsigned int idx = game::assets::audio_sample_idx_by_name[name];
	sample = game::assets::audio_samples[idx];
}

void WaveGroup::remove_defeated_enemies()
{
	if (!waves.empty()){
		std::list<Wave*>::iterator it = waves.begin();

		while (it != waves.end()){
			(*it)->remove_defeated_enemies();
			it++;
		}
	}
}

void WaveGroup::update(float dt, long time)
{
    AudioSource *src = NULL;
    if(sample && !playback_started) {
		source.set_sample(sample);
		source.set_looping(false);
		source.set_volume(1.0);
		source.play();
        //game::engine::audio_manager->play_sample(sample, 1.0, AUDIO_PLAYMODE_ONCE);
        playback_started = true;
        game::engine::audio_manager->set_streams_volume(0.15);
    }

	/*if (sample){
		src = game::engine::audio_manager->get_audio_source(sample);
	}*/

    if(source.is_playing()) {
        return;
    }

	game::engine::audio_manager->set_streams_volume(1.0);

    if (!waves.empty()) {
        std::list<Wave*>::iterator it = waves.begin();
        (*it)->update(dt, time);

        advance_waves(time);
    }
}

void WaveGroup::render(unsigned int render_mask, long time)
{
	if (!waves.empty()){
		std::list<Wave*>::iterator it = waves.begin();

		(*it)->render(render_mask, time);
	}
}

bool WaveGroup::is_cleared() const
{
	return waves.empty();
}
