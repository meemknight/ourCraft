#version 330 core


out vec3 outColor;

void doLightCalculations(vec3 N, vec3 L)
{

	vec4 color = texture(u_color, v_uv);
	vec3 color.rgb = pow(color.rgb, vec3(2.2));
	
	//do all your things
	

	color = toneMapper(color * u_exposure);

	outColor = pow(color.rgb, vec3(1/2.2));


}



