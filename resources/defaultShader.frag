#version 430 core
#extension GL_ARB_bindless_texture: require

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_screenSpacePositions;
layout (location = 2) out ivec3 out_normals;


layout(binding = 0) uniform sampler2D u_numbers;

in vec2 v_uv;
in float v_ambient;
in vec3 v_semiViewSpacePos; //world space view position and view only with transpation

in flat uvec2 v_textureSampler;
in flat uvec2 v_normalSampler;
in flat uvec2 v_materialSampler;
in flat int v_ambientInt;

in flat int v_skyLight; //ambient sun value
in flat int v_skyLightUnchanged;
in flat int v_normalLight;


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
	float u_fogCloseGradient;
	int u_shadows;

};


uniform vec3 u_pointPosF;
uniform ivec3 u_pointPosI;
uniform vec3 u_sunDirection;


uniform int u_underWater;
uniform int u_showLightLevels;

in vec4 v_fragPos;
in vec4 v_fragPosLightSpace;
in flat ivec3 v_blockPos;

uniform sampler2D u_sunShadowTexture;

uniform sampler2D u_brdf;

uniform vec3 u_sunLightColor;
uniform vec3 u_ambientColor;


uniform sampler2D u_depthTexture;
uniform int u_depthPeelwaterPass = 0;
uniform int u_hasPeelInformation = 0;
uniform sampler2D u_PeelTexture;
uniform sampler2D u_dudv;
uniform sampler2D u_dudvNormal;
uniform sampler2D u_skyTexture;
uniform sampler2D u_ao;

uniform float u_waterMove;
uniform sampler2D u_caustics;

uniform mat4 u_inverseProjMat;
uniform mat4 u_inverseViewProjMat;

uniform mat4 u_cameraProjection;
uniform mat4 u_inverseView;
uniform mat4 u_view;

uniform mat4 u_lastViewProj;


in flat int v_flags; //todo no need to have a flag to specify if the block is water, just use the texture id or something

uniform sampler2D u_lastFrameColor;
uniform sampler2D u_lastFramePositionViewSpace;

///
const float pointLightColor = 2.f;
const float atenuationFactor = 0.5f;
const float causticsTextureScale = 3.f;
const float causticsChromaticAberationStrength = 0.004;	
const float waterSpeed = 15.f;
const float causticsLightStrength = 1.4;	
const float causticsLightPower = 1.0;	
const bool physicallyAccurateReflections = false;
///

ivec3 fromFloatTouShort(vec3 a)
{
	//[-1 1] -> [0 2]
	a += 1.f;

	//[0 2] -> [0 1]
	a /= 2.f;

	//[0 1] -> [0 65536]
	a *= 65536;

	return ivec3(a);
}


bool isWater()
{
	return ((v_flags & 1) != 0);
}


float computeFog(float dist)
{
	float rezClose = 1;

	if(u_fogCloseGradient!=0)
	{
		rezClose = exp(-pow(dist*(1.f/64), u_fogCloseGradient));
		if(rezClose > 0.95){rezClose = 1;}
		rezClose = rezClose / 4.f;
		rezClose = rezClose + 0.75f;
	}


	float rez = exp(-pow(dist*(1.f/u_fogDistance), 64));
	if(rez > 0.8){rez = pow(rez,0.5f);}
	return pow(rez,2) * rezClose;
}

float computeFogUnderWater(float dist)
{
	float rez = exp(-pow(dist*(1/u_underwaterDarkenDistance), u_fogGradientUnderWater));
	return pow(rez,4);
}

readonly restrict layout(std430) buffer u_lights
{
	ivec4 lights[];
};

uniform int u_lightsCount;

uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;

uniform float u_near;
uniform float u_far;

in flat ivec3 fragmentPositionI;
in vec3 fragmentPositionF;
in flat vec3 v_normal;

const float PI = 3.14159265359;

vec3 compute(ivec3 destI, vec3 destF, ivec3 sourceI, vec3 sourceF)
{
	ivec3 intPart = destI - sourceI;
	vec3 floatPart = destF - sourceF;

	return floatPart + vec3(intPart);
}

float atenuationFunction(float dist)
{
	float maxDist = 15;

	//if(dist >= maxDist)return 0;
	//return 1;

	return max(0, (maxDist-dist)/(dist*dist*atenuationFactor+maxDist));
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
	//if(u_tonemapper == 0)
	//{
	//	//return pow(a, float(2.2));
	//	return toLinearSRGB(a);
	//}else if(u_tonemapper == 1)
	//{
	//	return toLinearAXG(a);
	//}else if(u_tonemapper == 2)
	//{
	//	return toLinearSRGB(a);
	//}
	return toLinearSRGB(a);
}

vec3 toLinear(vec3 a)
{
	return toLinearSRGB(a);
}


//https://gamedev.stackexchange.com/questions/22204/from-normal-to-rotation-matrix#:~:text=Therefore%2C%20if%20you%20want%20to,the%20first%20and%20second%20columns.
mat3x3 NormalToRotation(in vec3 normal)
{
	// Find a vector in the plane
	vec3 tangent0 = cross(normal, vec3(1, 0, 0));
	if (dot(tangent0, tangent0) < 0.001)
		tangent0 = cross(normal, vec3(0, 1, 0));
	tangent0 = normalize(tangent0);
	// Find another vector in the plane
	vec3 tangent1 = normalize(cross(normal, tangent0));
	// Construct a 3x3 matrix by storing three vectors in the columns of the matrix

	return mat3x3(tangent0,tangent1,normal);

	//return ColumnVectorsToMatrix(tangent0, tangent1, normal);
}


vec2 getDudvCoordsStatic()
{
	vec2 samplingPoint = - v_blockPos.zx + vec2(v_uv.x, v_uv.y);
	return samplingPoint * (1/20.f);
}

vec2 getDudvCoords(float speed)
{
	vec2 samplingPoint = - v_blockPos.zx + vec2(v_uv.x + u_waterMove*speed, v_uv.y + u_waterMove*speed/2);
	return samplingPoint * (1/20.f);
}

