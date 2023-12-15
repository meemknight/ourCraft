#version 330
//https://learnopengl.com/PBR/IBL/Diffuse-irradiance
//this shader converts from a hdr image to a cubemap
//todo rename shader

layout (location = 0) in vec3 aPos;

out vec3 v_localPos;

uniform mat4 u_viewProjection;


void main()
{
	v_localPos = aPos;
	gl_Position = u_viewProjection * vec4(aPos, 1.0);
}  