#include "weapon_system.h"
#include "game.h"

WeaponSystem::WeaponSystem()
{
	for (int i = 0; i < 2; i++){
		guns[i] = NULL;
	}

	sink_rate = 15.0;
	fire_rate = 3.0;
	heat_rate = 0;
	heat = 0;
	shooting_state = false;
	overheated = false;
	shots_fired = 0;
	range = 300.0;

	overheat_alarm_source_idx = -1;
	warning_source_idx = -1;

	guns_ctrl = new XFormNode;

	coolant_charges = 3;
	cooling_amount = 50;
	coolant_recharge_rate = 0.0333;

	deploy_coolant = false;

	dmg = 20.0f;
	overheat_dmg = 40.0f;

	heat_factor = 0.0;
}

WeaponSystem::~WeaponSystem()
{
    for(unsigned int i = 0 ; i < 2 ; i++) {
        if(guns[i])
            delete guns[i];
    }

	if (guns_ctrl)
		delete guns_ctrl;

    if(!projectiles.empty()) {
        std::list<Projectile*>::iterator it = projectiles.begin();

        while(it != projectiles.end()) {
            delete (*it);
            it = projectiles.erase(it);
        }
    }

	if (!weapon_emitters.empty())
		weapon_emitters.clear();
}

/*Private Functions*/
void WeaponSystem::shoot(long time, float dt)
{
	static float projectiles_to_shoot = 0;
	projectiles_to_shoot += fire_rate * dt;
	int num_shots = projectiles_to_shoot;
	projectiles_to_shoot -= num_shots;

	int i = 0;
	while (i < num_shots){
		if ((shots_fired % 2) == 0){
			shoot_gun(GUN_LEFT, time);
			shots_fired++;
		}
		else{
			shoot_gun(GUN_RIGHT, time);
			shots_fired++;
		}
		i++;
	}
}

void WeaponSystem::shoot_gun(GunPosition gun_pos, long time)
{
	Projectile *p = new Projectile;
	p->init();
	Object *front = guns[gun_pos]->get_object("NUC_energy.nogravity$front");
	Object *back = guns[gun_pos]->get_object("NUC_energy.nogravity$back");
    Vector3 front_pos = front->get_matrix().get_translation();
	Vector3 back_pos = back->get_matrix().get_translation();
	Vector3 vel = front_pos - back_pos;
	vel.normalize();
	p->set_velocity(vel);
	p->set_position(front_pos);
	p->set_speed(125.0);
	if (heat >= 70) {
		p->set_dmg(overheat_dmg);
	}
	else {
		p->set_dmg(dmg);
	}
	p->calc_matrix(time);
	NucEmitter *em = p->get_emitter();
	Vector4 start_color = em->get_start_color();
	Vector4 end_color = em->get_end_color();

	float vec_term = (std::max)(0.0, (std::min)(1.0 - heat_factor * 1.51, 1.0));

	Vector3 factor_vec = Vector3(1.0, vec_term, vec_term);
	em->set_start_color(Vector4(start_color.x, start_color.y * factor_vec.y, start_color.z * factor_vec.z, start_color.w));
	em->set_end_color(Vector4(end_color.x, end_color.y  * factor_vec.y, end_color.z  * factor_vec.z, end_color.w));
	em->calc_matrix(time);
	add_projectile(p);


	muzzle_emitters[gun_pos]->set_active(true);
	muzzle_emitters[gun_pos]->set_activation_time(time);

	int anim_idx = guns[gun_pos]->lookup_animation("shoot");
	if (anim_idx == -1)
		return;
	guns[gun_pos]->start_animation(anim_idx, time);
	guns[gun_pos]->set_anim_speed(anim_idx, fire_rate);

	heat += log(heat_rate) * 1.1;
	heat_rate++;

	if (heat >= 100){
		heat = 100;
		heat_rate = 0;
		overheated = true;
		game::engine::audio_manager->play_sample(game::utils::get_audio_sample_by_name("overheat"), 1.0, AUDIO_PLAYMODE_ONCE);
		game::engine::audio_manager->play_sample(game::utils::get_audio_sample_by_name("alarm"), 0.4, AUDIO_PLAYMODE_ONCE, Vector3(), &overheat_alarm_source_idx);
		game::engine::audio_manager->play_sample(game::utils::get_audio_sample_by_name("warning"), 1.5, AUDIO_PLAYMODE_LOOP, Vector3(), &warning_source_idx);
	}

	game::engine::audio_manager->play_sample(game::utils::get_audio_sample_by_name("laser_gun"), 1.0, AUDIO_PLAYMODE_ONCE, front_pos);
}

void WeaponSystem::add_projectile(Projectile *p)
{
	projectiles.push_back(p);
}

void WeaponSystem::update_projectiles(long time, float dt)
{
	std::list<Projectile*>::iterator it = projectiles.begin();

	while (it != projectiles.end()){
		(*it)->update(dt);

		if ((-(*it)->get_position()).length() > range || (*it)->has_collided()){
			(*it)->destroy();
			it = projectiles.erase(it);
		}
		else{
			it++;
		}
	}
}

