#ifndef SCENE_H_
#define SCENE_H_

#include <string>
#include "mesh.h"
#include "object.h"
#include "light.h"
#include "camera.h"
#include "shader.h"
#include "float.h"


class Scene
{
private:
	std::vector<Mesh*> meshes;
	std::vector<Object*> objects;
	std::vector<Light*> lights;
	std::vector<Camera*> cameras;

    bool owns_meshes;

    long max_anim_time;

	Object *lookup_object(Object *obj, const std::string &name);

public:
	Camera* active_camera;
	Scene();
    Scene(const Scene &sc);
    Scene &operator = (const Scene &sc);
	~Scene();

	bool load(const char *fname);
    bool load_animation(const char *fname, const char *anim_name, bool loop_state);

	void add_mesh(Mesh *mesh);
	void add_object(Object *obj);
	void add_light(Light *lt);
	void add_camera(Camera *cam);

	// get by index
	Mesh* get_mesh(unsigned int idx);
	Object* get_object(unsigned int idx);
	Light* get_light(unsigned int idx);
	Camera* get_camera(unsigned int idx);

	unsigned int get_mesh_count() const;
	unsigned int get_object_count() const;
    unsigned int get_light_count() const;
    unsigned int get_camera_count() const;

	// get by name
    Mesh* get_mesh(std::string name);
    Object* get_object(std::string name);
    Light* get_light(std::string name); //TODO.....
    Camera* get_camera(std::string name);

	void setup_lights();
	void setup_cameras(long time = 0);
	void setup_scene(long time = 0);
	void set_active_camera();
    void render(unsigned int render_mask, long time = 0);
    void start_animation(unsigned int idx, long start_time);
    int lookup_animation(const char *name);
    void set_curr_anim_speed(float speed);
    void set_anim_speed(unsigned int idx, float speed);

    /*Gets the maximum animation duration in a Scene level
     * and not just for a single object hierarchy*/
    long get_max_anim_time();
    long get_max_anim_time(unsigned int idx);

	bool has_meshes();
	bool has_objects();
	bool has_cameras();
	bool has_lights();

    bool is_anim_finished(long time);
};

#endif	// SCENE_H_
