#include "collision_entity.h"

HitPoint::HitPoint()
{
    pos=Vector3(0,0,0);
    normal=Vector3(0,0,0);
    dist=0;
    col_entity=NULL;
}

HitPoint::HitPoint(const Vector3 &p, const Vector3 &n, float d, const CollisionEntity *c)
{
    pos=p;
    normal=n;
    dist=d;
    col_entity=c;
}

CollisionEntity::~CollisionEntity()
{

}
