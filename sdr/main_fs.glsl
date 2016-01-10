uniform vec3 st_light_color_diff;
uniform vec4 st_light_color_ambient;
uniform float st_light_intensity;
uniform vec4 st_mtl_diffuse;//color
uniform vec4 st_mtl_specular; //color
uniform float st_mtl_shininess;
uniform float st_mtl_alpha;

uniform sampler2D st_diff_tex;
uniform sampler2D st_spec_tex;
uniform sampler2D st_norm_map_tex;
uniform sampler2DShadow st_shadow2D_tex;

varying vec3 view_dir;
varying vec3 light_dir;
varying vec2 out_uv;
varying float atten;


varying vec4 shadow_coords;

void main()
{
    vec3 v_dir = normalize(view_dir);
    vec3 l_dir = normalize(light_dir);

    vec4 diff_texel = texture2D(st_diff_tex, out_uv); //diffuse texels.
    vec4 spec_texel = texture2D(st_spec_tex, out_uv); //specular texels.

    

	vec2 bias = vec2(1.0 / 2048.0, 1.0 / 2048.0);
	vec3 shad_tex_coord = shadow_coords.xyz / shadow_coords.w;
	vec3 p0 = shad_tex_coord - vec3(0.5 * bias.x, 0.5 * bias.y, 0.0);
	vec4 shadow = shadow2D(st_shadow2D_tex, p0);
	shadow += shadow2D(st_shadow2D_tex, p0 + vec3(bias.x, 0.0, 0.0));
	shadow += shadow2D(st_shadow2D_tex, p0 + vec3(0.0, bias.y, 0.0));
	shadow += shadow2D(st_shadow2D_tex, p0 + vec3(bias.x, bias.y, 0.0));
	shadow /= 4.0;

    float stex = shadow.x;

    float gloss_texel = spec_texel.a;
    float default_shininess = 60.0;//128.0;

    vec3 normal_map = normalize(texture2D(st_norm_map_tex, out_uv).xyz * 2.0 - 1.0); //normal map texels(moved to the 1/-1 range to be used as normals).
    vec3 reflection = reflect(-l_dir, normal_map);

    float diff_light = max(dot(normal_map, l_dir), 0.0) * st_light_intensity; //diffuse lighting calculation.
    float spec_light = pow(max(dot(reflection, v_dir), 0.0), default_shininess) * st_light_intensity; //specular lighting calculation.

    vec3 diff_color = diff_texel.xyz * st_light_color_diff * diff_light; //final diffuse color.
    vec3 spec_color = spec_texel.xyz * spec_light; //final specular color.

    vec3 ambient = diff_texel.xyz * st_light_color_ambient.xyz;
    vec3 final_color = ambient + (diff_color + spec_color) * atten * stex;


    float final_alpha = st_mtl_alpha * diff_texel.a;

    if(final_alpha < 0.05){
        discard;
    }

    gl_FragColor.rgb = final_color; //total final color calulation.
    gl_FragColor.a = st_mtl_alpha * diff_texel.a; //alpha value of the total final color.
}
