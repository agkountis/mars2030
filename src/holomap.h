#ifndef HOLOMAP_H_
#define HOLOMAP_H_
#include "enemy.h"
#include <list>

struct Holonode
{
	Object node;
	Enemy *en;

	Holonode();
};

class Holomap:public XFormNode
{
private:
	std::list<Holonode*> nodes;
	Object base;
	Object range_dome;
	Object center;
	XFormNode *attachment_dummy;

	float max_detect_dist;
	float max_gun_range;
	float holomap_scale;

	void clamp_to_unit(float *x, float *y, float *z);

public:
	Holomap();
	Holomap(float detect_dist, float gun_range);
	~Holomap();

	void init();

	void set_max_detect_dist(float max_dist);
	void set_max_gun_range(float gun_range);
	void set_attachment_dummy(XFormNode *att_dummy);

	void add_node(Holonode *node);
	void update();
	void render(unsigned int render_mask, long time = 0);
};

#endif
