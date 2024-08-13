#version 430 core
#extension GL_ARB_bindless_texture: require

layout (location = 0) out vec4 out_color;

in flat uvec2 v_textureSampler;
in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_crackTexture;

uniform int u_crackPosition;

void main()
{


	{
	//	float a = texture(sampler2D(v_textureSampler), v_uv).a;
	//	if(a <= 0){discard;}
	}

	vec2 uv = v_uv;
	uv.x += u_crackPosition;
	uv.x/=10;

	vec4 c = texture(u_crackTexture, uv).rgba;
	if(c.a < 0.01){discard;}

	out_color = vec4(c.rgb,1);

}