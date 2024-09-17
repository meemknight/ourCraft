#version 430 core



layout(location = 0) out vec4 outColor;
noperspective in vec2 v_texCoords;

uniform sampler2D u_toBlurcolorInput;

uniform bool u_horizontal;
uniform int u_mip;
uniform vec2 u_texel;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
	//vec2 texOffset = 1.0 / textureSize(u_toBlurcolorInput, u_mip); // gets size of single texel
	vec2 texOffset = u_texel; // gets size of single texel
	vec3 result = textureLod(u_toBlurcolorInput, v_texCoords, u_mip).rgb * weight[0]; // current fragment's contribution
	
	if(u_horizontal)
	{
		for(int i = 1; i < 5; ++i)
		{
			result += textureLod(u_toBlurcolorInput, v_texCoords + vec2(texOffset.x * i, 0.0), u_mip).rgb * weight[i];
			result += textureLod(u_toBlurcolorInput, v_texCoords - vec2(texOffset.x * i, 0.0), u_mip).rgb * weight[i];
		}
	}
	else
	{
		for(int i = 1; i < 5; ++i)
		{
			result += textureLod(u_toBlurcolorInput, v_texCoords + vec2(0.0, texOffset.y * i), u_mip).rgb * weight[i];
			result += textureLod(u_toBlurcolorInput, v_texCoords - vec2(0.0, texOffset.y * i), u_mip).rgb * weight[i];
		}
	}

	outColor.rgb = vec3(result);
	outColor.a = 1;

}