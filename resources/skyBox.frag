#version 330 core
//
// Atmospheric scattering fragment shader
//
// Author: Sean O'Neil
//
// Copyright (c) 2004 Sean O'Neil
//
//--//https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering
//moddified

layout(location = 0) out vec4 outColor;

in vec3 v_vsViewDirection;
out vec3 fragColor;

vec3 u_skyColor = vec3(98, 131, 156)/255.0;
vec3 u_sunColor = vec3(0.098, 0.091, 0.024) * 0.6;
uniform vec3 u_waterColor = vec3(24, 85, 217) /255.f * 0.6;
float u_g = 0.75;
uniform vec3 u_sunPos;
uniform int u_underWater;


void main (void)
{

	vec3 skyColor = u_skyColor;
	vec3 sunColor = u_sunColor*0.5;

	vec3 localPos = normalize(v_vsViewDirection);
	vec3 lightPos = normalize(u_sunPos);

	vec3 upVector = vec3(0,1,0);
	float fCosEarth = max(dot(localPos, upVector),0);
	float foneMinusCosEarth = 1-fCosEarth;
	//float fCosSunEarth = 1-max(dot(lightPos, upVector), 0);
	float fCosSunEarth = 1-abs(dot(lightPos, upVector));

	float g2 = u_g * u_g;
	
	float fCosSun = dot(lightPos, localPos);
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCosSun*fCosSun) / pow(1.0 + g2 - 2.0*u_g*fCosSun, 1.5);
	float horizonIntensity = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCosSunEarth*fCosSunEarth) / pow(1.0 + g2 - 2.0*u_g*fCosSunEarth, 1.5);
	 
	vec3 computedSkyColor = skyColor
							+ fMiePhase * sunColor 
							+ (fCosSunEarth)*sunColor* 1 * pow(foneMinusCosEarth, 16)
							+ pow(foneMinusCosEarth, 16) * sunColor * 1
							;

	//u_groundColor = vec3(0.2,0.2,0.2);

	vec3 computedGroundColor = vec3(0.1,0.2,0.1);// + pow(foneMinusCosEarth, 16) * sunColor;

	//if(fCosEarth < 0.02)
	//{
	//	float a = min(max(fCosEarth/0.02,0), 1);
	//	a*=a*a;
	//	fragColor.rgb = mix(u_waterColor.rgb, computedSkyColor.rgb, vec3(a));	
	//}else
	{
		fragColor.rgb =  computedSkyColor;
	}

	if(u_underWater != 0)
	{
		fragColor.rgb = mix(u_waterColor.rgb, computedSkyColor.rgb, vec3(0.5));	
	}

	//fragColor.rgb =  firstColor + vec3(fCos);

}
