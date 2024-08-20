#version 430 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec2 v_uv;

uniform mat4 u_modelViewProjectionMatrix;

void main()
{
	v_uv = uv;

	
	vec4 pos = vec4(position, -4, 1);

	gl_Position = u_modelViewProjectionMatrix * pos;
	gl_Position.z = gl_Position.w;

}