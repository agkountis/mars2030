#ifndef ENTITY_H_
#define ENTITY_H_
#include <iostream>
#include "scene.h"
#include "sphere.h"

class Entity:public XFormNode
{
private:
	Scene *sc;
    Sphere *b_obj;
    bool solid;

    static void set_hierarchy_anim_looping(bool state , XFormNode *node);

public:
	Entity();
	Entity(Scene *scn);
    Entity(const Entity &ent);
    Entity &operator = (const Entity &ent);
	~Entity();
    void set_scene(Scene *scn);

	Scene* get_scene();

    void set_solid(bool state);
    bool is_solid() const;
    bool load(const char *fname);
    bool load_animation(const char *fname , const char *anim_name, bool loop_state);
    void render(unsigned int render_mask, long time = 0);
    void set_anim_looping(bool state);
    void start_animation(int idx, long start_time = 0);
    int lookup_animation(const char *name) const;
    bool is_anim_finished(long time);
    void set_anim_speed(float speed);
    void set_anim_speed(unsigned int idx, float speed);


    //TODO will see if this is an ok design.
    void init_b_obj();
    Sphere *get_b_obj() const;

    XFormNode *clone();
};


#endif //ENTITY_H_
