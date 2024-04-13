#version 330 core

//https://developer.download.nvidia.com/presentations/2008/SIGGRAPH/HBAO_SIG08b.pdf

layout(location = 0) out vec4 fragColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_gPosition;
uniform isampler2D u_gNormal;
uniform sampler2D u_texNoise;

uniform mat4 u_projection; // camera projection matrix
uniform mat4 u_view; // camera view matrix

const float INFINITY = 1.f/0.f;

vec3 fromuShortToFloat(ivec3 a)
{
	vec3 ret = a;

	//[0 65536] -> [0 1]
	ret /= 65536;

	//[0 1] -> [0 2]
	ret *= 2.f;

	//[0 2] -> [-1 1]
	ret -= 1.f;

	return normalize(ret);
}

float saturate(float a)
{
	return min(max(a,0),1);
}

vec2 computeAO(vec3 normal, vec2 direction, vec2 screenSize, vec3 fragPos)
{
	float RAD = 0.05;
	float RAD_FOR_DIRECTION = length( direction*vec2(10.0) / (vec2(abs(fragPos.z))*screenSize));

	vec3 viewVector = normalize(fragPos);

	vec3 leftDirection = cross(viewVector, vec3(direction,0));
	vec3 projectedNormal = normal - dot(leftDirection, normal) * leftDirection;
	float projectedLen = length(projectedNormal);
	projectedNormal /= projectedLen;

	vec3 tangent = cross(projectedNormal, leftDirection);


	//vec3 leftDirection = cross(normal, vec3(direction, 0));
	//vec3 tangent = normalize( cross(leftDirection, normal) );

	const float bias = (3.141592/360)*20.f;

	float tangentAngle = atan(tangent.z / length(tangent.xy));
	float sinTangentAngle = sin(tangentAngle + bias);
	
	//

	vec2 texelSize = vec2(1.f,1.f) / screenSize;

	float highestZ = -INFINITY;
	vec3 foundPos = vec3(0,0,-INFINITY);
	
	for(int i=2; i<=10; i++)
	{
		vec2 marchPosition = v_texCoords + i*texelSize*direction;
		
		vec3 fragPosMarch = texture(u_gPosition, marchPosition).xyz;
		
		vec3 hVector = normalize(fragPosMarch-fragPos); //inspre origine

		float rangeCheck = 1 - saturate(length(fragPosMarch-fragPos) / RAD-1);

		//hVector.z = mix(hVector.z, fragPos.z-RAD*2, rangeCheck);

		if(hVector.z > highestZ && length(fragPosMarch-fragPos) < RAD)
		{
			highestZ = hVector.z;
			foundPos = fragPosMarch;
		}
	}



	//float rangeCheck = smoothstep(0.0, 1.0, 10*length(screenSize) / length(foundPos - fragPos));
	//if(length(foundPos - fragPos) > length(screenSize)*10){rangeCheck=0;}


	//vec3 foundPos = vec3(0,0,-INFINITY);
	//float longest = -INFINITY;
	//for(int i=1; i<=10; i++)
	//{
	//	vec2 marchPosition = v_texCoords + i*texelSize*direction;
	//	
	//	vec3 fragPosMarch = texture(u_gPosition, marchPosition).xyz;
	//	
	//	//find distance to tangent
	//	vec3 fragPosNormal = normalize(fragPosMarch);
	//	float dist = length(tangent-fragPosNormal);
	//
	//	if(dist > longest && length(fragPosMarch-fragPos) < length(i*texelSize)*abs(fragPos.z)*2)
	//	{
	//		dist = longest;
	//		foundPos = fragPosMarch;
	//	}
	//}

	vec3 horizonVector = (foundPos - fragPos);
	float horizonAngle = atan(horizonVector.z/length(horizonVector.xy));
	
	float sinHorizonAngle = sin(horizonAngle);	

	vec2 rez = vec2(saturate((sinHorizonAngle - sinTangentAngle))/2, projectedLen);
	return rez;
}

void main()
{
	fragColor.a = 1;

	vec2 screenSize = textureSize(u_gPosition, 0).xy/2.f; //smaller rez
	vec2 noiseScale = vec2(screenSize.x/4.0, screenSize.y/4.0);
	vec2 noisePos = v_texCoords * noiseScale;

	vec3 fragPos   = texture(u_gPosition, v_texCoords).xyz; //view space

	if(fragPos.z == -INFINITY){fragColor.a = 1; return;}

	vec3 normal    = normalize( vec3( 
	transpose(inverse(mat3(u_view))) * 
		fromuShortToFloat(texture(u_gNormal, v_texCoords).xyz)
		));


	//vec2 randomVec = normalize(texture2D(u_texNoise, noisePos).xy); 
	vec2 randomVec = vec2(0,1);

	vec2 rez = vec2(0,0);

	vec3 viewVector = normalize(fragPos);


	rez += computeAO(normal, vec2(randomVec), screenSize, fragPos);
	rez += computeAO(normal, -vec2(randomVec), screenSize, fragPos);
	rez += computeAO(normal, vec2(-randomVec.y,randomVec.x), screenSize, fragPos);
	rez += computeAO(normal, vec2(randomVec.y, -randomVec.x), screenSize, fragPos);
	
	rez.x /= rez.y;
	
	fragColor.r = 1.f - rez.x;
}