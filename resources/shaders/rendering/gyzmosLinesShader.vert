#version 430 core

layout(location = 0) in vec3 in_vertexData;

uniform mat4 u_gyzmosLineShaderViewProjection;

void main()
{
	gl_Position = u_gyzmosLineShaderViewProjection * vec4(in_vertexData, 1);
}