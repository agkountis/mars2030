#include "light.h"
#include "unistate.h"

Light::Light()
{
	color_diff=Vector3(0,0,0);
	color_spec=Vector3(0,0,0);
    att_constant = att_linear = att_quadratic = 0;
    set_node_type(NODE_LIGHT);
}

Light::~Light()
{
}

const Vector3& Light::get_color_diff() const
{
	return color_diff;
}

const Vector3& Light::get_color_spec() const
{
	return color_spec;
}

const float Light::get_att_constant() const
{
	return att_constant;
}

const float Light::get_att_linear() const
{
	return att_linear;
}

const float Light::get_att_quadratic() const
{
	return att_quadratic;
}

void Light::set_color_diff(const Vector3 &col)
{
    color_diff.x = col.x;
    color_diff.y = col.y;
    color_diff.z = col.z;
}

void Light::set_color_diff(const float r,const float g,const float b)
{
    color_diff.x = r;
    color_diff.y = g;
    color_diff.z = b;
}

void Light::set_color_spec(const Vector3 &col)
{
    color_spec.x = col.x;
    color_spec.y = col.y;
    color_spec.z = col.z;
}

void Light::set_color_spec(const float r,const float g,const float b)
{
	color_spec.x=r;
	color_spec.y=g;
	color_spec.z=b;
}

void Light::set_att_constant(const float att_const)
{
	att_constant=att_const;
}

void Light::set_att_linear(const float att_lin)
{
    att_linear = att_lin;
}

void Light::set_att_quadratic(const float att_quadr)
{
    att_quadratic = att_quadr;
}

void Light::set_light()
{
	set_unistate("st_light_pos",get_position());
	set_unistate("st_light_color_diff",get_color_diff());
	set_unistate("st_light_att_constant",get_att_constant());
	set_unistate("st_light_att_linear",get_att_linear());
	set_unistate("st_light_att_quadratic",get_att_quadratic());
}

XFormNode* Light::clone()
{
    return new Light(*this);
}
