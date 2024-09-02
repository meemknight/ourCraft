#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform mat4 u_viewProjection;
uniform mat4 u_modelMatrix;

out vec2 v_uv;

void main()
{

	gl_Position = u_viewProjection * u_modelMatrix * vec4(position, 1);
	v_uv = uv;


}