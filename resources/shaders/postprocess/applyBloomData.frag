#version 430 core

layout(location = 0) out vec4 outColor;
noperspective in vec2 v_texCoords;

layout(binding = 0) uniform sampler2D u_color;
layout(binding = 1) uniform sampler2D u_lensDirt;
layout(binding = 2) uniform sampler2D u_hitDirt;
layout(binding = 3) uniform sampler2D u_waterDirt;

uniform float u_waterDropsPower;

vec3 reinhard(vec3 v)
{
	return v / (1.0f + v);
}


vec3 addLensBlur(vec3 bloom, float presence, float strength)
{
	return reinhard(bloom * presence) * strength;
}

void main()
{
	vec3 readColor = texture(u_color, v_texCoords).rgb;
	vec3 dirtIntensity = texture(u_lensDirt, v_texCoords).rgb;
	vec3 hitIntensity = texture(u_hitDirt, v_texCoords).rgb;
	vec3 waterIntensity = texture(u_waterDirt, v_texCoords).rgb;


	const float lensDirtStrength = 2.0; //final power
	const float lensDirtPresence = 1.5; //how easy is it for it to appear
	readColor = min(readColor, vec3(1,1,1) * 2);

	readColor += addLensBlur(dirtIntensity * readColor, lensDirtPresence, lensDirtStrength);
	//readColor += addLensBlur(hitIntensity * readColor, 10, 4);
	readColor += addLensBlur(waterIntensity * readColor, 6 * u_waterDropsPower, 4);

	outColor.a = 1;
	outColor.rgb = readColor;
}