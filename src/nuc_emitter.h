#ifndef NUC_EMITTER_H
#define NUC_EMITTER_H
#include <list>
#include "nuc_physics.h"
#include "nuc_particle.h"
#include "texture.h"
#include "nuc_renderer.h"
#include "xform_node.h"
#include "cfg.h"

struct NucEmitterConfig
{
    float spawn_rate;
    float lifespan; /*Particle lifespan*/
    int max_particles;
    float spawn_radius;
    std::string tex_path;

    /*The colors the particle will take during its lifetime*/
    Vector4 start_color;
    Vector4 end_color;

    float size; /*The size of the particles*/

    /*The duration that the emitter will be emitting
     *If the value is 0 the emitter will emit non stop.*/
    long emission_duration;

    NucEmitterConfig();
};

class NucEmitter:public XFormNode
{
protected:
    NucEmitterConfig config;

    //Not owned by the emitter do not destroy in the destructor.----------------
    NucRenderer *renderer; /*The renderer of the Nucleus Particle System*/
    NucPhysics *physics; /*The Physics Simulator*/
    //------------------------------------------------------------------------

    goatgfx::Texture *tex; /*The texture to be used for the particles*/

    float particles_to_spawn;

    std::list<NucParticle*> particles; /*the particles in the Nucleus particles system */
    std::list<NucParticle*>::iterator particle_it;

    bool active; /*Determines if hte emitters produces particles or not*/
    bool marked_for_death; /*Determines if the nuc_manager will kill it or not*/
	long activation_time; /*The 1st activation timestamp.*/

public:
    NucEmitter();
    virtual ~NucEmitter();

    /*Setter functions*/
    void set_spawn_rate(float spawn_rate);
    void set_lifespan(float lifespan);
    void set_max_particles(int max_particles);
    void set_particles_to_spawn(float particles_to_spawn);
    void set_spawn_radius(float spawn_radius);
    void set_start_color(const Vector4 &col);
    void set_end_color(const Vector4 &col);
    void set_emitter_config(const NucEmitterConfig &config);
    void set_renderer(NucRenderer *renderer);
    void set_physics_simulator(NucPhysics *sim);
    void set_texture(goatgfx::Texture *tex);
    void set_active(bool state);
    void set_marked_for_death(bool state);
	void set_activation_time(long time);

    /*Getter functions*/
    float get_spawn_rate() const;
    float get_lifespan() const;
    const Vector3 &get_init_velocity() const;
    int get_max_particles() const;
    float get_particles_to_spawn() const;
    float get_spawn_radius() const;
    const Vector4 &get_start_color() const;
    const Vector4 &get_end_color() const;
    const NucEmitterConfig &get_emitter_config() const;
    const NucRenderer *get_renderer() const;
    const NucPhysics *get_physics_simulator() const;
    goatgfx::Texture *get_texture() const;
    unsigned int get_particle_count() const;
    bool is_active() const;
    bool is_marked_for_death() const;
	long get_activation_time() const;

    /*Overload this function if u want to change the way u load configuration
     *files for the emitter*/
    virtual bool load_config(const std::string &path);

    /*Overload this function if u want to change the way u load the
     *emitter texture.*/
    virtual bool load_texture(std::string path);

    /*Overload this function if u want to change what happens when
     *the emitter updates.*/
    virtual void update(float dt_sec, long time = 0);

    /*Overload this function if u want to change the way particles
     *will spawn.*/
    virtual void spawn_particle(long time = 0);

    /*Overload this function if u want to change the initialization*/
    virtual bool init();

    void render(unsigned int render_mask, long time = 0);
	void clear_particles();
};

#endif // NUC_EMITTER_H
