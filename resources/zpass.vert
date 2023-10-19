#version 430 core

layout(location = 0) in int in_faceOrientation; //up down left etc
layout(location = 1) in int in_textureIndex; //dirt grass stone etc
layout(location = 2) in ivec3 in_facePosition; // int x y z
layout(location = 3) in int in_skyAndNormalLights; 

uniform mat4 u_viewProjection;
uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;
uniform float u_time;

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

vec3 calculateVertexPos(int vertexId)
{

	vec3 pos = vec3(0);
	vec3 vertexShape = vec3(0);


	vertexShape.x = vertexData[in_faceOrientation * 3 * 6 + vertexId * 3 + 0];
	vertexShape.y = vertexData[in_faceOrientation * 3 * 6 + vertexId * 3 + 1];
	vertexShape.z = vertexData[in_faceOrientation * 3 * 6 + vertexId * 3 + 2];

	if(in_faceOrientation >= 10) //animated
	{

		if(in_facePosition.y % 2 == 0)
		{
			vertexShape.x = -vertexShape.x;
			vertexShape.z = -vertexShape.z;
			//v_uv.x = 1.f-v_uv.x;
		}

		vertexShape.x += vertexData[(in_faceOrientation-10) * 3 * 6 + vertexId * 3 + 0];
		vertexShape.y += vertexData[(in_faceOrientation-10) * 3 * 6 + vertexId * 3 + 1];
		vertexShape.z += vertexData[(in_faceOrientation-10) * 3 * 6 + vertexId * 3 + 2];

	}

	pos.xyz += vertexShape;

	return pos;
}


void main()
{

	vec3 diffI = in_facePosition - u_positionInt;
	vec3 diffF = diffI - u_positionFloat;
	

	vec3 fragmentPositionF = calculateVertexPos(gl_VertexID);
	ivec3 fragmentPositionI = in_facePosition;
	
	vec4 posView = vec4(fragmentPositionF + diffF,1);
	
	posView = u_viewProjection * posView;

	gl_Position = posView;
	


}