#include <assert.h>
#include "enemy.h"
#include "game.h"

Enemy::Enemy()
{
    e_templ = NULL;
    death_flag = false;
    hp = 0;
	dmg = 0;
    anim_delay = rand() % 1000 + 1;
}

Enemy::~Enemy()
{
    if(e_templ)
        delete e_templ;
}
void Enemy::set_enemy_template(EnemyTemplate *templ)
{
    e_templ = new EnemyTemplate(*templ);
}

void Enemy::set_velocity(const Vector3 &vel)
{
    velocity = vel;
}

void Enemy::set_position(const Vector3 &pos)
{
    e_templ->set_position(pos);
}

void Enemy::set_rotation(const Vector3 &axis, float angle)
{
    e_templ->set_rotation(axis, angle);
}

void Enemy::set_death_flag(bool state)
{
    death_flag = state;
}

void Enemy::init_hp()
{
    if(e_templ)
        hp = e_templ->get_hp();
}

void Enemy::set_hp(int hp)
{
    this->hp = hp;
}

void Enemy::set_speed(float spd)
{
    speed = spd;
}

void Enemy::set_dmg(int dmg)
{
    this->dmg = dmg;
}


void Enemy::set_anim_looping(bool state)
{
    if(e_templ){
        e_templ->set_anim_looping(state);
    }
    else{
        std::cout<<"Error-->Enemy::set_anim_looping(bool state) : Current enemy object has no template!"<<std::endl;
    }
}

bool Enemy::has_died() const
{
	return death_flag;
}

bool Enemy::is_solid() const
{
	return e_templ->is_solid();
}

void Enemy::set_rotation(const Quaternion &q)
{
    e_templ->set_rotation(q);
}

void Enemy::set_scaling(const Vector3 &scaling)
{
    e_templ->set_scaling(scaling);
}

Vector3 Enemy::get_position() const
{
	if (e_templ)
		return e_templ->get_position();
	return Vector3();
}

EnemyTemplate* Enemy::get_enemy_template()
{
    if(e_templ)
        return e_templ;
    std::cout<<"Enemy::get_enemy_template()-->>>The template pointer is empty"<<std::endl;
    return 0;
}

const Vector3& Enemy::get_velocity()
{
    return velocity;
}

int Enemy::get_hp()
{
    return hp;
}

float Enemy::get_speed()
{
    return speed;
}

int Enemy::get_dmg()
{
    return dmg;
}

Sphere* Enemy::get_bounding_sphere() const
{
    if(e_templ)
        return e_templ->get_b_obj();
    return NULL;
}

void Enemy::render(unsigned int render_mask, long time)
{
    if(e_templ){
        e_templ->render(render_mask, time - anim_delay);
    }
    else{
        std::cout<<"Enemy::render(ShaderProg *srd, long time)->>>Template is empty cant render"<<std::endl;
        return;
    }
}

void Enemy::render_shields()
{
	game::engine::current_sdr = game::engine::drone_shield_sdr;
	e_templ->get_b_obj()->visualization(get_position());
	game::engine::current_sdr = game::engine::main_sdr;
}

void Enemy::update(float dt, long time)
{
	Vector3 pos = get_position();
	Vector3 next_pos = pos + velocity * speed * dt;

	Vector3 front = (next_pos - pos).normalized();
	Vector3 side = cross_product(front, Vector3(0, 1, 0)).normalized();
	Vector3 up = cross_product(side, front).normalized();
	Matrix4x4 lookat;
	lookat.set_row_vector(Vector4(side.x, side.y, side.z, 0.0), 0);
	lookat.set_row_vector(Vector4(up.x, up.y, up.z, 0.0), 1);
	lookat.set_row_vector(Vector4(-front.x, -front.y, -front.z, 0.0), 2);

	//Matrix4x4 lookat;
	//lookat.set_lookat(pos, next_pos);

	//e_templ->set_matrix(lookat.inverse());
	set_rotation(lookat.get_rotation_quat().inverse());
	set_position(next_pos);
	set_velocity(-next_pos.normalized());

	check_state();
}

void Enemy::calc_template_xform(long time)
{
    e_templ->calc_matrix(time);
}

void Enemy::destroy()
{
    delete this;
}

