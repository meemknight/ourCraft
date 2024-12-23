#version 430 core

layout (location = 0) out vec4 out_color;
noperspective in vec2 v_texCoords;

uniform sampler2D u_color;
uniform sampler2D u_positionViewSpace;
uniform sampler2D u_materials;
uniform isampler2D u_normals;
uniform mat4 u_cameraProjection;
uniform mat4 u_inverseView;
uniform mat4 u_view;
uniform mat4 u_inverseCameraViewProjection;

//////////////////////////////////////////////
//https://imanolfotia.com/blog/1
//https://github.com/ImanolFotia/Epsilon-Engine/blob/master/bin/Release/shaders/SSR.glsl
//SSR

vec2 reprojectViewSpace(vec2 currentTextureSpacePos);

//vec3 PositionFromDepth(float depth) {
//    float z = depth * 2.0 - 1.0;
//
//    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, z, 1.0);
//    vec4 viewSpacePosition = invprojection * clipSpacePosition;
//
//    // Perspective division
//    viewSpacePosition /= viewSpacePosition.w;
//
//    return viewSpacePosition.xyz;
//}

const float INFINITY = 1.f/0.f;
const float SSR_minRayStep = 1.0;
const int	SSR_maxSteps = 50;
const int	SSR_numBinarySearchSteps = 20;
const float SSR_maxRayStep = 150.2;
const float SSR_maxRayDelta = 10.0;

//old
//const float SSR_minRayStep = 1.0;
//const int	SSR_maxSteps = 150;
//const int	SSR_numBinarySearchSteps = 10;
//const float SSR_maxRayStep = 3.2;
//const float SSR_maxRayDelta = 3.0;

vec2 BinarySearch(inout vec3 dir, inout vec3 hitCoord, 
inout float dDepth, vec2 oldValue)
{
	float depth;

	vec4 projectedCoord;
	
	vec2 foundProjectedCoord = oldValue;

	for(int i = 0; i < SSR_numBinarySearchSteps; i++)
	{

		projectedCoord = u_cameraProjection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
		depth = textureLod(u_positionViewSpace, projectedCoord.xy, 2).z;
		//depth = texture(u_positionViewSpace, projectedCoord.xy).z;
 
		if(depth < -1000) //-INFINITY
			continue;

		foundProjectedCoord = projectedCoord.xy;

		dDepth = hitCoord.z - depth;

		dir *= 0.5;
		if(dDepth > 0.0)
			hitCoord += dir;
		else
			hitCoord -= dir;    
	}

		projectedCoord = u_cameraProjection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
		
		depth = texture(u_positionViewSpace, projectedCoord.xy).z;
	
	if(!(depth < -1000))
	{
		foundProjectedCoord = projectedCoord.xy;
	}

	return foundProjectedCoord.xy;
}

vec2 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth, vec3 worldNormal, vec3 viewDir)
{
	dir *= mix(SSR_minRayStep, SSR_maxRayStep, abs(dot(worldNormal, viewDir)));//maxRayStep;
 
	float depth;
	vec4 projectedCoord;
 
	for(int i = 0; i < SSR_maxSteps; i++)
	{
		hitCoord += dir;
 
		projectedCoord = u_cameraProjection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
		
		if(projectedCoord.x > 1.f || projectedCoord.y > 1.f
			||projectedCoord.x < -1.f || projectedCoord.y < -1.f
		)
		{
			break;
		}

		depth = textureLod(u_positionViewSpace,
			reprojectViewSpace(projectedCoord.xy),2).z;
		//depth = texture(u_positionViewSpace, projectedCoord.xy).z;

		if(depth > 1000.0)
			continue;
		
		if(depth < -1000) //-INFINITY
			continue;

		dDepth = hitCoord.z - depth;

		if((dir.z - dDepth) < SSR_maxRayStep && dDepth <= 0.0)
		{
			vec2 Result;
			Result = BinarySearch(dir, hitCoord, dDepth, projectedCoord.xy);
			//Result = projectedCoord.xy;

			if(dDepth < -SSR_maxRayDelta)
			{
				break; //fail //project to infinity :(((
			}
			
			depth = texture(u_positionViewSpace, reprojectViewSpace(Result.xy)).z;
			if(depth < -10000)
				{break;}//fail

			return Result;
		}
		
	}
 
	//signal fail	
	dDepth = -INFINITY;
	return vec2(0,0);
}


