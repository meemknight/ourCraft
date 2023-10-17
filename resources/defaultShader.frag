#version 430 core
#extension GL_ARB_bindless_texture: require

layout (location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D u_texture;
layout(binding = 1) uniform sampler2D u_numbers;

in vec2 v_uv;
in float v_color;

in flat uvec2 v_textureSampler;
in flat uvec2 v_normalSampler;
in flat int v_ambientLight;

in flat int v_skyLight;
in flat int v_normalLight;

uniform int u_showLightLevels;

uniform vec3 u_pointPosF;
uniform ivec3 u_pointPosI;
uniform vec3 u_sunDirection;

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
	float maxDist = 15;
	float a = 0.1;

	//if(dist >= maxDist)return 0;
	//return 1;

	return max(0, (maxDist-dist)/(dist*dist*a+maxDist));
}


//https://gamedev.stackexchange.com/questions/22204/from-normal-to-rotation-matrix#:~:text=Therefore%2C%20if%20you%20want%20to,the%20first%20and%20second%20columns.
mat3x3 NormalToRotation(in vec3 normal)
{
	// Find a vector in the plane
	vec3 tangent0 = cross(normal, vec3(1, 0, 0));
	if (dot(tangent0, tangent0) < 0.001)
		tangent0 = cross(normal, vec3(0, 1, 0));
	tangent0 = normalize(tangent0);
	// Find another vector in the plane
	vec3 tangent1 = normalize(cross(normal, tangent0));
	// Construct a 3x3 matrix by storing three vectors in the columns of the matrix

	return mat3x3(tangent0,tangent1,normal);

	//return ColumnVectorsToMatrix(tangent0, tangent1, normal);
}

vec3 applyNormalMap(vec3 inNormal)
{
	vec3 normal = texture(sampler2D(v_normalSampler), v_uv).rgb;

	normal = normalize(2*normal - 1.f);
	mat3 rotMat = NormalToRotation(inNormal);
	normal = rotMat * normal;
	normal = normalize(normal);
	return normal;
}

float computeLight(vec3 N, vec3 L, vec3 V)
{
	vec3 H = normalize(L + V);
			
	float shininess = 16;
	float spec = pow(max(dot(N, H), 0.0), shininess);
	float diffuse = max(dot(L, N), 0.0); 	
	
	return spec + diffuse;
}

void main()
{
	vec4 textureColor = texture(sampler2D(v_textureSampler), v_uv);
	if(textureColor.a < 0.1){discard;}

	float light = 0;	
	vec3 N = applyNormalMap(v_normal);
	vec3 V = normalize(compute(u_positionInt, u_positionFloat, fragmentPositionI, fragmentPositionF));

	for(int i=0; i< u_lightsCount; i++)
	{
		vec3 L = compute(lights[i].rgb, vec3(0), fragmentPositionI, fragmentPositionF);
		//vec3 L =compute(u_pointPosI, u_pointPosF, fragmentPositionI, fragmentPositionF);
		
		float menhetanDistance = dot((abs(fragmentPositionI-lights[i].rgb)),vec3(1));
		//float menhetanDistance = dot((abs(L)),vec3(1));

		float LightDist = length(L);
		if((15-menhetanDistance) + 0.1 > v_normalLight){continue;}
		L = normalize(L);		

		light += computeLight(N,L,V) * atenuationFunction(LightDist)*1.f;
	}
	//light = 0;

	//sun light
	if(v_skyLight > 0)
	{
		vec3 L = u_sunDirection;
		light += computeLight(N,L,V) * 1.f;
	}


	out_color = vec4(textureColor.rgb*(v_color+light),textureColor.a);
		

	if(u_showLightLevels != 0)
	{
		vec4 numbersColor = texture(u_numbers, vec2(v_uv.x / 16.0 + (1/16.0)*v_ambientLight, v_uv.y));
		if(numbersColor.a > 0.1)
		{out_color.rgb = mix(numbersColor.rgb, out_color.rgb, 0.5);}
	}

	out_color = clamp(out_color, vec4(0), vec4(1));
}