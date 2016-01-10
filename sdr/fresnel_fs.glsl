uniform samplerCube st_diff_tex;

varying vec3 reflect_vec;
varying float ratio;

void main()
{
	vec3 reflect_color;

	reflect_color = textureCube(st_diff_tex, reflect_vec).rgb;

	vec3 final_color = reflect_color * ratio;

	gl_FragColor.rgb = final_color;//vec3(ratio, 0.0, 0.0);
	gl_FragColor.a = 1.0  * ratio; 
}