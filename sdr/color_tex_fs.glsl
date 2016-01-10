uniform sampler2D st_fbo_color_tex;

varying vec2 out_uv;

void main()
{
    vec4 final_color = texture2D(st_fbo_color_tex, out_uv);

    gl_FragColor = final_color;
}
