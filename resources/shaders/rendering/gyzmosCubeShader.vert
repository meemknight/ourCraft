#version 430 core

layout(location = 0) in vec3 in_vertexData;
layout(location = 1) in ivec3 in_blockPosition;

uniform mat4 u_viewProjection;
uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;

void main()
{

	ivec3 intPosition = in_blockPosition - u_positionInt;
	vec3 floatPosition = intPosition - u_positionFloat;
	
	vec4 pos = vec4(floatPosition.xyz,1);
	pos.xyz += in_vertexData;

	pos = u_viewProjection * pos;

	gl_Position = pos;	
}