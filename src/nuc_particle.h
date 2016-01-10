#ifndef NUC_PARTICLE_H
#define NUC_PARTICLE_H
#include "vmath.h"

struct ParticleAttributes
{
    Vector3 position;
    Vector4 color;
    float size;

    ParticleAttributes();
};

class NucParticle
{
private:
	Vector3 velocity;
    ParticleAttributes p_attribs;

    float life;

	bool active;
    double spawn_time;


public:
     float init_life; //testing
    NucParticle();
    NucParticle(const Vector3 &vel, const Vector3 &pos, const Vector4 &col, float life, bool active, float size);
    ~NucParticle();

	void set_velocity(const Vector3 &vel);
    void set_position(const Vector3 &pos);
    void set_color(const Vector4 &col);
    void set_life(float life);
    void set_active(bool state);
    void set_size(float size);
    void set_particle_attributes(const ParticleAttributes &attrs);
    void set_spawn_time(double time_sec);

	const Vector3 &get_velocity() const;
    const Vector3 &get_position() const;
    const Vector4 &get_color() const;
    float get_life() const;
    bool is_active() const;
    bool is_alive() const;
    float get_size() const;
    const ParticleAttributes& get_particle_attributes() const;
    double get_spawn_time() const;
};

#endif //NUC_PARTICLE_H