vec2 getDudvCoords2(float speed)
{
	vec2 samplingPoint = - v_blockPos.zx + vec2(v_uv.x + u_waterMove*speed + 3, v_uv.y + u_waterMove*speed/2 + 5);
	samplingPoint.y = -samplingPoint.y;
	return samplingPoint * 0.1;
}

vec2 getDudvCoords3(float speed)
{
	vec2 samplingPoint = - v_blockPos.zx + vec2(v_uv.x - u_waterMove*speed, v_uv.y - u_waterMove*speed/3);
	samplingPoint.y = -samplingPoint.y;
	return samplingPoint * 0.1;
}

vec2 getDudvCoords4(float speed)
{
	vec2 samplingPoint = - v_blockPos.zx + vec2(v_uv.x + u_waterMove*speed, v_uv.y - u_waterMove*speed*2);
	samplingPoint.y = -samplingPoint.y;
	return samplingPoint * 0.1;
}

vec2 getDudvCoords5(float speed)
{
	vec2 samplingPoint = - v_blockPos.zx + vec2(v_uv.x + u_waterMove*speed, v_uv.y + u_waterMove*speed/2);
	return samplingPoint * (1/50.f);
}

vec2 getDudvCoords6(float speed)
{
	vec2 samplingPoint = - v_blockPos.zx + vec2(v_uv.x + u_waterMove*speed, v_uv.y - u_waterMove*speed*2);
	samplingPoint.y = -samplingPoint.y;
	return samplingPoint * (1/100.f);
}



float linearizeDepth(float d)
{
	return 2 * u_near * u_far / (u_far + u_near - (2*d-1.f) * (u_far - u_near) );
}

float getLastDepthLiniarized(vec2 p, out float nonLinear)
{
	float lastDepth = texture(u_depthTexture, p).x;
	nonLinear = lastDepth;
	float linear = linearizeDepth(lastDepth);
	return linear;	
}

vec3 applyNormalMap(vec3 inNormal)
{
	vec3 normal;
	if( isWater() && (u_shaders!=0) )
	{
		vec2 firstDudv = texture(u_dudvNormal, getDudvCoords3(waterSpeed/3.f)).rg;
		normal = texture(u_dudvNormal, getDudvCoords5(waterSpeed*2)+firstDudv*0.04 ).rgb;

		//normal = texture(u_dudvNormal, getDudvCoords(waterSpeed)).rgb*1;
		////normal += texture(u_dudvNormal, getDudvCoords2(10)).rgb;
		//normal += texture(u_dudvNormal, getDudvCoords3(waterSpeed)).rgb*0.5;

		//normal = normalize(normal);

		//normal = (vec3(0.5,0.5,1) + (vec3(normal.x,0,normal.z) - vec3(0.5,0,0.5)) * 0.5);
		normal = (vec3(0.5,0.5,1) + (vec3(normal.x,normal.y,0) - vec3(0.5,0.5,0)) * 0.2);
		//normal = vec3(normal);

		//return normalize(vec3(0.0,1,0));

		//normal = normalize(mix(normal, vec3(0.5,0.5,1), 0.9));
		//normal = vec3(0.5,0.5,1);
		//normal = normalize(2*normal - 1.f);
		//return normal;
		//return inNormal;
	}else
	{
		normal = texture(sampler2D(v_normalSampler), v_uv).rgb;
	}

	normal = normalize(2*normal - 1.f);
	mat3 rotMat = NormalToRotation(inNormal);
	normal = rotMat * normal;
	normal = normalize(normal);

	if(!gl_FrontFacing){normal = -normal;};

	return normal;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	//float r = (roughness + 1.0);
	//float k = (r*r) / 8.0;			//disney

	float k = roughness*roughness / 2;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / max(denom, 0.0000001);
}

//oclude light that is hidded begind small geometry roughnesses
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

//n normal
//h halfway vector
//a roughness	(1 rough, 0 glossy) 
//this gets the amount of specular light reflected
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	//GGX/Trowbridge-Reitz
	//			 a^2
	// ------------------------
	// PI ((N*H)^2 (a^2-1)+1)^2

	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return  a2 / max(denom, 0.0000001);
}

//cosTheta is the dot between the normal and halfway
//ratio between specular and diffuse reflection
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
float fresnelSchlickWater(float cosTheta)
{
	float F0 = float(0.02);
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}   

vec3 fSpecular(vec3 normal, vec3 halfwayVec, vec3 viewDir, 
vec3 lightDirection, float dotNVclamped, float roughness, vec3 F)
{
	//fCook-Torrance
	float NDF = DistributionGGX(normal, halfwayVec, roughness);       
	float G   = GeometrySmith(normal, viewDir, lightDirection, roughness);   
	float denominator = 4.0 * dotNVclamped  
		* max(dot(normal, lightDirection), 0.0);
	vec3 specular     = (NDF * G * F) / max(denominator, 0.001);

	return specular;
}

//https://mimosa-pudica.net/improved-oren-nayar.html
vec3 fDiffuseOrenNayar2(vec3 color, float roughness, vec3 L, vec3 V, vec3 N)
{
	float a = roughness;
	float a2 = a*a;
	//vec3 A = 1.f/PI * (1 - 0.5 * a2/(a2 + 0.33) + 0.17*color*a2/(a2+0.13));
	//float B = 0.45*a2/(a2+0.09);

	float A = 1.0/(PI+(PI/2.0-2/3.0)*a);
	float B = PI/(PI+(PI/2.0-2/3.0)*a);

	float s = dot(L,N) - dot(N,L)*dot(N,V);

	float t;
	if(s <= 0)
		t = 1;
	else
		t = max(dot(N,L), dot(N,V));

	return color * (A + B * s/t);
}


