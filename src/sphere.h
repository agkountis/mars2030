#ifndef SPHERE_H
#define SPHERE_H
#include "collision_entity.h"

class Sphere:public CollisionEntity
{
private:
    Vector3 pos;
    float radius;

	Mesh mesh;

public:
    Sphere();
    Sphere(const Vector3 &p, float r);

    void set_pos(const Vector3 &pos);
    void set_radius(float rad);
    const Vector3 &get_pos() const;
    float get_radius() const;
    bool intersect(const Ray &ray, HitPoint *pt) const;

    void calc_pos(Object *root);
    void calc_radius(Object *root); //must always be called after calc_pos.

    void visualization(const Vector3 &obj_pos);
};

#endif // SPHERE_H
