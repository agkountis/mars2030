#version 120

attribute vec3 attr_pos;
attribute vec4 attr_col;
attribute float attr_size;

uniform mat4 st_view_matrix;
uniform mat4 st_proj_matrix;
uniform vec2 st_screen_size;

varying vec4 color;

void main()
{
	vec4 local_pos = vec4(attr_pos, 1.0);
	gl_Position = st_proj_matrix * st_view_matrix * local_pos;

	float viewspace_z = (st_view_matrix * local_pos).z;
	vec4 p0 = st_proj_matrix * vec4(0.0, 0.0, viewspace_z, 1.0);
	vec4 p1 = st_proj_matrix * vec4(attr_size, 0.0, viewspace_z, 1.0);

	p0.xyz /= p0.w;
	p1.xyz /= p1.w;

	p0.xy = (p0.xy * vec2(0.5, 0.5) + vec2(0.5, 0.5)) * st_screen_size;
	p1.xy = (p1.xy * vec2(0.5, 0.5) + vec2(0.5, 0.5)) * st_screen_size;

	float sz = length(p1.xy - p0.xy);
	gl_PointSize = sz;

	color = attr_col;
}