//https://github.com/meemknight/gl3d
vec3 computePointLightSource(vec3 lightDirection, float metallic, float roughness, in vec3 lightColor, 
	in vec3 viewDir, in vec3 color, in vec3 normal, in vec3 F0)
{

	float dotNVclamped = clamp(dot(normal, viewDir), 0.0, 0.99);

	vec3 halfwayVec = normalize(lightDirection + viewDir);
	
	vec3 radiance = lightColor; //here the first component is the light color
	
	vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);

	vec3 specular = fSpecular(normal, halfwayVec, viewDir, lightDirection, dotNVclamped, roughness, F);

	vec3 kS = F; //this is the specular contribution
	vec3 kD = vec3(1.0) - kS; //the difuse is the remaining specular
	kD *= 1.0 - metallic;	//metallic surfaces are darker
	
	//vec3 diffuse = fDiffuse(color.rgb);
	//vec3 diffuse = fDiffuseOrenNayar(color.rgb, roughness, lightDirection, viewDir, normal);
	vec3 diffuse = fDiffuseOrenNayar2(color.rgb, roughness, lightDirection, viewDir, normal);
	
	float NdotL = max(dot(normal, lightDirection), 0.0);        
	return (kD * diffuse + specular) * radiance * NdotL;

}


float computeLight(vec3 N, vec3 L, vec3 V)
{
	vec3 H = normalize(L + V);
			
	float shininess = 16;
	float spec = pow(max(dot(N, H), 0.0), shininess);
	float diffuse = max(dot(L, N), 0.0); 	
	
	return spec + diffuse;
}


uniform mat4 u_lightSpaceMatrix;
uniform ivec3 u_lightPos;

//pointPos is in worls space
float testShadow(vec3 pointPos, float dotLightNormal)
{
	vec4 fragPosLightSpace = 
		u_lightSpaceMatrix * vec4(pointPos, 1);

	vec3 pos = fragPosLightSpace.xyz * 0.5 + 0.5;
	pos.z = min(pos.z, 1.0);
	
	float depth = texture(u_sunShadowTexture, pos.xy).r;
		
	float bias = max(0.001, 0.05 * (1.f - dotLightNormal));

	return (depth+bias) < pos.z ? 0.0 : 1.0;	
	
}

float shadowCalc(float dotLightNormal)
{
	vec3 pos = v_fragPosLightSpace.xyz * 0.5 + 0.5;
	pos.z = min(pos.z, 1.0);
	
	float depth = texture(u_sunShadowTexture, pos.xy).r;
		
	float bias = max(0.001, 0.05 * (1.f - dotLightNormal));

	return (depth+bias) < pos.z ? 0.0 : 1.0;	
	//return (depth) != 1.f ? 0.0 : 1.0;	
}

//https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/
float InterleavedGradientNoise(vec2 position_screen)
{
  vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
  return fract(magic.z * fract(dot(position_screen, magic.xy)));
}

//https://developer.download.nvidia.com/cg/sincos.html
void sincos(float a, out float s, out float c)
{
	s = sin(a);
	c = cos(a);
}

//https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/
vec2 vogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
  float GoldenAngle = 2.4f;

  float r = sqrt(sampleIndex + 0.5f) / sqrt(samplesCount);
  float theta = sampleIndex * GoldenAngle + phi;

  float sine, cosine;
  sincos(theta, sine, cosine);
  
  return vec2(r * cosine, r * sine);
}

float testShadowValue(float dotLightNormal, vec3 pos)
{
	//float closestDepth = texture(map, coords).r; 
	//return  (currentDepth - bias) < closestDepth  ? 1.0 : 0.0;
	float depth = texture(u_sunShadowTexture, pos.xy).r;
	float bias = max(0.001, 0.05 * (1.f - dotLightNormal));
	return (depth+bias) < pos.z ? 0.0 : 1.0;
}

float getShadowDistance(vec3 pos)
{
	float depth = texture(u_sunShadowTexture, pos.xy).r;
	return max(depth - pos.z, 0.f);
}

float shadowCalc2(float dotLightNormal)
{
	
	if(u_shadows == 0){ return 1.f; }

	if(u_shadows == 1){return shadowCalc(dotLightNormal);}

	vec3 projCoords = v_fragPosLightSpace.xyz * 0.5 + 0.5;
	projCoords.z = min(projCoords.z, 1.0);

	// keep the shadow at 1.0 when outside or close to the far_plane region of the light's frustum.
	if(projCoords.z > 0.99995)
		return 1.f;
	//if(projCoords.z < 0)
	//	return 1.f;

	//float shadowDistance = getShadowDistance(projCoords);
	//shadowDistance /= 10;
	//shadowDistance = min(shadowDistance, 1);

	//float closestDepth = texture(shadowMap, projCoords.xy).r; 
	float currentDepth = projCoords.z;

	//todo move
	vec2 texelSize = 1.0 / textureSize(u_sunShadowTexture, 0).xy;
	float shadow = 0.0;

	bool fewSamples = false;
	int kernelHalf = 1;
	int kernelSize = kernelHalf*2 + 1;
	int kernelSize2 = kernelSize*kernelSize;

	//float receiverDepth = currentDepth;
	//float averageBlockerDepth = texture(sampler2DArray(shadowMap), vec3(projCoords.xy, index)).r;
	//float penumbraSize = 4.f * (receiverDepth - averageBlockerDepth) / averageBlockerDepth;
	float penumbraSize = 1.f;

	//standard implementation
	{
	
		int sampleSize = 9;
		int checkSampleSize = 5;
		float size = 1.5;

		float noise = InterleavedGradientNoise(gl_FragCoord.xy) * 2 * PI;

		for(int i=sampleSize-1; i>=sampleSize-checkSampleSize; i--)
		{
			vec2 offset = vogelDiskSample(i, sampleSize, noise);
			vec2 finalOffset = offset * texelSize * size;
			
			float s = testShadowValue(dotLightNormal, projCoords + vec3(finalOffset,0));
			shadow += s;
		}

		//optimization
		if(true && (shadow == 0 || shadow == checkSampleSize))
		{
			shadow /= checkSampleSize;
		}else
		{
			for(int i=sampleSize-checkSampleSize-1; i>=0; i--)
			{
				vec2 offset = vogelDiskSample(i, sampleSize, noise);
				vec2 finalOffset = offset * texelSize * size;
				
				float s = testShadowValue(dotLightNormal, projCoords + vec3(finalOffset,0));
				shadow += s;
			}

			shadow /= sampleSize;
		}

	}
	
	
	//float shadowPower = mix(32,1.f,shadowDistance);
	float shadowPower = 2;

	return pow(clamp(shadow, 0, 1), shadowPower);
}

