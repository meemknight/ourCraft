#version 430 core
layout(location = 0) out vec4 color;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_sunTexture;

void main()
{
	vec4 c = texture(u_sunTexture, v_uv).rgba;
	color.rgba = c;
	color.rgb *= 1.5;
}