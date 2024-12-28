#version 330 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_texture;
uniform vec2 u_center;

void main() {
	vec4 color = vec4(0.0);
	float totalWeight = 0.0;

	int numSamples = 200;
	float blurStrength = 0.6;
	float rayFrequency = 20.0; // Number of rays
	float rayAmplitude = 0.1; // Strength of the ray modulation

	// Direction from center to the current pixel
	vec2 direction = normalize(u_center - v_texCoords);

	// Calculate the angle of the direction
	float angle = atan(direction.y, direction.x); // Angle in radians

	// Blur along the direction of the light
	for (int i = 0; i < numSamples; ++i) 
	{
		float t = float(i) / float(numSamples);

		// Modulate based on the angle
		float rayModulation = 1.0 + rayAmplitude * sin(angle * rayFrequency);

		// Offset the texture coordinates
		vec2 offset = direction * t * blurStrength * rayModulation;

		// Sample the texture with offset
		color += texture(u_texture, v_texCoords + offset) * (1.0 - t); // Linear falloff
		totalWeight += (1.0 - t);

	
	}

	outColor = color / totalWeight; // Normalize result
}
