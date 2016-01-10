#ifndef NUCPHYSICS_H
#define NUCPHYSICS_H
#include <list>
#include <iostream>
#include "vmath.h"
#include "nuc_particle.h"
#include "cfg.h"

class NucPhysics
{
protected:
    std::string name;
    Vector3 init_velocity;
    float velocity_range;
    Vector3 external_force;

public:
    NucPhysics();
    NucPhysics(const Vector3 &init_vel, const Vector3 &ext_force, float vel_range);
    virtual ~NucPhysics();

    void set_name(std::string &name);
    void set_init_velocity(const Vector3 &init_vel);
    void set_velocity_range(float vel_range);
    void set_external_force(const Vector3 &ext_force);

    const std::string &get_name() const;
    const Vector3 &get_init_velocity() const;
    float get_velocity_range() const;
    const Vector3 &get_external_force() const;

    virtual bool load_config(const std::string &path); //TODO

    virtual void simulate(float dt_sec, std::list<NucParticle*> &particles, long time_sec = 0);
};

#endif // NUCPHYSICS_H