uvec3 murmurHash33(uvec3 src) {
	const uint M = 0x5bd1e995u;
	uvec3 h = uvec3(1190494759u, 2147483647u, 3559788179u);
	src *= M; src ^= src>>24u; src *= M;
	h *= M; h ^= src.x; h *= M; h ^= src.y; h *= M; h ^= src.z;
	h ^= h>>13u; h *= M; h ^= h>>15u;
	return h;
}

// 3 outputs, 3 inputs
vec3 hash33(vec3 src) {
	uvec3 h = murmurHash33(floatBitsToUint(src));
	return uintBitsToFloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
}

vec3 computeJitt(vec3 wp, vec2 Resolution, vec3 viewNormal, float Roughness)
{
	vec2 NoiseScale = Resolution / 4.0;
	//vec3 random = hash33(wp + iTime);//vec3(texture(noiseTexture, (TexCoords.xy*10.0) + (1.0 - iTime)).rgb);
	vec3 random = hash33(wp);//vec3(texture(noiseTexture, (TexCoords.xy*10.0) + (1.0 - iTime)).rgb);
	random = dot(random, viewNormal) > 0.0 ? random : -random;
	float factor = Roughness*0.20;
	vec3 hs = random * 2.0 - 1.0;
	vec3 jitt = hs * factor;
	return vec3(jitt);
}

vec3 SSR(out bool success, vec3 viewPos, vec3 N, 
	out float mixFactor, float roughness, vec3 wp, vec3 viewDir, vec3 viewSpaceNormal, vec2 rezolution)
{
	//return vec3(0,0,0);

	mixFactor = 0;
	success = false;

	// Reflection vector
	vec3 reflected = normalize(reflect(normalize(viewPos), viewSpaceNormal));
	//vec3 reflected = R; //todo check
	
	//found = true;
	//return viewPos;

	if(reflected.z > 0){return vec3(0,0,0);}

	vec3 hitPos = viewPos;
	float dDepth;

	//vec3 wp = vec3(vec4(viewPos, 1.0) * invView);
	//vec3 jitt = mix(vec3(0.0), vec3(hash(wp)), spec);
	
	//todo test
	vec3 jitt = computeJitt(wp, rezolution, viewSpaceNormal, roughness); //use roughness for specular factor
	//vec3 jitt = vec3(0.0);

	vec2 coords = RayMarch( normalize((vec3(jitt) + reflected) *
		max(SSR_minRayStep, -viewPos.z)), hitPos, dDepth,
	N, viewDir);
	
	if(dDepth < -1000){return vec3(0);}


	vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
 
	float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

	float ReflectionMultiplier = 
			screenEdgefactor * 
			-reflected.z;
 
	if(ReflectionMultiplier <= 0.001)
	{
		return vec3(0.f);
	}

	// Get color
	vec3 lastFrameColor = textureLod(u_color, reprojectViewSpace(coords.xy), 0).rgb;
	//vec3 SSR = lastFrameColor * clamp(ReflectionMultiplier, 0.0, 0.9) * F;  
	vec3 SSR = lastFrameColor;

	mixFactor = clamp(ReflectionMultiplier, 0.0, 1.f);  

	success = true;
	return SSR;
}

////////////////////////////////////////////

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}   

//https://www.reddit.com/r/GraphicsProgramming/comments/102sdnh/how_would_i_calculate_the_viewdirection_without/
vec3 getViewVector(vec2 fragCoord)
{
	float depth     = 1;
	vec4  unproject = u_inverseCameraViewProjection
	* vec4(fragCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0); // Transform the UV and depth map sampled value into [-1,1] range
	vec3  worldPos  = unproject.xyz / unproject.w;
	
	return -normalize(worldPos);
}

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