void WeaponSystem::update_guns(long time, float dt)
{
	static float sinks_to_perform = 0;
	
	if (shooting_state && !overheated){
		shoot(time, dt);
	}
	else{
		sinks_to_perform += sink_rate * dt;
		int num_sinks = sinks_to_perform;
		sinks_to_perform -= num_sinks;

		int i = 0;
		while (i < num_sinks){
			heat--;
			heat_rate--;

			if (heat_rate <= 1)
				heat_rate = 1;

			if (heat <= 0){
				overheated = false;
				heat = 0;

				game::engine::audio_manager->stop_source(overheat_alarm_source_idx);
				game::engine::audio_manager->stop_source(warning_source_idx);
			}
			i++;
		}
	}

	heat_factor = heat / 100.0f;

	set_unistate("st_heat_factor", heat_factor);

	if (deploy_coolant){
		if (coolant_charges){
			use_coolant();
			AudioSample *sample = game::utils::get_audio_sample_by_name("coolant");
			game::engine::audio_manager->play_sample(sample, 0.3, AUDIO_PLAYMODE_ONCE);
		}
		set_coolant_usage(false);
	}

	if (coolant_charges < 3){
		static float coolant_to_recharge = 0;
		coolant_to_recharge += coolant_recharge_rate * dt;
		int num_charges = coolant_to_recharge;
		coolant_to_recharge -= num_charges;

		int i = 0;
		while (i < num_charges){
			coolant_charges++;
			if (coolant_charges > 3)
				coolant_charges = 3;
			i++;
		}
	}
}
/*-------------------------------*/

void WeaponSystem::init()
{
	NucEmitterConfig cfg;
	cfg.emission_duration = 100;
	cfg.start_color = Vector4(1.0, 1.0, 1.0, 1.0);
	cfg.end_color = Vector4(1.0, 1.0, 1.0, 0.3);
	cfg.max_particles = 2000;
	cfg.lifespan = 0.03;
	cfg.spawn_rate = 100;
	cfg.spawn_radius = 0.1;
	cfg.tex_path = "data/textures/particles/star.jpg";
	cfg.size = 3.0;

	for (unsigned int i = 0; i < NUM_GUNS; i++) {
		Object *root = NULL;
		root = guns[i]->get_object(0);

		game::utils::create_scene_psys_emitters(root);

		muzzle_emitters[i] = new NucEmitter;
		muzzle_emitters[i]->set_emitter_config(cfg);

		muzzle_emitters[i]->set_renderer(game::engine::ps_r);
		muzzle_emitters[i]->set_physics_simulator(game::engine::physics_sims[game::engine::physics_sim_idx_by_name["nogravity"]]);
		muzzle_emitters[i]->set_marked_for_death(false);
		muzzle_emitters[i]->init();
		muzzle_emitters[i]->set_active(false);

		guns[i]->get_object("NUC_energy.nogravity$front")->add_child(muzzle_emitters[i]);

		weapon_emitters.push_back(muzzle_emitters[i]);
		weapon_emitters.push_back(dynamic_cast<NucEmitter*>(guns[i]->get_object("NUC_energy.nogravity$front")->get_child(0)));
		weapon_emitters.push_back(dynamic_cast<NucEmitter*>(guns[i]->get_object("NUC_energy.nogravity$back")->get_child(0)));

		game::engine::nuc_manager->add_emitter(muzzle_emitters[i]);
	}

	for (size_t i = 0; i < weapon_emitters.size(); i++) {
		weapon_emitters[i]->set_renderer(game::engine::ps_heat_renderer);
	}
}

void WeaponSystem::set_shooting_state(bool state)
{
	shooting_state = state;
}

void WeaponSystem::set_coolant_charges(unsigned int charges)
{
	coolant_charges = charges;
}

int WeaponSystem::get_heat() const
{
	return heat;
}

XFormNode* WeaponSystem::get_gun_ctrl_node_ptr()
{
	return guns_ctrl;
}

int WeaponSystem::get_coolant_charges() const
{
	return coolant_charges;
}

bool WeaponSystem::is_overheated() const
{
	return overheated;
}

bool WeaponSystem::is_shooting() const
{
	return shooting_state;
}

void WeaponSystem::add_gun(GunPosition gun_pos, Scene *gun)
{
	guns[gun_pos] = gun;
	guns_ctrl->add_child((XFormNode*)guns[gun_pos]->get_object(0));
}

bool WeaponSystem::load_gun(GunPosition gun_pos, const char *path)
{
	guns[gun_pos] = new Scene;
	guns[gun_pos]->load(path);
	if (!guns[gun_pos]){
		std::cout << "Failed to load " << path << ". Aborting!" << std::endl;
		return false;
	}

	guns_ctrl->add_child((XFormNode*)guns[gun_pos]->get_object(0));

	return true;
}

void WeaponSystem::load_gun_anim(GunPosition gun_pos, const char *anim_path, const char *anim_name, bool loop_state)
{
	guns[gun_pos]->load_animation(anim_path, anim_name, loop_state);
}

void WeaponSystem::use_coolant()
{
	heat -= cooling_amount;
	coolant_charges--;

	if (heat <= 0)
		heat = 0;
	if (coolant_charges <= 0)
		coolant_charges = 0;
}

void WeaponSystem::set_coolant_usage(bool state)
{
	deploy_coolant = state;
}

void WeaponSystem::update(long time, float dt)
{
	update_guns(time, dt);
	update_projectiles(time, dt);
}

void WeaponSystem::render_projectiles(unsigned int render_mask, long time)
{
	std::list<Projectile*>::iterator it = projectiles.begin();
	while (it != projectiles.end()){
		(*it)->render(render_mask, time);
		it++;
	}
}

void WeaponSystem::render_guns(unsigned int render_mask, long time)
{
	guns_ctrl->calc_matrix(time);

	for (int i = 0; i < NUM_GUNS; i++){
		if (guns[i]){
			guns[i]->render(render_mask, time);
		}
		else{
			std::cerr << "Weapon System: Guns not assigned!!!" << std::endl;
		}
	}
}
