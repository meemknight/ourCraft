#version 330 core

layout (location = 0) in vec2 p;

uniform mat4 u_mat;

void main()
{
	
	gl_Position = u_mat * vec4(p, 0, 1);

}