#include "nuc_physics.h"

NucPhysics::NucPhysics()
{
    velocity_range = 0;
}

NucPhysics::NucPhysics(const Vector3 &init_vel, const Vector3 &ext_force, float vel_range)
{
    init_velocity = init_vel;
    external_force = ext_force;
    velocity_range = vel_range;
}

NucPhysics::~NucPhysics()
{

}

void NucPhysics::set_name(std::string &name)
{
    this->name = name;
}

void NucPhysics::set_init_velocity(const Vector3 &init_vel)
{
    init_velocity = init_vel;
}

void NucPhysics::set_velocity_range(float vel_range)
{
    velocity_range = vel_range;
}

void NucPhysics::set_external_force(const Vector3 &ext_force)
{
    external_force = ext_force;
}

const std::string& NucPhysics::get_name() const
{
    return name;
}

const Vector3& NucPhysics::get_init_velocity() const
{
    return init_velocity;
}

float NucPhysics::get_velocity_range() const
{
    return velocity_range;
}

const Vector3& NucPhysics::get_external_force() const
{
    return external_force;
}

bool NucPhysics::load_config(const std::string &path)
{
    ConfigFile cfg;

    if(!cfg.open(path.c_str()))
    {
        std::cout<<"NucPhysics->>>Could not open the config file!"<<std::endl;
        return false;
    }

    init_velocity = cfg.get_vec("physics_variables.initial_velocity");
    external_force = cfg.get_vec("physics_variables.external_force");
    velocity_range = cfg.get_num("physics_variables.velocity_range", 0);

    return true;
}

void NucPhysics::simulate(float dt_sec, std::list<NucParticle *> &particles, long time_sec)
{
    std::list<NucParticle*>::iterator it = particles.begin();

    for(; it != particles.end() ; it++)
    {
        NucParticle *p = *it;

        p->set_position(p->get_position() + p->get_velocity() * dt_sec);
        p->set_velocity(p->get_velocity() + external_force * dt_sec);

        p->set_life(p->get_life() - dt_sec);
    }
}
