#version 430 core
/*
	http://advances.realtimerendering.com/s2014/index.html
	NEXT GENERATION POST PROCESSING IN CALL OF DUTY: ADVANCED WARFARE
*/

layout(location = 0) out vec4 outColor;
noperspective in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform int u_mip;


void main()
{
	vec2 texel = 1.f / textureSize(u_texture, u_mip).xy;
	outColor.rgb = vec3(0.f);

	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(-1, +1), u_mip).rgb;
	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(0, +1), u_mip).rgb*2;
	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(+1, +1), u_mip).rgb;
	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(-1, 0), u_mip).rgb*2;
	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(0, 0), u_mip).rgb*4;
	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(+1, 0), u_mip).rgb*2;
	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(-1, -1), u_mip).rgb;
	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(0, -1), u_mip).rgb*2;
	outColor.rgb += textureLod(u_texture, v_texCoords + texel*vec2(+1, -1), u_mip).rgb;

	//outColor.rgb /= 16.f;
	outColor.rgb /= 12.f;
	outColor.a = 1;

}

