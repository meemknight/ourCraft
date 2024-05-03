#version 330 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_color;
uniform float u_time;


//https://www.shadertoy.com/view/XttyRX
#define tau 6.28318530718

float sin01(float x) {
	return (sin(x*tau)+1.)/2.;
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

	outColor = texture2D(u_color, texCoords);

}

/*
	 // Get original texture coordinates
	vec2 texCoords = v_texCoords;

	// Distort texture coordinates using sine wave
	texCoords.y += sin(texCoords.x * 50.0 + u_time * 6) * 0.002; // Adjust frequency and amplitude as needed

	// Sample the color texture with the distorted coordinates
	outColor = texture2D(u_color, texCoords);
*/