#version 120

uniform sampler2D st_diff_tex;
varying vec4 color;

void main()
{
        vec4 diff_col = texture2D(st_diff_tex, gl_PointCoord);
        gl_FragColor.rgb = diff_col.rgb * color.rgb;
        gl_FragColor.a = color.a;
}
