#version 120

uniform sampler2D st_diff_tex;
uniform float st_heat_factor;

varying vec4 color;

void main()
{
        vec4 diff_col = texture2D(st_diff_tex, gl_PointCoord);
        gl_FragColor.rgb = diff_col.rgb * color.rgb * vec3(1.0, clamp(1.0 - st_heat_factor * 1.51, 0.0, 1.0), clamp(1.0 - st_heat_factor * 1.51, 0.0, 1.0));
        gl_FragColor.a = color.a;
}