vec3 SSR(out bool success,vec3 viewPos, vec3 N, 
	out float mixFactor, float roughness, vec3 wp, vec3 viewDir, vec3 viewSpaceNormal, vec2 rezolution);

vec3 computeAmbientTerm(vec3 gammaAmbient, vec3 N, vec3 V, vec3 F0, float roughness, 
	float metallic, vec3 albedo, out bool ssrSuccess, vec3 out_color);

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


//position view space -> proj matrix -> perspective divide -> ndc

float luminosity(vec3 a)
{
	return dot(a, vec3(0.21,0.72,0.07));
}

vec2 rotate45(vec2 a)
{
	a -= vec2(0.5);
	a = vec2((a.x+a.y), (a.y-a.x))/sqrt(2.f);
	a += vec2(0.5);
	return a;
}

vec2 rotate90TextureCoord(vec2 a)
{
	a -= vec2(0.5);
	a = vec2(-a.y, a.x);
	a += vec2(0.5);
	return a;
}

vec2 rotate180TextureCoord(vec2 a)
{
	a -= vec2(0.5);
	a = -vec2(a.x, a.y);
	a += vec2(0.5);
	return a;
}

vec2 rotateMinus90TextureCoord(vec2 a)
{
	a -= vec2(0.5);
	a = vec2(a.y, -a.x);
	a += vec2(0.5);
	return a;
}

vec2 flipTextureCoord(vec2 a)
{
	a -= vec2(0.5);
	a = vec2(a.y, a.x);
	a += vec2(0.5);
	return a;
}


float getBlockAO()
{
	
	//determine shape:
	//none front back left right, corner1, 2, 3, 4, full

	int pos = (v_flags & 0xF0) >> 4;

	if(pos == 0){return 1;}

	vec2 cornerDarken1 = - vec2(0.5, (sqrt(2)-1.f)/2.f) * 1.5;

	vec2 uv = vec2(v_uv.x,v_uv.y);

	vec2 rotated1 = rotate45(uv) + cornerDarken1;
	vec2 rotated2 = rotate45(rotate90TextureCoord(uv)) + cornerDarken1;
	vec2 rotated3 = rotate45(rotateMinus90TextureCoord(uv)) + cornerDarken1;
	vec2 rotated4 = rotate45(flipTextureCoord(uv)) + cornerDarken1;

	vec2 cornerDarken =  - vec2(0.5, (sqrt(2)-1.f)/2.f)*3;

	vec2 rez[14] = { //front back left right
		vec2(rotate90TextureCoord(uv)), rotateMinus90TextureCoord(uv), rotate180TextureCoord(uv), vec2(uv.x, uv.y),
		vec2(rotated4.x, 1-rotated4.y), vec2(rotated1.x, 1-rotated1.y),  vec2(rotated3.x, 1-rotated3.y), vec2(rotated2.x, 1-rotated2.y),
		vec2(rotated4.x, 1-rotated4.y) + cornerDarken, vec2(rotated1.x, 1-rotated1.y)+ cornerDarken,  vec2(rotated3.x, 1-rotated3.y)+ cornerDarken, vec2(rotated2.x, 1-rotated2.y)+ cornerDarken,
		vec2(0.5, 0.0),
		vec2(0.5, 0.4),
		};

	vec2 newUV = rez[pos - 1]; 
	
	float ao = texture2D(u_ao, newUV).r;
	return clamp(pow(ao, 1.2) * 1.1f, 0.6f,1);
}



