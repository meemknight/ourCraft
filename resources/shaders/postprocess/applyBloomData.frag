#version 430 core

layout(location = 0) out vec4 outColor;
noperspective in vec2 v_texCoords;

layout(binding = 0) uniform sampler2D u_color;


void main()
{
	vec3 readColor = texture(u_color, v_texCoords).rgb;
	outColor.a = 1;
	outColor.rgb = readColor;
}