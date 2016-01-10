#include "nuc_emitter.h"
#include "game.h"

NucEmitterConfig::NucEmitterConfig()
{
    /*i placed values for testing*/
    spawn_rate = 0;
    lifespan = 0;
    max_particles = 0;
    spawn_radius = 0;

    start_color = Vector4(1, 1, 1, 1);
    end_color = Vector4(1, 1, 1, 1);

    size = 1.0;

    emission_duration = 0;
}

NucEmitter::NucEmitter()
{
    renderer = NULL;
    particles_to_spawn = 0;
    tex = NULL;
    active = false;
    marked_for_death = true;
	activation_time = 0;

    set_node_type(NODE_PSYS);
}

NucEmitter::~NucEmitter()
{
    if(!particles.empty()) {
        this->clear_particles();
    }
}

void NucEmitter::set_spawn_rate(float spawn_rate)
{
    config.spawn_rate = spawn_rate;
}

void NucEmitter::set_lifespan(float lifespan)
{
    config.lifespan = lifespan;
}

void NucEmitter::set_max_particles(int max_particles)
{
    config.max_particles = max_particles;
}

void NucEmitter::set_particles_to_spawn(float particles_to_spawn)
{
    this->particles_to_spawn = particles_to_spawn;
}

void NucEmitter::set_spawn_radius(float spawn_radius)
{
    config.spawn_radius = spawn_radius;
}

void NucEmitter::set_start_color(const Vector4 &col)
{
    config.start_color = col;
}

void NucEmitter::set_end_color(const Vector4 &col)
{
    config.end_color = col;
}

void NucEmitter::set_emitter_config(const NucEmitterConfig &config)
{
    this->config = config;
}

void NucEmitter::set_renderer(NucRenderer *renderer)
{
    this->renderer = renderer;
}

void NucEmitter::set_physics_simulator(NucPhysics *sim)
{
    physics = sim;
}

void NucEmitter::set_texture(goatgfx::Texture *tex)
{
    this->tex = tex;
}

void NucEmitter::set_active(bool state)
{
    active = state;
}

void NucEmitter::set_marked_for_death(bool state)
{
    marked_for_death = state;
}

void NucEmitter::set_activation_time(long time)
{
	activation_time = time;
}

float NucEmitter::get_spawn_rate() const
{
    return config.spawn_rate;
}

float NucEmitter::get_lifespan() const
{
    return config.lifespan;
}

int NucEmitter::get_max_particles() const
{
    return config.max_particles;
}

float NucEmitter::get_particles_to_spawn() const
{
    return particles_to_spawn;
}

float NucEmitter::get_spawn_radius() const
{
    return config.spawn_radius;
}

const Vector4& NucEmitter::get_start_color() const
{
    return config.start_color;
}

const Vector4& NucEmitter::get_end_color() const
{
    return config.end_color;
}

const NucEmitterConfig& NucEmitter::get_emitter_config() const
{
    return config;
}

const NucRenderer* NucEmitter::get_renderer() const
{
    return renderer;
}

const NucPhysics* NucEmitter::get_physics_simulator() const
{
    return physics;
}

goatgfx::Texture* NucEmitter::get_texture() const
{
    return tex;
}

unsigned int NucEmitter::get_particle_count() const
{
    return particles.size();
}

bool NucEmitter::is_active() const
{
    return active;
}

bool NucEmitter::is_marked_for_death() const
{
    return marked_for_death;
}

long NucEmitter::get_activation_time() const
{
	return activation_time;
}

bool NucEmitter::load_config(const std::string &path)
{
    NCF ncf;

    ncf.set_source(path.c_str());

    if(ncf.parse() != 0) {
        std::cout<<"Could not parse the file '"<<path<<"'."<<std::endl;
        return false;
    }

    ConfigFile cfg;

    cfg.set_ncf(&ncf);

    config.spawn_rate = cfg.get_num("emitter_variables.spawn_rate", 1.0);
    config.lifespan = cfg.get_num("emitter_variables.lifespan",1.0);
    config.max_particles = cfg.get_int("emitter_variables.max_particles", 1);
    config.spawn_radius = cfg.get_num("emitter_variables.spawn_radius", 1.0);
    config.tex_path = cfg.get_str("emitter_variables.tex_path");
    config.start_color = cfg.get_vec("emitter_variables.start_color", Vector4(1.0, 1.0, 1.0, 1.0));
    config.end_color = cfg.get_vec("emitter_variables.end_color", Vector4(1.0, 1.0, 1.0, 1.0));
    config.size = cfg.get_num("emitter_variables.size", 1.0);
    config.emission_duration = cfg.get_int("emitter_variables.emission_duration");

    return true;
}

bool NucEmitter::load_texture(std::string path)
{
    tex = game::engine::texset->get(path.c_str());

    if(!tex) {
        std::cout<<"NucEmmiter->>> Texture loading failed!"<<std::endl;
        return false;
    }

    return true;
}

