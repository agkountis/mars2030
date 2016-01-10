#include "object.h"
#include "unistate.h"
#include "game.h"

Object::Object()
{
	mesh=NULL;
    set_node_type(NODE_OBJ);
    mesh_ownership = false;
	specific_sdr = NULL;
	casts_shad = true;
	recieves_shad = true;
}

Object::~Object()
{
    if(mesh)
        if(mesh_ownership)
            delete mesh;
}

void Object::set_mesh(Mesh *mesh)
{
	this->mesh=mesh;
}

void Object::set_mesh_ownership(bool ownership)
{
    mesh_ownership = ownership;
}

void Object::set_specific_shader(ShaderProg *sdr)
{
	specific_sdr = sdr;
}

void Object::set_shadow_casting(bool state)
{
	casts_shad = state;
}

void Object::set_shadow_recieval(bool state)
{
	recieves_shad = state;
}

Mesh* Object::get_mesh()
{
	return mesh;
}

bool Object::owns_mesh() const
{
    return mesh_ownership;
}

bool Object::casts_shadows() const
{
	return casts_shad;
}

bool Object::recieves_shadows() const
{
	return recieves_shad;
}

const Mesh* Object::get_mesh() const
{
	return mesh;
}

void Object::load_material(const aiMaterial *m)
{
	material.load_material(m);
}

void Object::setup_material()
{
	material.setup_material();
}

Material* Object::get_material()
{
    return &material;
}

void Object::render(unsigned int render_mask, long time)
{
	calc_matrix(time);

    if(render_mask &(1 << get_node_type())){
		if (mesh){
			if (!casts_shad && game::engine::rndr_pass_type == RNDR_PASS_SHADOW){
				//DO NOTHING!
			}
			else{
				set_world_matrix(get_matrix());
				setup_material();

				if (specific_sdr){
					specific_sdr->bind();
				}
				else{
					game::engine::current_sdr->bind();
				}
				mesh->render();
			}
		}
    }

    for(unsigned int i = 0 ; i < get_children_count() ; i++)
	{
        XFormNode *child = get_child(i);
        child->render(render_mask, time);
	}
}

XFormNode* Object::clone()
{
    return new Object(*this);
}

