uniform vec4 st_mtl_diffuse;//color
uniform float st_mtl_alpha;
uniform float st_time_msec;

uniform sampler2D st_diff_tex;

varying vec2 out_uv;

void main()
{
    vec4 diff_texel = texture2D(st_diff_tex, out_uv);

    float final_alpha = st_mtl_alpha * diff_texel.a;

    if(final_alpha < 0.17){
        discard;
    }

    gl_FragColor.rgb = diff_texel.rgb * st_mtl_diffuse.rgb;
    gl_FragColor.a = final_alpha;
}

