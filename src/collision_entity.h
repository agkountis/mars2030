#ifndef COLLISION_ENTITY_H
#define COLLISION_ENTITY_H
#include "object.h"
#include "vmath.h"

class CollisionEntity;

struct HitPoint
{
    Vector3 pos; //world position of the intersection point.
    Vector3 normal; //surface normal vector at the intersection point
    float dist; //parametric distance of the intersection along the ray
    const CollisionEntity *col_entity;

    HitPoint();
    HitPoint(const Vector3 &p,const Vector3 &n,float d,const CollisionEntity *c);
};

class CollisionEntity
{
public:
    virtual ~CollisionEntity();
    virtual bool intersect(const Ray &ray,HitPoint *pt) const = 0;
    virtual void visualization(const Vector3 &obj_pos) = 0;
};

#endif // COLLISION_ENTITY_H
