#version 430 core

layout(location = 0) in int in_faceOrientation; //up down left etc
layout(location = 1) in int in_textureIndex; //dirt grass stone etc
layout(location = 2) in ivec3 in_facePosition; // int x y z

ivec3 facePosition;

uniform mat4 u_viewProjection;
uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;

out vec2 v_uv;
out flat uvec2 v_textureSampler;

uniform int u_renderOnlyWater;

uniform float u_timeGrass;

//geometry
readonly restrict layout(std430) buffer u_vertexData
{
	float vertexData[];
};

readonly restrict layout(std430) buffer u_vertexUV
{
	float vertexUV[];
};

readonly restrict layout(std430) buffer u_textureSamplerers
{
	uvec2 textureSamplerers[];
};

uint grassMask[] =
{
	//grass
	1,
	1,
	0,
	0,

	0,
	1,
	1,
	0,

	1,
	1,
	0,
	0,

	0,
	0,
	1,
	1,
};

int waterMask[] =
{
	1,1,0,0, 0,1,1,0, 1,1,1,1, 0,0,0,0, 0,1,1,0, 1,1,0,0,  

	//1,1,0,0, 0,1,1,0, 0,1,1,0, 1,1,0,0,
	// 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

	//1,1,-1,-1, -1,1,1,-1, -1,1,1,-1, 1,1,-1,-1,
	// 0,0,-1,-1, -1,0,0,-1, -1,0,0,-1, 0,0,-1,-1,

	 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
	 0,0,1,1, 1,0,0,1, 1,0,0,1, 0,0,1,1,
};

float biasUp(float a)
{
	return (a + 0.75)/1.75;
}

float biasUp2(float a)
{
	return ((a + 1)/2.f)/2.0 + 0.5;
}

