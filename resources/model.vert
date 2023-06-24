#version 430 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normals;
layout(location = 2) in vec2 in_uvs;

uniform mat4 u_viewProjection;

uniform ivec3 u_cameraPositionInt;
uniform vec3 u_cameraPositionFloat;

uniform ivec3 u_entityPositionInt;
uniform vec3 u_entityPositionFloat;

void main()
{

	ivec3 intPosition = u_entityPositionInt - u_cameraPositionInt;
	vec3 floatPosition = intPosition - u_cameraPositionFloat;
	
	vec4 pos = vec4(floatPosition.xyz,1);
	pos.xyz += in_vertexData;

	pos = u_viewProjection * pos;

	gl_Position = pos;	
}