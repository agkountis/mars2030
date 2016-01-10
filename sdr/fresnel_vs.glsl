#version 120

attribute vec3 attr_vertex;
attribute vec3 attr_normal;

uniform mat4 st_view_matrix;
uniform mat3 st_view_matrix3;
uniform mat4 st_view_matrix_inverse;
uniform mat4 st_world_matrix;
uniform mat3 st_world_matrix3;
uniform mat4 st_world_matrix_inverse;
uniform mat4 st_proj_matrix;

varying vec3 reflect_vec;
varying float ratio;

void main()
{
	vec4 local_pos = vec4(attr_vertex, 1.0);
	gl_Position = st_proj_matrix * st_view_matrix * st_world_matrix * local_pos;

	float fresnel_power = 5.0;
	float fresnel_bias = 0.10;
	float fresnel_scale = 1.0 - fresnel_bias;

	vec4 eye_pos = st_view_matrix * st_world_matrix * local_pos;

	vec3 i = normalize(eye_pos.xyz);
	vec3 n = normalize(st_view_matrix3 * st_world_matrix3 * attr_normal);

	ratio = fresnel_bias + fresnel_scale * pow((1.0 + dot(i, n)), fresnel_power);

	reflect_vec = reflect(i, n);
	reflect_vec = ( st_world_matrix_inverse * st_view_matrix_inverse * vec4(reflect_vec, 0.0)).xyz;
}