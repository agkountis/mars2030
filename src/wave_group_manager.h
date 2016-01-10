#ifndef WAVE_GROUP_MANAGER_H_
#define WAVE_GROUP_MANAGER_H_
#include "wave_group.h"

class WaveGroupManager
{
private:
	std::list<WaveGroup*> wave_groups;

	void advance_wave_groups();

public:
	WaveGroupManager();
	~WaveGroupManager();

	void add_wave_group(WaveGroup *wave_group);

	const std::list<Enemy*> &get_enemy_list() const;

	void remove_defeated_enemies();
	void update(float dt, long time);
	void render(unsigned int render_mask, long time = 0);
	bool has_wave_groups() const;
};

#endif
