#version 330 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_hbao;

void main()
{
	outColor = vec4(0,0,0,texture2D(u_hbao, v_texCoords).r);
	//outColor = vec4(0,0,0,0.5);
}