void Enemy::check_state()
{
    if((get_position()).length() < 5.0){
        death_flag = true;
		NucEmitter *em = new NucEmitter;
		NucEmitterConfig conf;
		conf.size = 9.0;
		conf.spawn_radius = 0.2;
		conf.max_particles = 20.0;
		conf.spawn_rate = 300.0;
		conf.tex_path = "data/texture/particles/expl01.png";
		conf.start_color = Vector4(1, 0.3, 0, 1);
		conf.end_color = Vector4(1, 0, 0, 0);
		conf.lifespan = 2;
		conf.emission_duration = 1000;
		
		em->set_emitter_config(conf);
		em->set_marked_for_death(true);
		em->set_renderer(game::engine::ps_r);
		em->set_physics_simulator(game::engine::physics_sims[game::engine::physics_sim_idx_by_name["medium_explosion"]]);
		em->init();
		Vector3 pos = this->get_position();
		em->set_position(pos);
		em->set_activation_time(game::engine::scrn_manager->get_active_screen_msec());
		game::engine::nuc_manager->add_emitter(em);
        game::engine::audio_manager->play_sample(game::utils::get_audio_sample_by_name("explosion"), 6.0, AUDIO_PLAYMODE_ONCE, -pos);

		int stage_hp = game::engine::active_stage->get_health() - dmg;
		if (stage_hp < 0)
			stage_hp = 0;

		game::engine::active_stage->set_health(stage_hp);
	}
	else if (hp <= 0){
		death_flag = true;
		NucEmitter *em = new NucEmitter;
		NucEmitterConfig conf;
		conf.size = 9.0;
		conf.spawn_radius = 0.2;
		conf.max_particles = 20.0;
		conf.spawn_rate = 300.0;
		conf.tex_path = "data/texture/particles/expl01.png";
		conf.start_color = Vector4(1, 0.3, 0, 1);
		conf.end_color = Vector4(1, 0, 0, 0);
		conf.lifespan = 2;
		conf.emission_duration = 1000;

		em->set_emitter_config(conf);
		em->set_marked_for_death(true);
		em->set_renderer(game::engine::ps_r);
		em->set_physics_simulator(game::engine::physics_sims[game::engine::physics_sim_idx_by_name["medium_explosion"]]);
		em->init();
		Vector3 pos = this->get_position();
		em->set_position(pos);
		em->set_activation_time(game::engine::scrn_manager->get_active_screen_msec());
		game::engine::nuc_manager->add_emitter(em);
        game::engine::audio_manager->play_sample(game::utils::get_audio_sample_by_name("explosion"), 6.0, AUDIO_PLAYMODE_ONCE, -pos);
		game::engine::active_stage->kill_count++;
	}
}

void Enemy::init()
{
    if(e_templ){
        hp = e_templ->get_hp();
        speed = e_templ->get_speed();
        dmg = e_templ->get_dmg();
    }
    else{
        std::cout<<"Error Initializing one of the enemies!"<<std::endl;
        std::cout<<"Aborting!"<<std::endl;
        std::cin.get();
        exit(1);
    }
}

bool Enemy::intersect(Ray ray, HitPoint *pt)
{
    Sphere *sph = (Sphere*)e_templ->get_b_obj();
    Sphere world_sph(get_position() + sph->get_pos(), sph->get_radius());
    return world_sph.intersect(ray, pt);
}

void Enemy::repulse(Wave *wave)
{
    Vector3 tmp_vel;

    enable_fpexcept();

	std::list<Enemy*> en_list = wave->get_enemy_list();
	std::list<Enemy*>::iterator it = en_list.begin();

	while (it != en_list.end()){
		if ((*it)->is_solid()){
			if((*it) == this) {
				it++;
				continue;
			}

			Vector3 pos = (*it)->get_position();
			Vector3 this_pos = this->get_position();

			Vector3 repulsion_vec = this_pos - pos;
			float dist = repulsion_vec.length();
			float dist_sqrd = dist*dist;
			if (dist > 0)
			{
				repulsion_vec /= dist;

				float repulsion_factor = (1.0 / dist_sqrd) * 2.0; //parametrize this from the config file
				repulsion_vec *= repulsion_factor;
				tmp_vel += repulsion_vec;
			}
		}
		it++;
	}

    velocity += tmp_vel;
    disable_fpexcept();
}
