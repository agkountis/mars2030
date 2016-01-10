#ifndef OBJECT_H_
#define OBJECT_H_

#include "xform_node.h"
#include "material.h"
#include "mesh.h"
#include "shader.h"

class Object : public XFormNode 
{
private:
	// the object does NOT OWN THE MESH, DON'T DESTROY IT in the destructor
	Mesh *mesh;	// if null, it's a dummy object
	Material material;
    bool mesh_ownership;

	ShaderProg *specific_sdr;

	bool casts_shad;
	bool recieves_shad;

public:
	Object();
	~Object();

	void set_mesh(Mesh *mesh);
    void set_mesh_ownership(bool ownership);
	void set_specific_shader(ShaderProg *sdr);
	void set_shadow_casting(bool state);
	void set_shadow_recieval(bool state);
	Mesh* get_mesh();
	const Mesh* get_mesh() const;
    bool owns_mesh() const;
	bool casts_shadows() const;
	bool recieves_shadows() const;

	// set material or whatever
	void load_material(const aiMaterial *m);
	void setup_material();
    Material *get_material();

    void render(unsigned int render_mask, long time = 0);
    XFormNode *clone();
};

#endif	// OBJECT_H_
