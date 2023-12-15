#version 330

layout (location = 0) in vec3 aPos;

out vec3 v_texCoords;

uniform mat4 u_viewProjection;


void main()
{
	v_texCoords = aPos;
	vec4 pos = u_viewProjection * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}  