#ifndef ENEMY_H
#define ENEMY_H
#include "enemy_template.h"



class Wave;

class Enemy
{
private:
    EnemyTemplate *e_templ;
    Vector3 velocity;
    int hp;
    float speed;
    int dmg;
    bool death_flag;
    long anim_delay;

public:
    Enemy();
    virtual ~Enemy();

    void set_enemy_template(EnemyTemplate *templ);
    void set_velocity(const Vector3 &vel);
    void set_position(const Vector3 &pos);
    void set_rotation(const Vector3 &axis , float angle);
    void set_rotation(const Quaternion &q);
    void set_scaling(const Vector3 &scaling);
    void set_death_flag(bool state);
    void init_hp(); //get template's hp
    void set_hp(int hp); //manually assign hp
    void set_speed(float spd);
    void set_dmg(int dmg);
    void set_anim_looping(bool state);

    bool has_died() const;
	bool is_solid() const;

    Vector3 get_position() const;
    EnemyTemplate* get_enemy_template();
    const Vector3 &get_velocity();
    int get_hp();
    float get_speed();
    int get_dmg();
    Sphere *get_bounding_sphere() const;

    virtual void render(unsigned int render_mask, long time = 0);
	void render_shields();
    virtual void update(float dt, long time = 0);
    void calc_template_xform(long time = 0);
    void destroy();
    void check_state();
    void init();
    bool intersect(Ray ray,HitPoint *pt);

    virtual void repulse(Wave *wave);
};

#endif // ENEMY_H
