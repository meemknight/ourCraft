#version 430 core
#extension GL_ARB_bindless_texture: require

layout (location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D u_texture;
layout(binding = 1) uniform sampler2D u_numbers;

in vec2 v_uv;
in float v_color;

in flat uvec2 v_textureSampler;
in flat int v_skyLight;

uniform int u_showLightLevels;

uniform vec3 u_pointPosF;
uniform ivec3 u_pointPosI;

readonly restrict layout(std430) buffer u_lights
{
	ivec4 lights[];
};

uniform int u_lightsCount;

uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;

in flat ivec3 fragmentPositionI;
in vec3 fragmentPositionF;
in flat vec3 v_normal;

vec3 compute(ivec3 destI, vec3 destF, ivec3 sourceI, vec3 sourceF)
{
	ivec3 intPart = destI - sourceI;
	vec3 floatPart = destF - sourceF;

	return floatPart + vec3(intPart);
}

float atenuationFunction(float dist)
{
	float maxDist = 16;
	float a = 1;

	return max(0, (maxDist-dist)/(dist*dist*a+maxDist));
}

void main()
{
	vec4 textureColor = texture(sampler2D(v_textureSampler), v_uv);
	if(textureColor.a < 0.1){discard;}

	float light = 0;	
	vec3 N = v_normal;

	for(int i=0; i< u_lightsCount; i++)
	{
		vec3 L = compute(lights[i].rgb, vec3(0), fragmentPositionI, fragmentPositionF);
		//vec3 L =compute(u_pointPosI, u_pointPosF, fragmentPositionI, fragmentPositionF);

		float LightDist = length(L);
		L = normalize(L);		

		vec3 V    = normalize(compute(u_positionInt, u_positionFloat, fragmentPositionI, fragmentPositionF));
		vec3 H = normalize(L + V);
			
		float shininess = 16;
		//float spec = pow(max(dot(N, H), 0.0), shininess);
		float diffuse = max(dot(L, N), 0.0); 	
	
		//light += spec + diffuse;
		light += diffuse * atenuationFunction(LightDist);
	}
	//light = 0;


	out_color = vec4(textureColor.rgb*(v_color+light),textureColor.a);
		

	if(u_showLightLevels != 0)
	{
		vec4 numbersColor = texture(u_numbers, vec2(v_uv.x / 16.0 + (1/16.0)*v_skyLight, v_uv.y));
		if(numbersColor.a > 0.1)
		{out_color.rgb = mix(numbersColor.rgb, out_color.rgb, 0.5);}
	}

	out_color = clamp(out_color, vec4(0), vec4(1));
}