vec3 calculateVertexPos(int vertexId)
{

	vec3 pos = vec3(0);
	vec3 vertexShape = vec3(0);

	if(in_faceOrientation >= 22) //water
	{
			int mask = waterMask[(in_faceOrientation-22)*4+vertexId];

		vertexShape.x += vertexData[(in_faceOrientation) * 3 * 4 + vertexId * 3 + 0];
		vertexShape.y += vertexData[(in_faceOrientation) * 3 * 4 + vertexId * 3 + 1];
		vertexShape.z += vertexData[(in_faceOrientation) * 3 * 4 + vertexId * 3 + 2];

		float SPEED = 3.1f;		
		float FREQUENCY = 0.5f;		
		float AMPLITUDE = 0.034f;		

		float SPEED2 = 1.92f;		
		float FREQUENCY2 = 0.04f;		
		float AMPLITUDE2 = 0.008f;		

		float offset = biasUp(cos((facePosition.x + vertexShape.x + vertexShape.z + 
		facePosition.z - u_timeGrass * SPEED) * FREQUENCY)) * AMPLITUDE;		

		float offset2 = (sin((1 + facePosition.x + vertexShape.x + (vertexShape.z + 
			facePosition.z) * 2 - u_timeGrass * SPEED) * FREQUENCY)) * AMPLITUDE;	

		vertexShape.y += mask * (offset + offset2 - 0.08);

	}else
	if(in_faceOrientation >= 10 && in_faceOrientation < 16) //animated trees
	{
		
		vertexShape.x += vertexData[(in_faceOrientation-10) * 3 * 4 + vertexId * 3 + 0];
		vertexShape.y += vertexData[(in_faceOrientation-10) * 3 * 4 + vertexId * 3 + 1];
		vertexShape.z += vertexData[(in_faceOrientation-10) * 3 * 4 + vertexId * 3 + 2];
		
		vec3 offset = vec3(0);		

		offset.y = 0.01*sin((u_timeGrass/0.2 )+vertexShape.x+facePosition.x);
		offset.y += 0.04*sin((u_timeGrass/1.0 )+vertexShape.z+facePosition.z);
		offset.xz = 0.05*sin((u_timeGrass/2.0 )+vertexShape.xz+facePosition.xz);

		offset.y += 0.010*cos((u_timeGrass/1.0 )+vertexShape.x+facePosition.x);
		offset.y += 0.02*cos((u_timeGrass/2.0 )+vertexShape.z+facePosition.z);
		offset.xz += 0.02*cos((u_timeGrass/3.0 )+vertexShape.xz+facePosition.xz);

		vertexShape += offset;

	}else if(in_faceOrientation >= 6 && in_faceOrientation < 16)
	{
		uint mask = grassMask[(in_faceOrientation-6)*4+vertexId];

		//grass
		vec2 dir = normalize(vec2(0.5,1));		
		float SPEED = 2.f;		
		float FREQUENCY = 2.5f;		
		float AMPLITUDE = 0.15f;		

		float SPEED2 = 1.f;		
		float FREQUENCY2 = 2.0f;		
		float AMPLITUDE2 = 0.9f;		
		vec2 dir2 = normalize(vec2(0.9,1));		

		float offset = biasUp(cos((facePosition.x * dir.x + 
		facePosition.z * dir.y - u_timeGrass * SPEED) * FREQUENCY)) * AMPLITUDE;		
	
		float offset2 = biasUp2(sin((facePosition.x * dir2.x +
			facePosition.z * dir2.y - u_timeGrass * SPEED) * FREQUENCY)) * AMPLITUDE;		

		vertexShape.x = vertexData[in_faceOrientation * 3 * 4 + vertexId * 3 + 0];
		vertexShape.y = vertexData[in_faceOrientation * 3 * 4 + vertexId * 3 + 1];
		vertexShape.z = vertexData[in_faceOrientation * 3 * 4 + vertexId * 3 + 2];
		
		vertexShape.x += mask * (offset * dir.x + offset2 * dir.x);
		vertexShape.z += mask * (offset * dir.y + offset2 * dir.y);

	}else
	{
		//todo optimize move up
		vertexShape.x = vertexData[in_faceOrientation * 3 * 4 + vertexId * 3 + 0];
		vertexShape.y = vertexData[in_faceOrientation * 3 * 4 + vertexId * 3 + 1];
		vertexShape.z = vertexData[in_faceOrientation * 3 * 4 + vertexId * 3 + 2];
	}

	pos.xyz += vertexShape;

	return pos;
}

vec2 calculateUVs(int vertexId)
{	
	vec2 uvs;
	uvs.x = vertexUV[(in_faceOrientation) * 2 * 4 + vertexId * 2 + 0];
	uvs.y = vertexUV[(in_faceOrientation) * 2 * 4 + vertexId * 2 + 1];
	return uvs;
}

void main()
{

	facePosition = in_facePosition; 
	facePosition.y &= 0x0000FFFF;
	
	int in_skyAndNormalLights = in_facePosition.y >> 16;
	in_skyAndNormalLights &= 0xFF;
	
	int in_flags =  in_facePosition.y >> 24;
	in_flags &= 0xFF;


	if((u_renderOnlyWater != 0) && ((in_flags & 1) == 0))
	{
		gl_Position = vec4(0,0,0,1);
		v_textureSampler = uvec2(0,0);
		return;
	}

	vec3 diffI = facePosition - u_positionInt;
	vec3 diffF = diffI - u_positionFloat;
	

	vec3 fragmentPositionF = calculateVertexPos(gl_VertexID);
	ivec3 fragmentPositionI = facePosition;
	v_uv = calculateUVs(gl_VertexID);
	

	vec4 posView = vec4(fragmentPositionF + diffF,1); //pos view semi actually
	
	//apply curvature
	if(false)	
	{
		float fragmentDist = length(posView.xyz);
		float curved = posView.y - 0.001f * fragmentDist * fragmentDist;
		posView.y = curved;
	}

	vec4 posProjection = u_viewProjection * posView;	



	gl_Position = posProjection;
	

	v_textureSampler = textureSamplerers[in_textureIndex];
}