void main()
{

	//depth peel for underwater stuff
	if(u_depthPeelwaterPass != 0)
	{
		vec2 p = v_fragPos.xy / v_fragPos.w;
		p += 1;
		p/=2;

		if (gl_FragCoord.z <= texture(u_depthTexture, p).x + 0.0000001) 
			discard; //Manually performing the GL_GREATER depth test for each pixel

	}

	const bool blockIsInWater = ((v_flags & 2) != 0);
	const float baseAmbient = 0.25;
	const float multiplier = 0.6;
	vec3 computedAmbient = vec3(toLinear(v_ambient *  multiplier * (1.f-baseAmbient) + baseAmbient));
	if(blockIsInWater)
	{
		computedAmbient *= vec3(0.38,0.4,0.42);
	}

	computedAmbient *= u_ambientColor;

	float blockAO = getBlockAO();
	computedAmbient *= blockAO;

	if(u_shaders == 0)
	{
		//load albedo
		vec4 textureColor = texture(sampler2D(v_textureSampler), v_uv);
		if(textureColor.a <= 0){discard;}
		//gamma correction
		textureColor.rgb = toLinear(textureColor.rgb);

		vec3 N = applyNormalMap(v_normal);

		float viewLength = length(v_semiViewSpacePos);
		vec3 V = -v_semiViewSpacePos / viewLength;

		vec3 finalColor = computedAmbient;
		for(int i=0; i< u_lightsCount; i++)
		{
			vec3 L = compute(lights[i].rgb, vec3(0), fragmentPositionI, fragmentPositionF);
			//vec3 L =compute(u_pointPosI, u_pointPosF, fragmentPositionI, fragmentPositionF);
			
			float menhetanDistance = dot((abs(fragmentPositionI-lights[i].rgb)),vec3(1));
			//float menhetanDistance = dot((abs(L)),vec3(1));

			float LightDist = length(L);
			if((15-menhetanDistance) + 0.1 > v_normalLight){continue;}
			L = normalize(L);		
			
			finalColor += clamp(dot(L, N), 0, 1) 
				* atenuationFunction(LightDist) * pointLightColor;
				
		}

		if(v_skyLightUnchanged > 3)
		{
			vec3 sunLightColor = u_sunLightColor;
			sunLightColor *= 1-((15-v_skyLightUnchanged)/9.f);
			sunLightColor *= ((v_ambientInt/15.f)*0.6 + 0.4f);

			vec3 L = u_sunDirection;

			if(dot(u_sunDirection, vec3(0,1,0))> -0.2)
			{
				float shadow = shadowCalc2(dot(L, v_normal));
				
				finalColor += clamp(dot(N, L), 0, 1)
					* sunLightColor * shadow;
			}
		}
		
		out_color = vec4(finalColor,textureColor.a);
		out_color.a = 1-out_color.a;
		out_color.a *= clamp(0,1,dot(N,V));
		out_color.a = 1-out_color.a;

		out_color.rgb = textureColor.rgb * finalColor;
		
		if(u_showLightLevels != 0)
		{
			vec4 numbersColor = texture(u_numbers, vec2(v_uv.x / 16.0 + (1/16.0)*v_ambientInt, v_uv.y));
			if(numbersColor.a > 0.1)
			{out_color.rgb = mix(toLinear(numbersColor.rgb), out_color.rgb, 0.5);}
		}
		

		if(u_underWater != 0)
		{
			out_color.rgb = mix(u_underWaterColor.rgb, out_color.rgb, 
								vec3(computeFogUnderWater(viewLength)) * u_underwaterDarkenStrength 
								+ (1-u_underwaterDarkenStrength)
							);
		}

		//out_color.rgb *= u_exposure;
		//
		////vec3 purkine = purkineShift(out_color.rgb);
		////out_color.rgb = mix(out_color.rgb, purkine, 0.05);
		//
		//out_color.rgb = tonemapFunction(out_color.rgb);
		//out_color.rgb = toGammaSpace(out_color.rgb);
		//
		//out_color = clamp(out_color, vec4(0), vec4(1));
	
	}else
	{

		//load material
		float metallic = 0;
		float roughness = 0;
		{
			vec2 materialColor = texture(sampler2D(v_materialSampler), v_uv).rg;
			
			roughness = pow((1 - materialColor.r),2);
			metallic =pow((materialColor.g),0.5);
			
			//roughness = u_roughness;
			//metallic = u_metallic;

			roughness = clamp(roughness, 0.09, 0.99);
			metallic = clamp(metallic, 0.0, 0.98);
		}

		//load albedo
		vec4 textureColor;
		{
			textureColor = texture(sampler2D(v_textureSampler), v_uv);
			if(textureColor.a <= 0){discard;}
			//gamma correction
			textureColor.rgb = toLinear(textureColor.rgb);

			if(isWater())
			{
				//textureColor.rgb = u_waterColor.rgb;
				textureColor.rgb = u_waterColor.rgb;
				roughness = 0.09;
				metallic = 0.0;
			}
		}
		
		
		vec3 skyBoxColor = texture(u_skyTexture, (gl_FragCoord.xy / textureSize(u_skyTexture, 0)) ).rgb;

		vec3 N = applyNormalMap(v_normal);
		//vec3 N = v_normal;
		//vec3 ViewSpaceVector = compute(u_positionInt, u_positionFloat, fragmentPositionI, fragmentPositionF);
		//float viewLength = length(ViewSpaceVector);
		//vec3 V = ViewSpaceVector / viewLength;
		
		float viewLength = length(v_semiViewSpacePos);
		vec3 V = -v_semiViewSpacePos / viewLength;

		//{
		//	vec4 worldSpaceV = inverse(u_view) * vec4(V, 0.0);
		//	vec3 worldSpaceDirection = worldSpaceV.xyz / worldSpaceV.w;
		//	V = worldSpaceDirection;
		//}

		//if(u_writeScreenSpacePositions != 0)
		{
			////out_screenSpacePositions.xyz = u_view* -ViewSpaceVector; 
			out_screenSpacePositions.xyz = (u_view * vec4(v_semiViewSpacePos,1)).xyz; //this is good
			//out_screenSpacePositions.a = 1;
		}
			
		out_normals = fromFloatTouShort(N);

		
		//caustics
		vec3 causticsColor = vec3(1);
		if(blockIsInWater)
		{
			vec2 dudv = vec2(0);
			dudv += texture(u_dudv, getDudvCoords(waterSpeed*2)).rg;
			dudv += texture(u_dudv, getDudvCoords5(waterSpeed)).rg*1;
			//dudv += texture(u_dudv, getDudvCoords2(waterSpeed*0.1)).rg * 0.4;
			//dudv *= texture(u_dudv, getDudvCoords2(1)).rg;

			vec2 coords = getDudvCoords(5.f) * causticsTextureScale + dudv / 10;
			//vec2 coords = dudv * ;
			

			causticsColor.r = pow(texture(u_caustics, coords + vec2(0,0)).r, causticsLightPower) * causticsLightStrength;
			causticsColor.g = pow(texture(u_caustics, coords + vec2(causticsChromaticAberationStrength,causticsChromaticAberationStrength)).g, causticsLightPower) * causticsLightStrength;
			causticsColor.b = pow(texture(u_caustics, coords + vec2(causticsChromaticAberationStrength,causticsChromaticAberationStrength)*2.0).b, causticsLightPower) * causticsLightStrength;


			//causticsColor = max(causticsColor, vec3(0,0,0));
			//out_color.rgb = texture(u_caustics, coords).rgb;
			//out_color.a = 1;
			//return ;
			//todo water uvs so the texture is continuous on all sides
		}
		

		vec3 finalColor = vec3(0);

		

		vec3 F0 = vec3(0.04); 
		F0 = mix(F0, textureColor.rgb, vec3(metallic));

		for(int i=0; i< u_lightsCount; i++)
		{
			vec3 L = compute(lights[i].rgb, vec3(0), fragmentPositionI, fragmentPositionF);
			//vec3 L =compute(u_pointPosI, u_pointPosF, fragmentPositionI, fragmentPositionF);
			
			float menhetanDistance = dot((abs(fragmentPositionI-lights[i].rgb)),vec3(1));
			//float menhetanDistance = dot((abs(L)),vec3(1));

			if(v_normalLight == 0){continue;}

			float LightDist = length(L);
			if((15-menhetanDistance) + 0.1 > v_normalLight){continue;}
			L = normalize(L);		

			//light += computeLight(N,L,V) * atenuationFunction(LightDist)*1.f;
			finalColor += computePointLightSource(L, metallic, roughness, vec3(pointLightColor,pointLightColor,pointLightColor)	* causticsColor, V, 
				textureColor.rgb, N, F0) 
				* atenuationFunction(LightDist) * ((v_normalLight/15.f) + 1.f) / 2.f;
				
		}
		//light = 0;

		//sun light
		vec3 sunLightColor = u_sunLightColor;
		if(v_skyLightUnchanged > 3)
		{
			
			sunLightColor *= 1-((15-v_skyLightUnchanged)/9.f);
			//sunLightColor *= ((v_ambientInt/15.f)*0.6 + 0.4f);

			if(isWater())
			{
				sunLightColor *= 1.0;
			}

			vec3 L = u_sunDirection;

			if(dot(u_sunDirection, vec3(0,1,0))> -0.2)
			{
				float shadow = shadowCalc2(dot(L, v_normal));

				finalColor += computePointLightSource(L, 
					metallic, roughness, sunLightColor * causticsColor, V, 
					textureColor.rgb, N, F0) * shadow * sqrt(blockAO);
			}
		}
		
		float dotNV = dot(N, V);
		vec3 freshnel = fresnelSchlickRoughness(dotNV, F0, roughness);
			
		out_color = vec4(finalColor,textureColor.a);
		out_color.a = 1-out_color.a;
		out_color.a *= dotNV;
		out_color.a = 1-out_color.a;
		//preview shadow
		//if(shadowCalc(dot(u_sunDirection, v_normal)) < 0.5)
		//{
		//	out_color.rgb = mix(out_color.rgb, vec3(1,0.5,0.7), 0.3);
		//}

		//ambient
		//todo light sub scatter
		bool ssrSuccess = false;

		if(physicallyAccurateReflections) //phisically accurate ambient
		{
			vec3 ssrNormal = N;
			//if(isWater()){ssrNormal = v_normal;}
			vec3 finalAmbient = computeAmbientTerm(computedAmbient, ssrNormal,
			V, F0, roughness, metallic, textureColor.rgb, ssrSuccess, out_color.rgb);
			out_color.rgb += finalAmbient;
		}else
		{
			out_color.rgb += textureColor.rgb * computedAmbient;
		}
		
		if(u_showLightLevels != 0)
		{
			vec4 numbersColor = texture(u_numbers, vec2(v_uv.x / 16.0 + (1/16.0)*v_ambientInt, v_uv.y));
			if(numbersColor.a > 0.1)
			{out_color.rgb = mix(toLinear(numbersColor.rgb), out_color.rgb, 0.5);}
		}
		

		if(u_underWater != 0)
		{


			float finalCoeficient = (computeFogUnderWater(viewLength)) * u_underwaterDarkenStrength 
								+ (1-u_underwaterDarkenStrength);
			//
			//float causticsBias = luminosity(sunLightColor *causticsColor);
			//
			//finalCoeficient += causticsBias * 0.01;

			finalCoeficient = clamp(finalCoeficient, 0, 1);
			out_color.rgb = mix(u_underWaterColor.rgb, out_color.rgb, 
								finalCoeficient
							);
		}


		//godRays
		if(false)
		{
			vec3 godRaysContribution = vec3(0);

			vec3 start = vec3((fragmentPositionI - u_lightPos) + fragmentPositionF);
			
			vec4 viewSpaceV = u_view * vec4(V, 0.0);
			vec3 viewSpaceDirection = viewSpaceV.xyz / viewSpaceV.w;
			
			vec3 direction = -normalize(start);

			vec3 end = vec3(0);
			float advance = length(start)/30;
			for(int i=1; i<30;i++)
			{
				start += direction*advance;
				
				if(testShadow(start, 1) > 0.5)
				{
					godRaysContribution += vec3(0.01);
				}

			}

			godRaysContribution = clamp(godRaysContribution, vec3(0), vec3(0.5));

			out_color.rgb += godRaysContribution;
		}


		//gamma correction + HDR
		//out_color.rgb *= u_exposure;
		//
		////vec3 purkine = purkineShift(out_color.rgb);
		////out_color.rgb = mix(out_color.rgb, purkine, 0.05);
		//
		//out_color.rgb = tonemapFunction(out_color.rgb);
		//out_color.rgb = toGammaSpace(out_color.rgb);
		//
		//out_color = clamp(out_color, vec4(0), vec4(1));
		
		//ssr
		//if(false)
		if(!physicallyAccurateReflections)
		if(roughness < 0.45)
		{
			vec2 fragCoord = gl_FragCoord.xy / textureSize(u_lastFramePositionViewSpace, 0).xy;

			//float dotNVClamped = clamp(dotNV, 0.0, 0.99);
			
			vec3 posViewSpace = texture(u_lastFramePositionViewSpace, 
				(fragCoord)).xyz;
			vec3 pos = vec3(u_inverseView * vec4(posViewSpace,1));

			vec3 viewSpaceNormal = normalize( vec3(transpose(inverse(mat3(u_view))) * N));

			vec3 ssrNormal = N;
			if(isWater()){ssrNormal = v_normal;}

			float mixFactor = 0;
			//vec3 ssr = SSR(posViewSpace, N, metallic, F, mixFactor, roughness, pos, V, viewSpaceNormal, 
			//	textureSize(u_lastFramePositionViewSpace, 0).xy);
			vec3 ssr = SSR(ssrSuccess, posViewSpace, ssrNormal, mixFactor, pow(roughness,2), pos, V, viewSpaceNormal, 
				textureSize(u_lastFramePositionViewSpace, 0).xy);
			
			
			
			//if is water just reflect 100% because we deal with it later
			if(isWater())
			{
				if(ssrSuccess) 
				{
					//out_color.rgb = mix(u_waterColor, ssr, 0.9);
					out_color.rgb = ssr;
				}
				//out_color.a = 1;
			}else
			{
				if(ssrSuccess) 
				{
					//ssr = mix(ssr, out_color.rgb, dotNV);
					ssr = mix(ssr, out_color.rgb, freshnel);
					out_color.rgb = mix(ssr, out_color.rgb, pow(roughness, 0.5));
				}	

				//if(success) {out_color.rgb = vec3(0,0,1);}else
				//{out_color.rgb = vec3(1,0,0);}

				//out_color.rgb *= posViewSpace;
			}
			//if(success)out_color.rgb = ssr;
			
		}
		


		//out_color.rgb = linear_to_srgb(out_color.rgb);
		
		//fog
		//{
		//	out_color.a *= ;	
		//}
		
		//is water	
		//if(false)
		if(isWater())
		{
			float reflectivity = dotNV;
			//float reflectivity = 0;

			//float mixFactor = clamp(fresnelSchlickWater(reflectivity),0,1);
			//float mixFactor = pow(clamp(1-reflectivity, 0, 1),2) * 0.2+0.1;
			float mixFactor = clamp(pow(1-clamp(reflectivity,0,1),2) * 0.8+0.1,0,1);
			//float mixFactor = 1-clamp(reflectivity,0,1);
			//if(ssrSuccess){mixFactor = pow(mixFactor, 0.9);}

			vec2 p = v_fragPos.xy / v_fragPos.w;
			p += 1;
			p/=2;

			float nonLinear = 0;
			float lastDepth = getLastDepthLiniarized(p, nonLinear);
			float currentDepth = linearizeDepth(gl_FragCoord.z);		
			float waterDepth = lastDepth - currentDepth;


			if(u_hasPeelInformation != 0)
			{
				

				
				//visualize dudv
				//out_color.rgb = texture(u_dudv, getDudvCoords3(1)).rgb;
				//out_color.a = 1;
				//return;

				vec2 dudv = vec2(0);
				dudv += texture(u_dudv, getDudvCoords(waterSpeed)).rg;
				dudv += texture(u_dudv, getDudvCoords2(dudv.x)).rg * 0.01;
				//dudv += texture(u_dudv, getDudvCoords3(1)).rg;
				vec2 dudvConv = (dudv * 2) - 1;
				vec2 distorsionCoord = dudvConv * 0.009;	
				float distortDepth = getLastDepthLiniarized(p + distorsionCoord, nonLinear);

				
				//v_fragPos
				vec3 perturbedFragPos = (u_inverseProjMat * vec4(p + distorsionCoord, nonLinear, 1)).rgb;
				vec3 direction = perturbedFragPos - v_fragPos.xyz;
				
				vec3 peelTexture = vec3(1,0.5,0.5);
				float finalDepth = 0;			
				if(distortDepth < currentDepth + 0.1)
				//if(dot(direction, v_normal) > 0 || distortDepth < currentDepth + 0.1)
				{
					//geometry in front of water
					//keep original texture
					peelTexture = texture(u_PeelTexture, p).rgb;
					finalDepth = waterDepth;
				}else
				{
					float distortWaterDepth = distortDepth - currentDepth;

					peelTexture = texture(u_PeelTexture, p + distorsionCoord).rgb;
					finalDepth = distortWaterDepth;
				}


				//darken water with depth
				peelTexture = mix(peelTexture, u_waterColor*0.7, 0.6 * clamp(pow(finalDepth/18.0,2),0,1) );


				//clamp(mixFactor,0,1);
				out_color.rgb = mix(peelTexture, out_color.rgb,  mixFactor);
				//out_color.rgb = peelTexture + out_color.rgb*mixFactor;


				//out_color.rgb = mix(out_color.rgb, peelTexture, clamp(reflectivity,0,1));
				//out_color.rgb = mix(peelTexture, out_color.rgb, pow(clamp(1-reflectivity, 0, 1),2) * 0.2+0.1 );
				//out_color.rgb = mix(peelTexture, u_waterColor*out_color.rgb, pow(clamp(1-reflectivity, 0, 1),2) * 0.2+0.1 );
				//out_color.rgb = mix(peelTexture, out_color.rgb, 0.0);
				

				//darken deep stuff, todo reenable and use final depth
				//out_color.rgb = vec3(waterDepth/20);
				//out_color.r = (currentDepth) / 10;
				//out_color.g = (lastDepth) / 10;
				//out_color.b = (lastDepth) / 10;
				//out_color.rg = dudv.rg;
				//out_color.b = 0;

				out_color.a = 1;
				//out_color.a = clamp(waterDepth/5,0,1);
				//out_color.r = clamp(waterDepth/5,0,1);
				//out_color.g = clamp(waterDepth/5,0,1);
				//out_color.b = clamp(waterDepth/5,0,1);
			}else
			{

				if(ssrSuccess)
				{
					float depthMix = 0.2 * clamp(pow(waterDepth/18.0,2),0,1) + 0;
					out_color.rgb = mix(out_color.rgb, u_waterColor*0.7, depthMix);
					//out_color.a = pow(mixFactor, depthMix); 
					//out_color.a = pow(mixFactor, 0.5); 
				}else
				{
					float depthMix = 0.4 * clamp(pow(waterDepth/18.0,2),0,1) + 0;
					out_color.rgb = mix(out_color.rgb, u_waterColor*0.7, depthMix);
					//out_color.a = pow(mixFactor, depthMix); 
				}
				
				out_color.a = pow(mixFactor, 0.7); 

				//make it darker so you can more easily see the back face
			}

			//out_color.rgba = vec4(waterDepth/15,0,0,1);
			

		}


		//fog
		if(u_underWater == 0)
		{

			out_color.rgb = mix(skyBoxColor, out_color.rgb, 
								computeFog(viewLength)
							);
		}

		//out_color.r = 1-roughness;
		//out_color.g = metallic;
		//out_color.b = 0;
		//out_color.a = 1;
	
	}

	
}


