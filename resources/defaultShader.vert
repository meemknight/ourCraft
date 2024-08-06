#version 430 core

layout(location = 0) in int in_faceOrientation; //up down left etc
layout(location = 1) in int in_textureIndex; //dirt grass stone etc
layout(location = 2) in ivec3 in_facePosition; // int x y z

//y component:
//0x    FF      FF      FF    FF
//   -flags----light----position--


ivec3 facePosition;

uniform mat4 u_viewProjection;
uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;
uniform float u_time;
uniform int u_skyLightIntensity;

uniform mat4 u_lightSpaceMatrix;
uniform ivec3 u_lightPos;

out flat ivec3 v_blockPos;

//todo remove? or stop using after a size
float vertexColor[] = float[](
		
	//front
	0.95,
	//back
	0.85,
	//top
	1.0,
	//bottom
	0.8,
	//left
	0.85,
	//right
	0.95,

	//grass
	0.95,
	0.95,
	0.95,
	0.95,

	// leaves
	//front
	0.95,
	//back
	0.85,
	//top
	1.0,
	//bottom
	0.8,
	//left
	0.85,
	//right
	0.95


);

//geometry
readonly restrict layout(std430) buffer u_vertexData
{
	float vertexData[];
};

readonly restrict layout(std430) buffer u_vertexUV
{
	float vertexUV[];
};

//todo change and send directly the texture's id
readonly restrict layout(std430) buffer u_textureSamplerers
{
	uvec2 textureSamplerers[];
};


out vec2 v_uv;
out float v_ambient;
out flat int v_ambientInt;

out flat uvec2 v_textureSampler;
out flat uvec2 v_normalSampler;
out flat uvec2 v_materialSampler;

//in world space
out flat ivec3 fragmentPositionI;
out vec3 fragmentPositionF;

out flat int v_skyLight;
out flat int v_normalLight;
out flat int v_skyLightUnchanged;

out flat vec3 v_normal;

//in view space
out vec4 v_fragPos;

out vec4 v_fragPosLightSpace;


out flat int v_flags;

vec2 calculateUVs(int vertexId)
{	
	vec2 uvs; 
	uvs.x = vertexUV[(in_faceOrientation) * 2 * 4 + vertexId * 2 + 0];
	uvs.y = vertexUV[(in_faceOrientation) * 2 * 4 + vertexId * 2 + 1];
	return uvs;
}

uniform float u_timeGrass;

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

out vec3 v_semiViewSpacePos;

vec3 normals[6] = {vec3(0,0,1),vec3(0,0,-1),vec3(0,1,0),vec3(0,-1,0),vec3(-1,0,0),vec3(1,0,0)};

void main()
{
	facePosition = in_facePosition; 
	facePosition.y &= 0x0000FFFF;
	
	int in_skyAndNormalLights = in_facePosition.y >> 16;
	in_skyAndNormalLights &= 0xFF;
	
	int in_flags =  in_facePosition.y >> 24;
	in_flags &= 0xFF;
	bool isWater =  ((in_flags & 1) != 0);

	v_skyLight = (in_skyAndNormalLights & 0xf0) >> 4;
	v_normalLight = (in_skyAndNormalLights & 0xf);
	v_skyLightUnchanged = v_skyLight;

	v_skyLight = max(v_skyLight - (15 - u_skyLightIntensity), 0);

	v_ambientInt = max(v_skyLight, v_normalLight);

	vec3 diffI = facePosition - u_positionInt;
	vec3 diffF = diffI - u_positionFloat;
	
	v_flags = in_flags;
	v_blockPos = facePosition;

	fragmentPositionF = calculateVertexPos(gl_VertexID);
	fragmentPositionI = facePosition;
	
	vec4 posView = vec4(fragmentPositionF + diffF,1);
	

	v_uv = calculateUVs(gl_VertexID);
	
	//todo don't do animations at far distances.
	//calculate normals	

	if(in_faceOrientation < 6)
	{
		v_normal = normals[in_faceOrientation];
	}
	else
	if(in_faceOrientation >= 22 && in_faceOrientation < 28) //water
	{
		v_normal = normals[in_faceOrientation-22];
	}
	else
	{
		vec3 pos1 = vec3(0);
		vec3 pos2 = vec3(0);
		vec3 pos3 = vec3(0);
	
		pos1 = posView.xyz;
		pos2 = calculateVertexPos((gl_VertexID + 1)%4) + diffF;
		pos3 = calculateVertexPos((gl_VertexID + 2)%4) + diffF;
		
		vec3 a = (pos3-pos2);
		vec3 b = (pos1-pos2);
		v_normal = normalize(cross(a, b));

	}

	//apply curvature
	if(false)	
	{
		float fragmentDist = length(posView.xyz);
		float curved = posView.y - 0.001f * fragmentDist * fragmentDist;
		posView.y = curved;
	}

	v_semiViewSpacePos = posView.xyz;
	vec4 posProjection = u_viewProjection * posView;

	gl_Position = posProjection;
	v_fragPos = posProjection;

	v_fragPosLightSpace = u_lightSpaceMatrix * vec4((fragmentPositionI - u_lightPos) + fragmentPositionF, 1);
	//v_fragPosLightSpace.xyz -= u_lightPos;


	if(in_faceOrientation<16 && !isWater)
	{
		v_ambient = (vertexColor[in_faceOrientation] * (v_ambientInt/15.f));
		//v_ambient = (v_ambientInt/15.f);
	}else
	{
		v_ambient = (v_ambientInt/15.f);
	}
	//v_color = vertexColor[in_faceOrientation];


	v_textureSampler = textureSamplerers[in_textureIndex];
	v_normalSampler = textureSamplerers[in_textureIndex+1];
	v_materialSampler = textureSamplerers[in_textureIndex+2];

}