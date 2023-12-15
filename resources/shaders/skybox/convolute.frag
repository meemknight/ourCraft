#version 150 core
//https://learnopengl.com/PBR/IBL/Diffuse-irradiance

out vec4 fragColor;
in vec3 v_localPos;

uniform samplerCube u_environmentMap;

const float PI = 3.14159265359;
float u_spread = 2.0 * PI;
uniform float u_sampleQuality;

void main()
{		
	// the sample direction equals the hemisphere's orientation 
	vec3 normal = normalize(v_localPos);
  
	vec3 irradiance = vec3(0.0);
  
	
	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up         = normalize(cross(normal, right));
	
	float sampleDelta = u_sampleQuality; //0.025; degfault
	float nrSamples = 0.0; 
	for(float phi = 0.0; phi < u_spread; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 
	
			irradiance += texture(u_environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}

	irradiance *= PI / float(nrSamples);
  
	fragColor = vec4(irradiance, 1.0);
}