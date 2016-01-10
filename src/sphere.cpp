#include "sphere.h"
#include "unistate.h"
#include "game.h"

static void add_all_mesh_verts(Object* root,Sphere *sph, int *total_verts);

Sphere::Sphere()
{
    pos = Vector3(0, 0, 0);
    radius = 1.0;

	game::utils::gen_uv_sphere(&mesh, 1.0, 16, 16, 1, 1);
}

Sphere::Sphere(const Vector3 &p, float r)
{
    pos = p;
    radius = r;
}

void Sphere::set_pos(const Vector3 &pos)
{
    this->pos = pos;
}

void Sphere::set_radius(float rad)
{
    radius = rad;
}

const Vector3& Sphere::get_pos() const
{
    return pos;
}

float Sphere::get_radius() const
{
    return radius;
}

bool Sphere::intersect(const Ray &ray, HitPoint *pt) const
{
    float a = dot_product(ray.dir , ray.dir);

    float b = 2.0 * ray.dir.x * (ray.origin.x - pos.x) +
            2.0 * ray.dir.y * (ray.origin.y - pos.y) +
            2.0 * ray.dir.z * (ray.origin.z - pos.z);

    float c = dot_product(ray.origin , ray.origin) + dot_product(pos , pos) -
            2.0 * dot_product(ray.origin , pos) - radius * radius;

    float discr = b * b - 4.0 * a * c;

    if(discr < 1e-4)
        return false;

    float sqrt_discr = sqrt(discr);
    float t0 = (-b + sqrt_discr) / (2.0 * a);
    float t1 = (-b - sqrt_discr) / (2.0 * a);

    if(t0 < 1e-4)
        t0 = t1;
    if(t1 < 1e-4)
        t1 = t0;

    float t = t0 < t1 ? t0 : t1;
    if(t < 1e-4)
        return false;

    pt->col_entity = this;
    pt->dist = t;
    pt->pos = ray.origin + ray.dir * t;
    pt->normal = (pt->pos - pos) / radius;

    return true;
}

void Sphere::visualization(const Vector3 &obj_pos)
{
    Object sph;
    sph.set_position(pos + obj_pos);
	sph.set_scaling(Vector3(radius, radius, radius));
	sph.set_mesh(&mesh);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	sph.set_shadow_casting(false);
    sph.render(RENDER_OBJ);
    glDisable(GL_BLEND);
}

void Sphere::calc_pos(Object *root)
{
    int total_verts = 0;
    add_all_mesh_verts(root,this,&total_verts);
    pos /= (float)total_verts;
}

void Sphere::calc_radius(Object *root)
{
    Mesh *m = NULL;
    Vertex *v = NULL;

    m = root->get_mesh();

    if(m){
        v = m->get_vertex_data();

        for(unsigned int i = 0 ; i < m->get_vertex_count() ; i++){
            Vector3 tmp = v->pos;
            tmp.transform(root->get_matrix());
            float dist = (tmp - pos).length();

            if(dist > radius){
                radius = dist;
            }
            v++;
        }
    }

    for(unsigned int i = 0 ; i < root->get_children_count() ; i++){
        Object *child = (Object*)root->get_child(i);
        calc_radius(child);
    }

}

static void add_all_mesh_verts(Object* root, Sphere *sph, int *total_verts)
{
    Mesh *m = NULL;
    m = root->get_mesh();

    if(m)
    {
        Vertex *v=NULL;
        v = m->get_vertex_data();

        for(unsigned int i = 0 ; i < m->get_vertex_count() ; i++)
        {
            Vector3 tmp = v->pos;
            tmp.transform(root->get_matrix());

            Vector3 pos = sph->get_pos();
            pos += tmp;
            sph->set_pos(pos);
			v++;
        }

        *total_verts += m->get_vertex_count();
    }

    for(unsigned int i=0; i<root->get_children_count();i++)
    {
        Object *child = (Object*)root->get_child(i);
        add_all_mesh_verts(child,sph,total_verts);
    }
}