//////////////////////////////////////////////
//https://imanolfotia.com/blog/1
//https://github.com/ImanolFotia/Epsilon-Engine/blob/master/bin/Release/shaders/SSR.glsl
//SSR


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
const float SSR_maxRayStep = 100.2;
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
 
		depth = textureLod(u_lastFramePositionViewSpace, projectedCoord.xy, 2).z;
		//depth = texture(u_lastFramePositionViewSpace, projectedCoord.xy).z;
 
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
		
		depth = texture(u_lastFramePositionViewSpace, projectedCoord.xy).z;
	
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

		depth = textureLod(u_lastFramePositionViewSpace,
			reprojectViewSpace(projectedCoord.xy),2).z;
		//depth = texture(u_lastFramePositionViewSpace, projectedCoord.xy).z;

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
			
			depth = texture(u_lastFramePositionViewSpace, reprojectViewSpace(Result.xy)).z;
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
	vec3 lastFrameColor = textureLod(u_lastFrameColor, 
		reprojectViewSpace(coords.xy), 0).rgb;
	//vec3 SSR = lastFrameColor * clamp(ReflectionMultiplier, 0.0, 0.9) * F;  
	vec3 SSR = lastFrameColor;

	mixFactor = clamp(ReflectionMultiplier, 0.0, 1.f);  

	success = true;
	return SSR;
}

