#ifndef WAVE_H
#define WAVE_H
#include "enemy.h"
#include "enemy_generator.h"
#include <list>


class Wave
{
private:
    std::list<Enemy*> enemies;
    std::list<EnemyGenerator*> generators;

    bool spawn_started;
    long init_delay;
    long activation_time;

public:
    Wave();
    ~Wave();

    void add_enemy(Enemy *enemy);
    void add_generator(EnemyGenerator *gen);
    void remove_defeated_enemies();
    void set_init_delay(long delay);
    void set_total_duration(long time);
    void set_activation_time(long time);

	const std::list<Enemy*> &get_enemy_list() const;
    unsigned int get_enemy_count() const;
    long get_init_delay() const;
    long get_total_duration() const;
    long get_activation_time() const;

    bool is_cleared() const;
    bool has_spawn_started() const;

    void render(unsigned int render_mask, long time = 0);
    void update(float dt, long time);
    void spawn(float dt, long time);
    void clear();
};

#endif // WAVE_H
