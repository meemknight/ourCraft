#version 430

layout(location = 0) out vec4 a_outColor;

in vec3 v_viewDirection;

layout(binding = 0) uniform samplerCube u_skybox1;
layout(binding = 1) uniform samplerCube u_skybox2;
layout(binding = 2) uniform sampler2D u_sunTexture;

uniform vec3 u_sunPos;
uniform float u_blend;

uniform float u_rotation1;
uniform float u_rotation2;


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

vec3 toLinearSRGB(vec3 sRGB)
{
	bvec3 cutoff = lessThan(sRGB, vec3(0.04045));
	vec3 higher = pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4));
	vec3 lower = sRGB/vec3(12.92);
	return mix(higher, lower, cutoff);
}

float toLinearSRGB(float sRGB)
{
	bool cutoff = sRGB < float(0.04045);
	float higher = pow((sRGB + float(0.055))/float(1.055), float(2.4));
	float lower = sRGB/float(12.92);
	return mix(higher, lower, cutoff);
}


float toLinear(float a)
{

	return toLinearSRGB(a);
}

vec3 toLinear(vec3 a)
{
	return toLinearSRGB(a);
}

//https://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 rotationMatrix(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	
	return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
				oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
				oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
				0.0,                                0.0,                                0.0,                                1.0);
}

void main() 
{

	// Sample the skybox using the view direction
	vec3 skyColor1 = texture(u_skybox1, normalize( ( rotationMatrix(vec3(0,1,0), u_rotation1) * vec4(v_viewDirection,1) ).rgb )   ).rgb;
	vec3 skyColor2 = texture(u_skybox2, normalize( ( rotationMatrix(vec3(0,1,0), u_rotation2) * vec4(v_viewDirection,1) ).rgb )   ).rgb;

	vec3 skyColor = mix(skyColor1, skyColor2, u_blend);

	// Calculate the direction from the fragment to the sun
	vec3 sunDirection = normalize(u_sunPos - gl_FragCoord.xyz);

	vec2 sunTexCoords = mapDirectionToUV();

	// Sample the sun texture using the corrected texture coordinates
	vec3 sunColor = texture(u_sunTexture, sunTexCoords).rgb;


	skyColor.rgb *= 1.0f; //brighten the sky
	sunColor.rgb *= 1.0f; //brighten the sky


	// Apply screen blend mode
	a_outColor.rgb = vec3(1.0) - (vec3(1.0) - skyColor.rgb) * (vec3(1.0) - sunColor);
	
	//a_outColor.rgb += mix(skyColor.rgb, sunColor.rgb, sunColor.a);

	// Ensure that the alpha channel is 1.0
	a_outColor.a = 1.0;
}