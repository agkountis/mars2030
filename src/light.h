#ifndef LIGHT_H_
#define LIGHT_H_
#include "xform_node.h"

class Light:public XFormNode
{
private:
	Vector3 color_diff;
	Vector3 color_spec;
	float att_constant;
	float att_linear;
	float att_quadratic;

public:
	Light();
	~Light();

	const Vector3& get_color_diff() const;
	const Vector3& get_color_spec() const;
	const float get_att_constant() const;
	const float get_att_linear() const;
	const float get_att_quadratic() const;
	void set_color_diff(const Vector3 &col);
	void set_color_diff(const float r,const float g,const float b);
	void set_color_spec(const Vector3 &col);
	void set_color_spec(const float r,const float g,const float b);
	void set_att_constant(const float att_const);
	void set_att_linear(const float att_lin);
	void set_att_quadratic(const float att_quadr);
	void set_light();
    XFormNode *clone();
};

#endif //LIGHT_H_
