#ifndef PROJECTILE_H_
#define PROJECTILE_H_
#include "object.h"
#include "nuc_emitter.h"

class Projectile : public Object
{
private:
	NucEmitter *proj_em;
	Mesh *m;
	Vector3 velocity;
	float speed;
	float dmg;

	bool collided;

public:
	Projectile();
	~Projectile();

	void set_velocity(const Vector3 &velocity);
	void set_speed(float speed);
	void init();
	void update(float dt);
	void destroy();
	NucEmitter *get_emitter();
	void set_dmg(float dmg);
	float get_dmg() const;

	bool has_collided() const;
};

#endif
