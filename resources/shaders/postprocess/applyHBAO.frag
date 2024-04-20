#version 330 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_hbao;
uniform sampler2D u_currentViewSpace;


uniform ShadingSettings
{
	vec3 u_waterColor;
	int u_tonemapper;
	vec3 u_underWaterColor;
	float u_fogDistance;
	float u_exposure;
	float u_underwaterDarkenStrength;
	float u_underwaterDarkenDistance;
	float u_fogGradientUnderWater;
	int u_shaders;
};

const float fogGradient = 16;

float computeFog(float dist)
{
	float rez = exp(-pow(dist*(1/(u_fogDistance-32)), fogGradient));
	if(rez > 0.9){return 1;};
	return rez;
}



void main()
{

	float dist = -texture2D(u_currentViewSpace, v_texCoords).z;

	outColor = vec4(0,0,0,texture2D(u_hbao, v_texCoords).r * 0.8 * 
		(computeFog(dist)) );
	//outColor = vec4(0,0,0,0.5);
}