void NucEmitter::update(float dt_sec, long time)
{
    calc_matrix(time);

    if(config.emission_duration){
        if(time - activation_time > config.emission_duration)
            active = false;
    }

    if(!particles.empty()){
        particle_it = particles.begin();

        /*Check if active particles are dead and delete them*/
        while(particle_it != particles.end()){
            NucParticle *p = *particle_it;
            if(p->is_active()){
                if(!p->is_alive()){
                    delete p;
                    particle_it = particles.erase(particle_it);
                    continue;
                }

				double time_sec = time / 1000.0;
				double t = (double)(time_sec - p->get_spawn_time()) / p->get_life();


				float x1, x2, y1, y2, z1, z2, w1, w2;

				x1 = config.start_color.x;
				x2 = config.end_color.x;

				y1 = config.start_color.y;
				y2 = config.end_color.y;

				z1 = config.start_color.z;
				z2 = config.end_color.z;

				w1 = config.start_color.w;
				w2 = config.end_color.w;

				float min_x = (std::min)(x1, x2);
				float max_x = (std::max)(x1, x2);

				float min_y = (std::min)(y1, y2);
				float max_y = (std::max)(y1, y2);

				float min_z = (std::min)(z1, z2);
				float max_z = (std::max)(z1, z2);

				float min_w = (std::min)(w1, w2);
				float max_w = (std::max)(w1, w2);

				Vector4 col;

				col.x = x1 + (x2 - x1) * t;
				col.y = y1 + (y2 - y1) * t;
				col.z = z1 + (z2 - z1) * t;
				col.w = w1 + (w2 - w1) * t;

				col.x = (std::max)(min_x, (std::min)(col.x, max_x));
				col.y = (std::max)(min_y, (std::min)(col.y, max_y));
				col.z = (std::max)(min_z, (std::min)(col.z, max_z));
				col.w = (std::max)(min_w, (std::min)(col.w, max_w));

				p->set_color(col);
			}

			particle_it++;
        }

        /*Physics simulation follows*/
        physics->simulate(dt_sec, particles);
    }

    /*Spawn new particles!*/
    particles_to_spawn += config.spawn_rate * dt_sec; //calculate the amount of particles to spawn.
    int num_spawn = particles_to_spawn;
    particles_to_spawn -= num_spawn;

    int i = 0;

    if(active){
        while(i < num_spawn && (int)particles.size() < config.max_particles){
            spawn_particle(time);
            i++;
        }
    }
}

void NucEmitter::spawn_particle(long time)
{
    NucParticle *p = new NucParticle;

    Vector3 init_vel = physics->get_init_velocity();
    float vel_range = physics->get_velocity_range();

    float rand_pos_x = ((float)rand() / (float)RAND_MAX) * config.spawn_radius * 2
            - config.spawn_radius;

    float rand_pos_y = ((float)rand() / (float)RAND_MAX) * config.spawn_radius * 2
            - config.spawn_radius;

    float rand_pos_z = ((float)rand() / (float)RAND_MAX) * config.spawn_radius * 2
            - config.spawn_radius;

    float rand_vel_x = ((float)rand() / (float)RAND_MAX) * vel_range * 2 - vel_range;

    float rand_vel_y = ((float)rand() / (float)RAND_MAX) * vel_range * 2 - vel_range;

    float rand_vel_z = ((float)rand() / (float)RAND_MAX) * vel_range * 2 - vel_range;

    Vector3 pos = get_matrix().get_translation();

    p->set_position(pos + Vector3(rand_pos_x, rand_pos_y, rand_pos_z));
    p->set_velocity(init_vel + Vector3(rand_vel_x, rand_vel_y, rand_vel_z));
    p->set_life(config.lifespan);
    p->set_color(config.start_color);
    p->set_size(config.size);
    p->set_spawn_time(time / 1000.0);

    p->set_active(true);
    p->init_life = config.lifespan; //testing

    particles.push_back(p);
}

bool NucEmitter::init()
{
    if(config.tex_path.empty()){
        std::cout<<"NucEmitter->>>Texture Path string is empty!!"<<std::endl;
        return false;
    }

    if(!load_texture(config.tex_path)){
        std::cout<<"NucEmitter->>>Failed to load the texture!!"<<std::endl;
        return false;
    }

    active = true;

    return true;
}

void NucEmitter::render(unsigned int render_mask, long time)
{
    if(render_mask &(1 << get_node_type())){
        renderer->pre_render_ops();
        renderer->render(particles, tex, config.size);
        renderer->post_render_ops();
    }

    for(unsigned int i = 0 ; i < get_children_count() ; i++){
        XFormNode *child = get_child(i);
        child->render(render_mask, time);
    }
}

void NucEmitter::clear_particles()
{
	particle_it = particles.begin();
	
	while (particle_it != particles.end()){
		delete (*particle_it);
		particle_it = particles.erase(particle_it);
	}
}
