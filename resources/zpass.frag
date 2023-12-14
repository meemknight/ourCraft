#version 430 core
#extension GL_ARB_bindless_texture: require

in flat uvec2 v_textureSampler;
in vec2 v_uv;

void main()
{

	vec4 textureColor;
	{
		float a = texture(sampler2D(v_textureSampler), v_uv).a;
		if(a <= 0){discard;}
	}

}