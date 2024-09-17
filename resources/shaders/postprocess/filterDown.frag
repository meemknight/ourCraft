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
	vec2 texel2 = texel*2;

	vec3 topLeft	= textureLod(u_texture, v_texCoords + texel2*vec2(-1, +1), u_mip).rgb;
	vec3 topMid		= textureLod(u_texture, v_texCoords + texel2*vec2(0, +1), u_mip).rgb;
	vec3 topRight	= textureLod(u_texture, v_texCoords + texel2*vec2(+1, +1), u_mip).rgb;
	vec3 midLeft	= textureLod(u_texture, v_texCoords + texel2*vec2(-1, 0), u_mip).rgb;
	vec3 midMid		= textureLod(u_texture, v_texCoords + texel2*vec2(0, 0), u_mip).rgb;
	vec3 midRight	= textureLod(u_texture, v_texCoords + texel2*vec2(+1, 0), u_mip).rgb;
	vec3 bottomLeft	= textureLod(u_texture, v_texCoords + texel2*vec2(-1, -1), u_mip).rgb;
	vec3 bottomMid	= textureLod(u_texture, v_texCoords + texel2*vec2(0, -1), u_mip).rgb;
	vec3 bottomRight= textureLod(u_texture, v_texCoords + texel2*vec2(+1, -1), u_mip).rgb;

	vec3 center1 = textureLod(u_texture, v_texCoords + texel*vec2(-1, +1), u_mip).rgb;
	vec3 center2 = textureLod(u_texture, v_texCoords + texel*vec2(+1, +1), u_mip).rgb;
	vec3 center3 = textureLod(u_texture, v_texCoords + texel*vec2(+1, -1), u_mip).rgb;
	vec3 center4 = textureLod(u_texture, v_texCoords + texel*vec2(-1, -1), u_mip).rgb;

	vec3 midKernel = 0.5 * (center1 + center2 + center3 + center4) * 0.25;
	vec3 topLeftKernel = 0.125 * (topLeft + topMid + midLeft + midMid) * 0.25;
	vec3 topRightKernel = 0.125 * (topRight + topMid + midRight + midMid) * 0.25;
	vec3 bottomLeftKernel = 0.125 * (bottomLeft + bottomMid + midLeft + midMid) * 0.25;
	vec3 bottomRightKernel = 0.125 * (bottomRight + bottomMid + midRight + midMid) * 0.25;

	outColor.rgb = midKernel + topLeftKernel + topRightKernel + bottomLeftKernel + bottomRightKernel;
	outColor.a = 1;
}