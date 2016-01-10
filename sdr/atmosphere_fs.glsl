uniform float st_mtl_alpha;

varying float fresnel_term;

void main()
{
    float final_alpha = st_mtl_alpha;

	//vec3(0.71, 0.384, 0.235)
	//vec3(0.607, 0.184, 0.082)
	//vec3(0.772, 0.596, 0.333)
	//vec3(0.8, 0.6, 0.0)
	//vec3(0.607, 0.250, 0.082)
	//vec3(1.0, 0.45, 0.0)
    gl_FragColor.rgb = pow(vec3(1.0, 0.45, 0.0), vec3(2.2, 2.2, 2.2));
    gl_FragColor.a = final_alpha * fresnel_term;
}
