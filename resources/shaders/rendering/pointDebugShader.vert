#version 430 core

uniform mat4 u_viewProjection;
uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;

uniform ivec3 u_blockPositionInt;
uniform vec3 u_blockPositionFloat;

void main()
{

	ivec3 intPosition = u_blockPositionInt - u_positionInt;
	vec3 floatPosition = intPosition - u_positionFloat;
	
	vec4 pos = vec4(floatPosition.xyz,1);
	pos.xyz += u_blockPositionFloat;

	pos = u_viewProjection * pos;

	gl_Position = pos;	
}