////////////////////////////////////////////


//normal in world space, viewDir, wp = world space position
//F = freshnell
//todo send F 
vec3 computeAmbientTerm(vec3 gammaAmbient, vec3 N, vec3 V, vec3 F0, float roughness, 
float metallic, vec3 albedo, 
out bool ssrSuccess, vec3 out_color
)
{
	ssrSuccess = false;
	vec3 ambient = vec3(0); //result
		
	float dotNVClamped = clamp(dot(N, V), 0.0, 0.99);
	vec3 F = fresnelSchlickRoughness(dotNVClamped, F0, roughness);
	vec3 kS = F;

	vec3 irradiance = vec3(0,0,0); //diffuse
	vec3 radiance = vec3(0,0,0); //specular
	vec2 brdf = vec2(0,0);
	vec2 brdfVec = vec2(dotNVClamped, roughness);

	if(true) //ssr
	{
		//radiance = SSR(viewPos, N, metallic, F, mixFactor, roughness, wp, V, viewSpaceNormal, rezolution);
	
		//if(roughness < 0.5)
		{
			vec2 fragCoord = gl_FragCoord.xy / textureSize(u_lastFramePositionViewSpace, 0).xy;
			
			vec3 posViewSpace = texture(u_lastFramePositionViewSpace,
				reprojectViewSpace(fragCoord)).xyz;
			vec3 pos = vec3(u_inverseView * vec4(posViewSpace,1));

			vec3 viewSpaceNormal = normalize( vec3(transpose(inverse(mat3(u_view))) * N));

			vec3 ssrNormal = N;
			if(isWater()){ssrNormal = v_normal;}

			float mixFactor = 0;
			//vec3 ssr = SSR(posViewSpace, N, metallic, F, mixFactor, roughness, pos, V, viewSpaceNormal, 
			//	textureSize(u_lastFramePositionViewSpace, 0).xy);
			vec3 ssr = SSR(ssrSuccess, posViewSpace, ssrNormal, mixFactor, pow(roughness,2), pos, V, viewSpaceNormal, 
				textureSize(u_lastFramePositionViewSpace, 0).xy) * 1.f;
			
			if(ssrSuccess) 
			{
				radiance.rgb = ssr;
			}

			//if is water just reflect 100% because we deal with it later
			//if(isWater())
			//{
			//	if(ssrSuccess) 
			//	{
			//		//out_color.rgb = mix(u_waterColor, ssr, 0.9);
			//		radiance.rgb = ssr;
			//	}
			//	//out_color.a = 1;
			//}else
			//{
			//	if(ssrSuccess) 
			//	{
			//		ssr = mix(ssr, out_color.rgb, dotNVClamped);
			//		radiance.rgb = mix(ssr, out_color.rgb, pow(roughness, 0.5));
			//	}	
			//
			//}
			
		}
	
	}

	//todo
	//if(lightPassData.skyBoxPresent != 0)
	//{
	//	
	//	irradiance = texture(u_skyboxIradiance, N).rgb * gammaAmbient; //this color is coming directly at the object
	//
	//	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	//	const float MAX_REFLECTION_LOD = 4.0;
	//	
	//	if(mixFactor < 0.999)
	//	{
	//		radiance = mix(textureLod(u_skyboxFiltered, R, roughness * MAX_REFLECTION_LOD).rgb * gammaAmbient, radiance, mixFactor);
	//	}
	//
	//	//brdfVec.y = 1 - brdfVec.y; 
	//	brdf  = texture(u_brdfTexture, brdfVec).rg;
	//
	//}else
	{
		
		//radiance = mix(gammaAmbient, radiance, mixFactor);
		irradiance = gammaAmbient ; //this color is coming directly at the object

		//brdfVec.y = 1 - brdfVec.y; 
		brdf = texture(u_brdf, brdfVec).rg;
	}

	//if(lightPassData.lightSubScater == 0)
	//{
	//	vec3 kD = 1.0 - kS;
	//	kD *= 1.0 - metallic;
	//	
	//	vec3 diffuse = irradiance * albedo;
	//	
	//	vec3 specular = radiance * (F * brdf.x + brdf.y);
	//
	//	//no multiple scattering
	//	ambient = (kD * diffuse + specular);
	//}else
	{
		//http://jcgt.org/published/0008/01/03/
		// Multiple scattering version
		vec3 FssEss = kS * brdf.x + brdf.y;
		float Ess = brdf.x + brdf.y;
		float Ems = 1-Ess;
		vec3 Favg = F0 + (1-F0)/21;
		vec3 Fms = FssEss*Favg/(1-(1-Ess)*Favg);
		// Dielectrics
		vec3 Edss = 1 - (FssEss + Fms * Ems);
		vec3 kD = albedo * Edss;

		// Multiple scattering version
		ambient = FssEss * radiance + (Fms*Ems+kD) * irradiance;
	}
	
	return ambient;
}
