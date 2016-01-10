attribute vec3 attr_vertex;
attribute vec2 attr_uv;

uniform mat4 st_view_matrix;
uniform mat4 st_world_matrix;
uniform mat4 st_proj_matrix;

varying vec2 out_uv;
varying vec3 out_local_pos;

void main()
{
        vec4 local_pos = vec4(attr_vertex,1.0);
        gl_Position = (st_proj_matrix * st_view_matrix * st_world_matrix) * local_pos;

		out_local_pos = local_pos.xyz;
        out_uv = attr_uv;
}
