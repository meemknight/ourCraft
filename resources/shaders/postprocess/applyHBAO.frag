#version 330 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_hbao;
uniform sampler2D u_currentViewSpace;
uniform float u_viewDistance;

const float fogGradient = 16;

float computeFog(float dist)
{
	//return 1;
	//float rez = exp(-pow(dist*(1/(u_fogDistance-32)), fogGradient));
	float rez = exp(-pow(dist*(1/(u_viewDistance)), fogGradient));
	if(rez > 0.9){return 1;};
	rez = pow(rez, 16);
	return rez;
}

const float AO_STRENGTH = 1.45;


void main()
{

	float dist = -texture2D(u_currentViewSpace, v_texCoords).z;

	vec2 texelSize = 1.0 / vec2(textureSize(u_hbao, 0));
	float aoValue = 0.0;
	for (int y = -2; y < 2; ++y) 
	{
		for (int x = -2; x < 2; ++x) 
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			aoValue += textureLod(u_hbao, v_texCoords + offset, 0).r;
		}
	}

	aoValue = aoValue / (4.0 * 4.0);
	//aoValue += textureLod(u_hbao, v_texCoords, 0).r;


	outColor = vec4(0,0,0, aoValue * AO_STRENGTH * (computeFog(dist)) );
	//outColor = vec4(0,0,0,0);
}