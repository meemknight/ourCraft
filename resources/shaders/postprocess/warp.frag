#version 330 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_color;
uniform float u_time;

uniform vec3 u_underwaterColor;
uniform sampler2D u_currentViewSpace;


//https://www.shadertoy.com/view/XttyRX
#define tau 6.28318530718

float sin01(float x) {
	return (sin(x*tau)+1.)/2.;
}


float u_underwaterDarkenDistance = 32;
float u_fogGradientUnderWater = 2;

float computeFogUnderWater(float dist)
{
	float rez = exp(-pow(dist*(1/u_underwaterDarkenDistance), u_fogGradientUnderWater));
	return pow(rez,4);
}

void main()
{
 
	float t = u_time * 1.0f;
	
	vec2 texCoords = v_texCoords;
	texCoords = texCoords * 2.f - 1.f;

	// Distort uv coordinates
	float amplitude = .010;
	float turbulence = 1.5;
	texCoords.xy += sin01(texCoords.x*turbulence + t) * amplitude;
	texCoords.xy -= sin01(texCoords.y*turbulence + t) * amplitude;

	texCoords = (texCoords + 1.f) /2.f;

	float dist = -texture2D(u_currentViewSpace, texCoords).z;
	

	if(dist <= 0.001)
	{
		outColor = vec4(u_underwaterColor, 1.f);
	}else
	{
		outColor = texture2D(u_color, texCoords);

		outColor.rgb = mix(outColor.rgb, u_underwaterColor, 0.1);

		//optional extra fog under water
		//outColor.rgb = mix(outColor.rgb, u_underwaterColor, (1-computeFogUnderWater(dist)) * 0.9);
 

	}





}

/*
	 // Get original texture coordinates
	vec2 texCoords = v_texCoords;

	// Distort texture coordinates using sine wave
	texCoords.y += sin(texCoords.x * 50.0 + u_time * 6) * 0.002; // Adjust frequency and amplitude as needed

	// Sample the color texture with the distorted coordinates
	outColor = texture2D(u_color, texCoords);
*/