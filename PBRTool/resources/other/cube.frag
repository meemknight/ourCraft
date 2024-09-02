#version 430 core
layout (location = 0) out vec4 outColor;

in vec2 v_uv;


layout(binding = 0) uniform sampler2D u_texture;

void main()
{
	vec4 color = texture(u_texture, v_uv);
	if(color.a <= 0){discard;}

	outColor = color;
}