uniform sampler2D st_shadow2D_tex;


void main()
{
    vec4 diff_texel = texture2D(st_shadow2D_tex, gl_TexCoord[0].xy);

    gl_FragColor = diff_texel;
}
