#version 330 core

layout(location = 0) in vec4 in_pos;

uniform mat4 u_viewProjection;

void main()
{
	
	vec4 pos = u_viewProjection * in_pos;

	gl_Position = pos;

}