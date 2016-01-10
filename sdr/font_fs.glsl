uniform sampler2D font_tex;
uniform float st_time_msec;
uniform vec4 st_font_color;

varying vec2 out_uv;


void main()
{
	vec4 texel = texture2D(font_tex, out_uv);

        gl_FragColor.rgb = texel.rgb + pow(st_font_color.rgb, vec3(2.2, 2.2, 2.2));
	gl_FragColor.a = texel.a * st_font_color.a;
}