void main()
{

	bool ssrSuccess = false;
	vec2 fragCoord = gl_FragCoord.xy / textureSize(u_positionViewSpace, 0).xy;

	float roughness = texture(u_materials, fragCoord).r;

	vec3 ssr = vec3(0,0,0);
	vec3 V = vec3(0,0,0);
	vec3 N = vec3(0,0,0);
	if(roughness < 0.45)
	{
		vec3 V = getViewVector(fragCoord);

		N = fromuShortToFloat(texture(u_normals, fragCoord).xyz);

		//float dotNVClamped = clamp(dotNV, 0.0, 0.99);
		
		vec3 posViewSpace = texture(u_positionViewSpace, 
			(fragCoord)).xyz;
		vec3 pos = vec3(u_inverseView * vec4(posViewSpace,1));

		vec3 viewSpaceNormal = normalize( vec3(transpose(inverse(mat3(u_view))) * N));

		vec3 ssrNormal = N;
		//if(isWater()){ssrNormal = v_normal;}

		float mixFactor = 0;
		//vec3 ssr = SSR(posViewSpace, N, metallic, F, mixFactor, roughness, pos, V, viewSpaceNormal, 
		//	textureSize(u_positionViewSpace, 0).xy);
		ssr = SSR(ssrSuccess, posViewSpace, ssrNormal, mixFactor, pow(roughness,2), pos, V, viewSpaceNormal, 
			textureSize(u_positionViewSpace, 0).xy);
		
		//if is water just reflect 100% because we deal with it later
		//if(isWater())
		//{
		//	if(ssrSuccess) 
		//	{
		//		//out_color.rgb = mix(u_waterColor, ssr, 0.9);
		//		out_color.rgb = ssr;
		//	}
		//	//out_color.a = 1;
		//}else
		//{
		//	if(ssrSuccess) 
		//	{
		//		//ssr = mix(ssr, out_color.rgb, dotNV);
		//		ssr = mix(ssr, out_color.rgb, freshnel);
		//		out_color.rgb = mix(ssr, out_color.rgb, pow(roughness, 0.5));
		//	}	
		//
		//	//if(success) {out_color.rgb = vec3(0,0,1);}else
		//	//{out_color.rgb = vec3(1,0,0);}
		//
		//	//out_color.rgb *= posViewSpace;
		//}
		//if(success)out_color.rgb = ssr;
			
	}else
	{
		out_color.rgba = vec4(0,0,0,0);
		return;
	}

	if(ssrSuccess)
	{
		vec3 F0 = vec3(0.04); 
		//F0 = mix(F0, textureColor.rgb, vec3(metallic)); //todo?

		float dotNV = dot(N, V);
		vec3 freshnel = fresnelSchlickRoughness(dotNV, F0, roughness);

		//ssr = mix(ssr, out_color.rgb, freshnel);
		//todo use pow(roughness, 0.5)?

		//out_color.rgba = vec4(ssr, pow(roughness, 0.5));

		out_color.rgba = vec4(ssr, freshnel * pow(roughness, 0.5));
		//out_color.rgba = vec4(1,0,0,1);

	
	}else
	{
		out_color.rgba = vec4(0,0,0,0);
	}



}


vec2 reprojectViewSpace(vec2 currentTextureSpacePos)
{
	// Convert current fragment's screen space coordinate to clip space
	//vec4 clipCoord = vec4(currentTextureSpacePos * 2.0 - 1.0, 0.0, 1.0);
	//
	//// Convert clip space coordinate to view space using the inverse of the current view projection matrix
	//vec4 posViewSpace = u_inverseViewProjMat * clipCoord;
	//posViewSpace /= posViewSpace.w; // Homogeneous divide to normalize
	//
	//posViewSpace = u_lastViewProj * posViewSpace;
	////posViewSpace = u_cameraProjection * u_view * posViewSpace;
	//
	//posViewSpace /= posViewSpace.w;
	//
	//return (posViewSpace.xy + 1) / 2;

	return currentTextureSpacePos;
}
