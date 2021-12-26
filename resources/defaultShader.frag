#version 430 core

layout (location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D u_texture;

in vec2 v_uv;
in float v_color;

void main()
{
	vec4 textureColor = texture(u_texture, v_uv);
	out_color = vec4(textureColor.rgb*v_color,1.0);
}