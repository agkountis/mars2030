#include "wave.h"
#include "game.h"


Wave::Wave()
{
    spawn_started = false;
    init_delay = 0;
    activation_time = 0;
}

Wave::~Wave()
{
    clear();

    if(!generators.empty())
    {
        std::list<EnemyGenerator*>::iterator it = generators.begin();

		while (it != generators.end()){
			delete (*it);
			it = generators.erase(it);
		}
    }
}

void Wave::add_enemy(Enemy *enemy)
{
    enemies.push_back(enemy);
}

void Wave::add_generator(EnemyGenerator *gen)
{
    generators.push_back(gen);
}

void Wave::remove_defeated_enemies()
{
    std::list<Enemy*>::iterator it = enemies.begin();

    for(unsigned int i = 0 ; i < enemies.size() ; i++){
        Enemy *en = *it;

        if(en->has_died()){
            delete en;
            it = enemies.erase(it);
        }
        else{
            it++;
        }
    }

}

void Wave::set_init_delay(long delay)
{
    init_delay = delay;
}

void Wave::set_activation_time(long time)
{
    activation_time = time;
}

const std::list<Enemy*>& Wave::get_enemy_list() const
{
	return enemies;
}

unsigned int Wave::get_enemy_count() const
{
    return enemies.size();
}

long Wave::get_init_delay() const
{
    return init_delay;
}

long Wave::get_activation_time() const
{
    return activation_time;
}

bool Wave::is_cleared() const
{
    return enemies.empty();
}

bool Wave::has_spawn_started() const
{
    return spawn_started;
}

void Wave::render(unsigned int render_mask, long time)
{
    if(!enemies.empty()){
        std::list<Enemy*>::iterator it = enemies.begin();

		while (it != enemies.end()) {
			(*it)->render(render_mask, time);
			it++;
		}

		it = enemies.begin();

		while (it != enemies.end()) {
			(*it)->render_shields();
			it++;
		}
    }
}

void Wave::update(float dt, long time)
{
    spawn(dt, time);

    if(!is_cleared()){
        std::list<Enemy*>::iterator it = enemies.begin();

		while (it != enemies.end()){
			(*it)->update(dt, time);
			(*it)->repulse(this);
			it++;
		}
    }
}

void Wave::spawn(float dt, long time)
{
    if(time - activation_time > init_delay){
		std::list<EnemyGenerator*>::iterator it = generators.begin();

		while (it != generators.end()){
			(*it)->generate(SPHERICAL_GEN, dt, time);
			if (enemies.size() > 0)
				spawn_started = true;
			if ((*it)->get_amount() <= 0){
				delete (*it);
				it = generators.erase(it);
			}
			else{
				it++;
			}
		}
    }
}

void Wave::clear()
{
    if(!is_cleared())
    {
        std::list<Enemy*>::iterator it = enemies.begin();

		while (it != enemies.end()){
			delete (*it);
			it = enemies.erase(it);
		}
    }
}
