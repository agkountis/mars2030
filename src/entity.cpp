#include "entity.h"


Entity::Entity()
{
    sc = NULL;
    b_obj = NULL;
    solid = true;
    set_name("default_dummy");
    set_node_type(NODE_ENT);
}

Entity::Entity(Scene *scn)
{
    sc = scn;
    b_obj = NULL;
    solid = true;
    set_name("default_dummy");
    set_node_type(NODE_ENT);
}

Entity::Entity(const Entity &ent)
{
    sc = new Scene(*ent.sc);

    b_obj = NULL;

    if(ent.b_obj)
        b_obj = new Sphere(*ent.b_obj);

    solid = ent.solid;
    set_node_type(ent.get_node_type());

    for(unsigned int i = 0 ; i < sc->get_object_count() ; i++)
    {
        add_child(sc->get_object(i));
    }
}

Entity& Entity::operator = (const Entity &ent)
{
    if(sc)
        delete sc;

    if(b_obj)
        delete b_obj;

    sc = NULL;
    b_obj = NULL;

    sc = new Scene(*ent.sc);
    b_obj = new Sphere(*ent.b_obj);
    solid = ent.solid; //not sure if needed.
    set_node_type(ent.get_node_type()); //not sure if needed.

    for(unsigned int  i = 0 ; i < sc->get_object_count() ; i++)
    {
        add_child(sc->get_object(i));
    }

    return *this;
}

Entity::~Entity()
{
	if(sc)
		delete sc;
    if(b_obj)
        delete b_obj;
}

void Entity::set_scene(Scene *scn)
{
    sc = scn;
}

Scene* Entity::get_scene()
{
	if(sc)
		return sc;
	std::cout<<"Entity::get_scene()->>> The scene pointer is null!"
		<<std::endl;
	return 0;
}

void Entity::set_solid(bool state)
{
    solid = state;
}

bool Entity::is_solid() const
{
    if(solid)
        return true;
    return false;
}

bool Entity::load(const char *fname)
{
    sc = new Scene;

	if(sc->load(fname))
	{
		Object *obj;
        for(unsigned int i = 0 ; i < sc->get_object_count() ; i++)
		{
            obj = sc->get_object(i);
			add_child(obj);
		}
		return true;
	}
	std::cout<<"Entity::load(const char *fname)->>> Failed to load the scene!"
		<<std::endl;
	return false;
}

bool Entity::load_animation(const char *fname , const char *anim_name, bool loop_state)
{
    if(sc->load_animation(fname , anim_name, loop_state))
        return true;
    return false;
}

void Entity::render(unsigned int render_mask, long time)
{
    calc_matrix(time);
    sc->render(render_mask, time);
}

void Entity::set_anim_looping(bool state)
{
    set_hierarchy_anim_looping(state , this);
}

void Entity::start_animation(int idx, long start_time)
{
    sc->start_animation(idx, start_time);
}

int Entity::lookup_animation(const char *name) const
{
     return sc->lookup_animation(name);
}

bool Entity::is_anim_finished(long time)
{
    return sc->is_anim_finished(time);

    /*XFormNode *child = get_child(0);
    long res = sc->get_max_anim_time();
    long t = (time - child->get_anim_start_time()) * child->get_anim_speed();

    if(t >= res)
        return true;

    return false;*/
}

void Entity::set_anim_speed(float speed)
{
    sc->set_curr_anim_speed(speed);
}

void Entity::set_anim_speed(unsigned int idx, float speed)
{
    sc->set_anim_speed(idx, speed);
}

void Entity::init_b_obj()
{
    Object *obj = NULL;

    if(sc) {
        obj = sc->get_object((unsigned int)0); //get the 1st node in the hierarchy.
        obj->calc_hierarchy_mat();

        b_obj = new Sphere;
        b_obj->calc_pos(obj);
        b_obj->calc_radius(obj);
    }
    else {
        std::cout<<"No scene object has been assigned to this entity"<<std::endl;
    }


}

Sphere* Entity::get_b_obj() const
{
    return b_obj;
}

void Entity::set_hierarchy_anim_looping(bool state, XFormNode *node)
{
    node->XFormNode::set_anim_looping(state);

    for(unsigned int i = 0 ; i < node->get_children_count() ; i++)
    {
        XFormNode *child = node->get_child(i);
        set_hierarchy_anim_looping(state , child);
    }
}

XFormNode* Entity::clone()
{
    return new Entity(*this);
}
