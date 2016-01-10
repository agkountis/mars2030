attribute vec3 attr_vertex;
attribute vec3 attr_normal;
attribute vec3 attr_tangent;
attribute vec2 attr_uv;

uniform mat4 st_view_matrix;
uniform mat3 st_view_matrix3;
uniform mat4 st_world_matrix;
uniform mat3 st_world_matrix3;
uniform mat4 st_proj_matrix;
uniform vec3 st_light_pos;
uniform float st_light_att_constant;
uniform float st_light_att_linear;
uniform float st_light_att_quadratic;

varying vec3 view_dir;
varying vec3 light_dir;
varying vec2 out_uv;
varying float atten;

void main()
{
        vec4 local_pos = vec4(attr_vertex,1.0);
        gl_Position = (st_proj_matrix * st_view_matrix * st_world_matrix) * local_pos;

        vec4 view_pos_calc = ((st_view_matrix * st_world_matrix) * local_pos);

        vec3 view_pos = view_pos_calc.xyz;
        view_dir = -view_pos;

        vec4 lpos = st_view_matrix * vec4(st_light_pos, 1.0);

        light_dir = lpos.xyz - view_pos;

        float dist = length(light_dir);
        atten = 1.0 / (st_light_att_constant + st_light_att_linear * dist + st_light_att_quadratic * dist * dist);

        mat3 st_normal_matrix = st_view_matrix3 * st_world_matrix3;

        vec3 normal = st_normal_matrix * attr_normal;
        vec3 tangent = st_normal_matrix * attr_tangent;
        vec3 binormal = cross(tangent,normal);


        mat3 tbn_matrix = mat3(tangent, binormal, normal);
        light_dir *= tbn_matrix;
        view_dir *= tbn_matrix;
        out_uv = attr_uv;
}
