uniform sampler2D font_tex;
uniform float st_time_msec;
uniform vec4 st_font_color;

varying vec2 out_uv;


void main()
{
	vec4 texel = texture2D(font_tex, out_uv);

	float scalar = cos(st_time_msec / 300.0) * 0.2 + 0.8;

	gl_FragColor.rgb = texel.rgb + vec3(st_font_color.r * scalar, st_font_color.g, st_font_color.b);
	gl_FragColor.a = texel.a * st_font_color.a * scalar;
}