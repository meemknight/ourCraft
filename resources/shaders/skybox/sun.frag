#version 430 core
layout(location = 0) out vec4 color;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_sunTexture;
layout (location = 1) out vec4 out_screenSpacePositions;
layout (location = 2) out ivec3 out_normals;
layout (location = 3) out vec3 out_bloom;



void main()
{
	vec4 c = texture(u_sunTexture, v_uv).rgba;
	color.rgba = c;
	color.rgb *= 1.5;

	out_bloom.rgb = color.rgb * 0.014;
}