#include "projectile.h"
#include "game.h"

static bool instersect_sphere_capsule(const Vector3 &sph_center, float sph_rad, const Vector3 &start, const Vector3 &end, float caps_rad)
{
	float axis_length_squared = (end - start).length_sq();

	float distance = 0.0;

	float rad_sum = sph_rad + caps_rad;

	/*if start == end*/
	if (axis_length_squared == 0)
		return false;
		
		//distance = (sph_center - start).length();

	/* Consider the line extending the segment, parameterized as v + t (w - v).
	 * We find projection of point p onto the line. 
	 * It falls where t = [(p-v) . (w-v)] / |w-v|^2 */
	float t = dot_product(sph_center - start, end - start) / axis_length_squared;

	/*Point is beyond the start of the capsule*/
	if (t < 0.0) {
		distance = (sph_center - start).length();

		if (distance <= rad_sum)
			return true;

		return false;
	}

	/*Point is after the end of the capsule*/
	if (t > 1.0) {
		distance = (sph_center - end).length();

		if (distance <= rad_sum)
			return true;

		return false;
	}

	/*If none of the above was satisfied the point is between the capsule's start and end points
	so we calulate it's projection on the capsul's axis*/

	Vector3 projection = start + t * (end - start);

	distance = (sph_center - projection).length();

	if (distance <= rad_sum)
		return true;

	return false;
}

Projectile::Projectile()
{
	collided = false;
	dmg = 0;
}

Projectile::~Projectile()
{

}

void Projectile::set_velocity(const Vector3 &velocity)
{
	this->velocity = velocity;
}

void Projectile::set_speed(float speed)
{
	this->speed = speed;
}

void Projectile::init()
{
	proj_em = new NucEmitter;

	unsigned int conf_idx = game::engine::emitter_config_idx_by_name["energy"];
	proj_em->set_emitter_config(game::engine::emitter_configs[conf_idx]);

	unsigned int phys_idx = game::engine::physics_sim_idx_by_name["nogravity"];
	proj_em->set_physics_simulator(game::engine::physics_sims[phys_idx]);

	proj_em->set_renderer(game::engine::ps_r);

	proj_em->init();

	add_child(proj_em);

	game::engine::nuc_manager->add_emitter(proj_em);
}

void Projectile::update(float dt)
{
	Vector3 pos = get_position();
	Vector3 next_pos = pos + velocity * speed * dt;

	WaveGroupManager *m = game::engine::active_stage->get_wave_group_manager_ptr();

	if (m->has_wave_groups()){
		std::list<Enemy*> enemies = m->get_enemy_list();
		std::list<Enemy*>::iterator it = enemies.begin();

		while (it != enemies.end()){
			Vector3 en_pos = (*it)->get_position();
			float proj_rad = this->proj_em->get_emitter_config().spawn_radius;
			float en_rad = (*it)->get_bounding_sphere()->get_radius();

			if (instersect_sphere_capsule(en_pos, en_rad, pos, next_pos, proj_rad)){
				(*it)->set_hp((*it)->get_hp() - dmg);
				collided = true;

				NucEmitter *em = new NucEmitter;
				NucEmitterConfig conf;
				conf.size = 3.0;
				conf.spawn_radius = 0.2;
				conf.max_particles = 20.0;
				conf.spawn_rate = 300.0;
				conf.tex_path = "data/texture/particles/star.jpg";
				conf.start_color = Vector4(1, 1, 1, 1);
				conf.end_color = Vector4(1, 1, 1, 0.1);
				conf.lifespan = 1.5;
				conf.emission_duration = 1000;

				em->set_emitter_config(conf);
				em->set_marked_for_death(true);
				em->set_renderer(game::engine::ps_r);
				em->set_physics_simulator(game::engine::physics_sims[game::engine::physics_sim_idx_by_name["nogravity"]]);
				em->init();
				Vector3 pos = get_position();
				em->set_position(pos);
				em->set_activation_time(game::engine::scrn_manager->get_active_screen_msec());
				game::engine::nuc_manager->add_emitter(em);

				break;
			}
			it++;
		}
	}

	set_position(next_pos);
}

void Projectile::destroy()
{
	proj_em->set_active(false);
	delete this;
}

NucEmitter* Projectile::get_emitter()
{
	return proj_em;
}

void Projectile::set_dmg(float dmg)
{
	this->dmg = dmg;
}

float Projectile::get_dmg() const
{
	return dmg;
}

bool Projectile::has_collided() const
{
	return collided;
}

