#version 430 core
#extension GL_ARB_bindless_texture: require

layout (location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D u_texture;
layout(binding = 1) uniform sampler2D u_numbers;

in vec2 v_uv;
in float v_color;

in flat uvec2 v_textureSampler;
in flat int v_skyLight;

uniform int u_showLightLevels;

void main()
{
	//vec4 textureColor = texture(u_texture, v_uv);
	
	vec4 textureColor = texture(sampler2D(v_textureSampler), v_uv);
	vec4 numbersColor = texture(u_numbers, vec2(v_uv.x / 16.0 + (1/16.0)*v_skyLight, v_uv.y));
	
	if(textureColor.a < 0.1){discard;}
	out_color = vec4(textureColor.rgb*v_color,textureColor.a);

	if(u_showLightLevels != 0)
	{
		if(numbersColor.a > 0.1)
		{out_color.rgb = mix(numbersColor.rgb, out_color.rgb, 0.5);}
	}
}