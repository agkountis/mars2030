#version 120

attribute vec3 attr_vertex;
attribute vec3 attr_normal;

uniform mat4 st_view_matrix;
uniform mat3 st_view_matrix3;
uniform mat4 st_world_matrix;
uniform mat3 st_world_matrix3;
uniform mat4 st_proj_matrix;

varying vec3 normal;

void main(void)
{
    vec4 local_pos=vec4(-attr_vertex,1.0);
    gl_Position=(st_proj_matrix* mat4(st_view_matrix3)*st_world_matrix)*local_pos;

    mat3 normal_matrix= st_world_matrix3;

    normal= normal_matrix * (-attr_normal);
}
