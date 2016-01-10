uniform sampler2D st_diff_tex;
uniform float st_mtl_alpha;

varying vec2 out_uv;

void main()
{
    vec4 diff_texel = texture2D(st_diff_tex, out_uv);
    float alpha = st_mtl_alpha * diff_texel.a;


    gl_FragColor = vec4(1.0, 1.0, 1.0, alpha);
}
