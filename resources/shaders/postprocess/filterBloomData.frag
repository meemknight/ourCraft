#version 430 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

layout(binding = 0) uniform sampler2D u_color;

uniform float u_exposure;
uniform float u_tresshold;
uniform float u_multiplier;

void main()
{

	vec3 readColor = texture(u_color, v_texCoords).rgb;


	float lightIntensity = dot(readColor.rgb, vec3(0.2126, 0.7152, 0.0722));

	outColor.a = 1;

	if(lightIntensity * u_exposure > u_tresshold)
	{
		outColor.rgb = readColor * u_multiplier;
	}else
	{
		outColor.rgb = vec3(0,0,0);
	}


}