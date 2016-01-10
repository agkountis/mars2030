#include "nuc_particle.h"

ParticleAttributes::ParticleAttributes()
{
    color = Vector3(1, 1, 1);
    size = 0;
}

NucParticle::NucParticle()
{
    life = 0;
    active = false;
}

NucParticle::NucParticle(const Vector3 &vel, const Vector3 &pos, const Vector4 &col,
                         float life, bool active, float size)
{
    velocity = vel;
    p_attribs.position = pos;
    p_attribs.color = col;
    this->life = life;
    this->active = active;
    p_attribs.size = size;
}

NucParticle::~NucParticle()
{

}

void NucParticle::set_velocity(const Vector3 &vel)
{
    velocity = vel;
}

void NucParticle::set_position(const Vector3 &pos)
{
    p_attribs.position = pos;
}

void NucParticle::set_color(const Vector4 &col)
{
    p_attribs.color = col;
}

void NucParticle::set_life(float life)
{
    this->life = life;
}

void NucParticle::set_active(bool state)
{
    active = state;
}

void NucParticle::set_size(float size)
{
    p_attribs.size = size;
}

void NucParticle::set_particle_attributes(const ParticleAttributes &attrs)
{
    p_attribs = attrs;
}

void NucParticle::set_spawn_time(double time_sec)
{
    spawn_time = time_sec;
}

const Vector3& NucParticle::get_velocity() const
{
    return velocity;
}

const Vector3& NucParticle::get_position() const
{
    return p_attribs.position;
}

const Vector4& NucParticle::get_color() const
{
    return p_attribs.color;
}

float NucParticle::get_life() const
{
    return life;
}

bool NucParticle::is_active() const
{
    return active;
}

bool NucParticle::is_alive() const
{
    if(life > 0)
        return true;

    return false;
}

float NucParticle::get_size() const
{
    return p_attribs.size;
}

const ParticleAttributes& NucParticle::get_particle_attributes() const
{
    return p_attribs;
}

double NucParticle::get_spawn_time() const
{
    return spawn_time;
}
