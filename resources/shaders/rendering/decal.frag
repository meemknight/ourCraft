#version 430 core
#extension GL_ARB_bindless_texture: require

layout (location = 0) out vec4 out_color;

in flat uvec2 v_textureSampler;
in vec2 v_uv;

void main()
{

	{
	//	float a = texture(sampler2D(v_textureSampler), v_uv).a;
	//	if(a <= 0){discard;}
	}

	out_color = vec4(1,0,0,1);

}