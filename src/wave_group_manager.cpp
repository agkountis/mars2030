#include "wave_group_manager.h"

WaveGroupManager::WaveGroupManager()
{

}

WaveGroupManager::~WaveGroupManager()
{
    if(!wave_groups.empty()){

        std::list<WaveGroup*>::iterator it = wave_groups.begin();

        while(it != wave_groups.end()) {
            delete (*it);
            it = wave_groups.erase(it);
        }
    }
}

/*Private Functions------------------------------*/
void WaveGroupManager::advance_wave_groups()
{
	std::list<WaveGroup*>::iterator it = wave_groups.begin();

	if ((*it)->is_cleared()){
		delete (*it);
        it = wave_groups.erase(it);
	}
}
/*-----------------------------------------------*/

void WaveGroupManager::add_wave_group(WaveGroup *wave_group)
{
	wave_groups.push_back(wave_group);
}

const std::list<Enemy*>& WaveGroupManager::get_enemy_list() const
{
	return (*wave_groups.begin())->get_enemy_list();
}

void WaveGroupManager::remove_defeated_enemies()
{
	if (!wave_groups.empty()){
		std::list<WaveGroup*>::iterator it = wave_groups.begin();

		while (it != wave_groups.end()){
			(*it)->remove_defeated_enemies();
			it++;
		}
	}
}

void WaveGroupManager::update(float dt, long time)
{
	if (!wave_groups.empty()){
		std::list<WaveGroup*>::iterator it = wave_groups.begin();

		(*it)->update(dt, time);

		advance_wave_groups();
	}
}

void WaveGroupManager::render(unsigned int render_mask, long time)
{
	if (!wave_groups.empty()){
		std::list<WaveGroup*>::iterator it = wave_groups.begin();
		(*it)->render(render_mask, time);
	}
}

bool WaveGroupManager::has_wave_groups() const
{
	return !wave_groups.empty();
}
