#include "holomap.h"
#include "game.h"

Holonode::Holonode()
{
	Mesh *m = new Mesh;
	game::utils::gen_uv_sphere(m, 0.03, 6, 6, 1.0, 1.0);
	node.set_mesh(m);
	node.set_mesh_ownership(true);
	node.set_specific_shader(game::engine::holomap_sdr);

	Material *mat = node.get_material();
	mat->diffuse = Vector4(1, 0, 0, 1);

	this->en = NULL;
}

Holomap::Holomap()
{
	max_detect_dist = 0.0;
	max_gun_range = 0.0;
	holomap_scale = 0.19;
	base.set_name("holomap_base");
	range_dome.set_name("range_dome");
	attachment_dummy = NULL;
}

Holomap::Holomap(float detect_dist, float gun_range)
{
	max_detect_dist = detect_dist;
	max_gun_range = gun_range;
	holomap_scale = 0.19;
	base.set_name("holomap_base");
	range_dome.set_name("range_dome");
	attachment_dummy = NULL;
}

Holomap::~Holomap()
{
	if (!nodes.empty()){
		std::list<Holonode*>::iterator it = nodes.begin();

		while (it != nodes.end()){
			remove_child((XFormNode*)&(*it)->node);
			delete (*it);
			it = nodes.erase(it);
		}
	}
}

/*Private Functions--------------------------------------------*/
void Holomap::clamp_to_unit(float *x, float *y, float *z)
{
	if (*x > 1.0){
		*x = 1.0;
	}
	else if (*x < -1.0){
		*x = -1.0;
	}

	if (*y > 1.0){
		*y = 1.0;
	}
	else if (*y < -1.0){
		*y = -1.0;
	}

	if (*z > 1.0){
		*z = 1.0;
	}
	else if (*z < -1.0){
		*z = -1.0;
	}
}
/*-------------------------------------------------------------*/

void Holomap::init()
{
	Mesh *m = new Mesh;
	Vertex v;
	Face f;

	v.pos = Vector3(1, 0, 1);
	v.u = 1;
	v.v = 0;
	m->add_vertex(v);

	v.pos = Vector3(1, 0, -1);
	v.u = 1;
	v.v = 1;
	m->add_vertex(v);

	v.pos = Vector3(-1, 0, 1);
	v.u = 0;
	v.v = 0;
	m->add_vertex(v);

	v.pos = Vector3(1, 0, -1);
	v.u = 1;
	v.v = 1;
	m->add_vertex(v);

	v.pos = Vector3(-1, 0, -1);
	v.u = 0;
	v.v = 1;
	m->add_vertex(v);

	v.pos = Vector3(-1, 0, 1);
	v.u = 0;
	v.v = 0;
	m->add_vertex(v);

	int counter = 0;

	for (unsigned int i = 0; i < 2; i++){
		for (unsigned int j = 0; j < 3; j++){
			f.vidx[j] = counter++;
		}
		m->add_index(f);
	}

	base.set_mesh(m);
	base.set_mesh_ownership(true);
	base.set_specific_shader(game::engine::holomap_4dnoise_sdr);
	base.set_scaling(Vector3(holomap_scale, holomap_scale, holomap_scale));
	Material *mat = base.get_material();
	mat->alpha = 1.0;
	mat->textures[TEX_DIFFUSE] = game::engine::texset->get("/data/textures/spaceships/grid5.png");

	m = new Mesh;
	float norm_range_dist = max_gun_range / max_detect_dist;

	if (norm_range_dist > 1.0)
		norm_range_dist = 1.0;
	
	game::utils::gen_uv_sphere(m, norm_range_dist , 20.0, 20.0, 1.0, 0.71);
	range_dome.set_mesh(m);
	range_dome.set_mesh_ownership(true);
	range_dome.set_scaling(Vector3(holomap_scale, holomap_scale, holomap_scale));
	range_dome.set_specific_shader(game::engine::holomap_4dnoise_sdr);
	mat = range_dome.get_material();
	mat->alpha = 1.0;
	mat->textures[TEX_DIFFUSE] = game::engine::texset->get("/data/textures/spaceships/grid5.png");

	m = new Mesh;
	game::utils::gen_uv_sphere(m, 0.06, 6.0, 6.0, 1.0, 1.0);
	center.set_mesh(m);
	center.set_mesh_ownership(true);
	center.set_scaling(Vector3(holomap_scale, holomap_scale, holomap_scale));
	center.set_specific_shader(game::engine::holomap_sdr);
	mat = center.get_material();
	mat->alpha = 1.0;
	mat->diffuse = Vector4(0, 1, 0, 1.0);

	add_child((XFormNode*)&base);
	add_child((XFormNode*)&range_dome);
	add_child((XFormNode*)&center);
}

void Holomap::set_max_detect_dist(float detect_dist)
{
	max_detect_dist = detect_dist;
}

void Holomap::set_max_gun_range(float gun_range)
{
	max_gun_range = gun_range;
}

void Holomap::set_attachment_dummy(XFormNode *att_dummy)
{
	attachment_dummy = att_dummy;
}

void Holomap::add_node(Holonode *node)
{
	if (node){
		add_child((XFormNode*)&node->node);
		nodes.push_back(node);
	}
}

void Holomap::update()
{

	if (!nodes.empty()){
		std::list<Holonode*>::iterator it = nodes.begin();

		while (it != nodes.end()){
			if ((*it)->en->has_died()){
				remove_child((XFormNode*)&(*it)->node);
				delete (*it);
				it = nodes.erase(it);
			}
			else{
				Vector3 en_pos = (*it)->en->get_position();
				Vector3 map_pos = en_pos / max_detect_dist;
				clamp_to_unit(&map_pos.x, &map_pos.y, &map_pos.z);
				Matrix4x4 map_scale;
				map_scale.scale(Vector4(holomap_scale, holomap_scale, holomap_scale, holomap_scale));
				map_pos.transform(map_scale);
				(*it)->node.set_position(map_pos);
				(*it)->node.set_scaling(Vector3(holomap_scale, holomap_scale, holomap_scale));
				it++;
			}
		}
	}
}

void Holomap::render(unsigned int render_mask, long time)
{
	if (attachment_dummy){
		Vector3 pos = attachment_dummy->get_matrix().get_translation();
		Quaternion rot = attachment_dummy->get_rotation();
		set_position(pos);
		set_rotation(rot);
	}

	calc_matrix(time);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glDisable(GL_CULL_FACE);
	base.render(render_mask, time);
	range_dome.render(render_mask, time);
	glEnable(GL_CULL_FACE);

	center.render(render_mask, time);

	if (!nodes.empty()){
		std::list<Holonode*>::iterator it = nodes.begin();

		while (it != nodes.end()){
			(*it)->node.render(render_mask, time);
			it++;
		}
	}

	glDisable(GL_BLEND);
}