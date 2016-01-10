#ifndef WEAPON_SYSTEM_H_
#define WEAPON_SYSTEM_H_
#include <list>
#include "scene.h"
#include "projectile.h"

class PointSpriteRenderer;

enum GunPosition {
	GUN_LEFT = 0, GUN_RIGHT, NUM_GUNS
};

class WeaponSystem
{
private:
	Scene *guns[2];
	std::list<Projectile*> projectiles;
	XFormNode *guns_ctrl;

	float range;

	int heat;
	int heat_rate;
	float sink_rate;
	float fire_rate;
	float heat_factor;

	int coolant_charges;
	int cooling_amount;
	bool deploy_coolant;
	float coolant_recharge_rate;

	float dmg;
	float overheat_dmg;

	bool overheated;
	bool shooting_state;

	long shots_fired;

	void shoot(long time, float dt);
	void shoot_gun(GunPosition gun_pos, long time);
	void add_projectile(Projectile *p);
	void update_projectiles(long time, float dt);
	void update_guns(long time, float dt);

	int overheat_alarm_source_idx;
	int warning_source_idx;

	NucEmitter *muzzle_emitters[NUM_GUNS];
	std::vector<NucEmitter*> weapon_emitters;

public:
	WeaponSystem();
	~WeaponSystem();

	void init();

	void set_shooting_state(bool state);
	void set_coolant_charges(unsigned int charges);

	int get_heat() const;
	XFormNode *get_gun_ctrl_node_ptr();
	int get_coolant_charges() const;
	bool is_overheated() const;
	bool is_shooting() const;

	void add_gun(GunPosition gun_pos, Scene *gun);
	bool load_gun(GunPosition gun_pos, const char *path);
	void load_gun_anim(GunPosition gun_pos, const char *anim_path, const char *anim_name, bool loop_state);

	void use_coolant();
	void set_coolant_usage(bool state);

	void update(long time, float dt);
	void render_projectiles(unsigned int render_mask, long time = 0);
	void render_guns(unsigned int render_mask, long time = 0);
};

#endif
