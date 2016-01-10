uniform samplerCube st_diff_tex;

varying vec3 normal;

void main(void)
{
    vec3 norm = normalize(normal);
    vec4 diff_texel = textureCube(st_diff_tex,norm); //diffuse texels.
    gl_FragColor.rgb = diff_texel.rgb * 0.3;
    gl_FragColor.a = 1.0;
}
