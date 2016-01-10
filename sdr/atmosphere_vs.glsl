attribute vec3 attr_vertex;
attribute vec2 attr_uv;
attribute vec3 attr_normal;

uniform mat4 st_view_matrix;
uniform mat4 st_world_matrix;
uniform mat4 st_proj_matrix;
uniform mat3 st_view_matrix3;
uniform mat3 st_world_matrix3;

varying float fresnel_term;

void main()
{
    vec4 local_pos = vec4(attr_vertex,1.0);
    gl_Position = (st_proj_matrix * st_view_matrix * st_world_matrix) * local_pos;

	float fresnel_power = 3.0;//2.8;
	float fresnel_bias = 0.03;//0.18;//0.005;
	float fresnel_scale = 1.0 - fresnel_bias;

	vec4 eye_pos = st_view_matrix * st_world_matrix * local_pos;

	vec3 i = normalize(eye_pos.xyz);
	vec3 n = normalize(st_view_matrix3 * st_world_matrix3 * attr_normal);

	fresnel_term = fresnel_bias + fresnel_scale * pow((1.0 + dot(i, n)), fresnel_power);
}