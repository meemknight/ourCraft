#version 430

layout(location = 0) out vec4 a_outColor;

in vec3 v_viewDirection;

layout(binding = 0) uniform samplerCube u_skybox;
layout(binding = 1) uniform sampler2D u_sunTexture;

uniform vec3 u_sunPos;

float remapFunction(float x)
{
	float a = 0.98;

	return (0.5/(1-a))*x  + (-0.5*a/(1-a));
}

vec2 mapDirectionToUV()
{

	vec3 view = normalize(v_viewDirection);	
	float sunDot = dot(u_sunPos, view);
	float sampleX = max(sunDot,0);
	sampleX	= remapFunction(sampleX);
	
	//todo
	//vec3 deltaVector = normalize(view - u_sunPos);
	//vec3 topVector = normalize(cross(deltaVector, view));	

	vec3 view2 = view;
	float sunDot2 = dot(u_sunPos, view2);
	float sampleY = max(sunDot2,0);
	sampleY	= remapFunction(sampleY);
	//sampleX -= 0.5;	
	
	//todo
	return vec2(sampleX, sampleY);
}

void main() 
{

	// Sample the skybox using the view direction
	vec4 skyColor = texture(u_skybox, normalize(v_viewDirection));

	// Calculate the direction from the fragment to the sun
	vec3 sunDirection = normalize(u_sunPos - gl_FragCoord.xyz);

	vec2 sunTexCoords = mapDirectionToUV();

	// Sample the sun texture using the corrected texture coordinates
	vec3 sunColor = texture(u_sunTexture, sunTexCoords).rgb;

	// Gamma correction
	skyColor.rgb = pow(skyColor.rgb, vec3(1.0 / 2.2));

	// Apply screen blend mode
	a_outColor.rgb = vec3(1.0) - (vec3(1.0) - skyColor.rgb) * (vec3(1.0) - sunColor);
	
	//a_outColor.rgb += mix(skyColor.rgb, sunColor.rgb, sunColor.a);

	// Ensure that the alpha channel is 1.0
	a_outColor.a = 1.0;
}