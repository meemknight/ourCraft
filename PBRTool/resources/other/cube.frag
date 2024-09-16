#version 430 core
layout (location = 0) out vec4 outColor;

in vec2 v_uv;
in vec3 v_normal;


layout(binding = 0) uniform sampler2D u_texture;
layout(binding = 1) uniform sampler2D u_PBR;
layout(binding = 2) uniform sampler2D u_brdf;



const float PI = 3.14159265359;

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


vec3 computeAmbientTerm(vec3 gammaAmbient, vec3 F0, float roughness, 
float metallic, vec3 albedo, float dotNVClamped
)
{
	vec3 ambient = vec3(0); //result
		
	vec3 F = fresnelSchlickRoughness(dotNVClamped, F0, roughness);
	vec3 kS = F;

	vec3 irradiance = vec3(0,0,0); //diffuse
	vec3 radiance = vec3(0,0,0); //specular
	vec2 brdf = vec2(0,0);
	vec2 brdfVec = vec2(dotNVClamped, roughness);

	
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

//

//https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
/*
=================================================================================================

  Baking Lab
  by MJP and David Neubelt
  http://mynameismjp.wordpress.com/

  All code licensed under the MIT license

=================================================================================================
 The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
 credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)
*/

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3x3 ACESInputMat = mat3x3
(
	0.59719, 0.35458, 0.04823,
	0.07600, 0.90834, 0.01566,
	0.02840, 0.13383, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3x3 ACESOutputMat = mat3x3
(
	 1.60475, -0.53108, -0.07367,
	-0.10208,  1.10813, -0.00605,
	-0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

vec3 ACESFitted(vec3 color)
{
	color = transpose(ACESInputMat) * color;
	// Apply RRT and ODT
	color = RRTAndODTFit(color);
	color = transpose(ACESOutputMat) * color;
	color = clamp(color, 0, 1);
	return color;
}

vec3 ACESFilmSimple(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0, 1);

}
//


// https://advances.realtimerendering.com/s2021/jpatry_advances2021/index.html
// https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.40.9608&rep=rep1&type=pdf
vec4 rgb_to_lmsr(vec3 c)
{
	const mat4x3 m = mat4x3(
		0.31670331, 0.70299344, 0.08120592, 
		0.10129085, 0.72118661, 0.12041039, 
		0.01451538, 0.05643031, 0.53416779, 
		0.01724063, 0.60147464, 0.40056206);
	return c * m;
}
vec3 lms_to_rgb(vec3 c)
{
	const mat3 m = mat3(
		 4.57829597, -4.48749114,  0.31554848, 
		-0.63342362,  2.03236026, -0.36183302, 
		-0.05749394, -0.09275939,  1.90172089);
	return c * m;
}

// https://www.ncbi.nlm.nih.gov/pmc/articles/PMC2630540/pdf/nihms80286.pdf
vec3 purkineShift(vec3 c)
{
	const vec3 m = vec3(0.63721, 0.39242, 1.6064);
	const float K = 45.0;
	const float S = 10.0;
	const float k3 = 0.6;
	const float k5 = 0.2;
	const float k6 = 0.29;
	const float rw = 0.139;
	const float p = 0.6189;
	
	vec4 lmsr = rgb_to_lmsr(c);
	
	vec3 g = vec3(1.0) / sqrt(vec3(1.0) + (vec3(0.33) / m) * (lmsr.xyz + vec3(k5, k5, k6) * lmsr.w));
	
	float rc_gr = (K / S) * ((1.0 + rw * k3) * g.y / m.y - (k3 + rw) * g.x / m.x) * k5 * lmsr.w;
	float rc_by = (K / S) * (k6 * g.z / m.z - k3 * (p * k5 * g.x / m.x + (1.0 - p) * k5 * g.y / m.y)) * lmsr.w;
	float rc_lm = K * (p * g.x / m.x + (1.0 - p) * g.y / m.y) * k5 * lmsr.w;
	
	vec3 lms_gain = vec3(-0.5 * rc_gr + 0.5 * rc_lm, 0.5 * rc_gr + 0.5 * rc_lm, rc_by + rc_lm);
	vec3 rgb_gain = lms_to_rgb(lms_gain);
	
	return c + rgb_gain;
}

float luminance(vec3 c)
{
	return dot(c, vec3(0.2126390059, 0.7151686788, 0.0721923154));
}

vec3 linear_to_srgb(vec3 c)
{
	const float yb = pow(0.055 * 2.4 / ((2.4 - 1.0) * (1.0 + 0.055)), 2.4);
	const float rs = pow((2.4 - 1.0) / 0.055, 2.4 - 1.0) * pow((1.0 + 0.055) / 2.4, 2.4);
	return vec3(
		(c.x >= yb) ? ((1.0 + 0.055) * pow(c.x, 1.0 / 2.4) - 0.055) : (c.x * rs),
		(c.y >= yb) ? ((1.0 + 0.055) * pow(c.y, 1.0 / 2.4) - 0.055) : (c.y * rs),
		(c.z >= yb) ? ((1.0 + 0.055) * pow(c.z, 1.0 / 2.4) - 0.055) : (c.z * rs));
}

//https://www.shadertoy.com/view/4tXcWr
//srgb
vec3 fromLinearSRGB(vec3 linearRGB)
{
	bvec3 cutoff = lessThan(linearRGB, vec3(0.0031308));
	vec3 higher = vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055);
	vec3 lower = linearRGB * vec3(12.92);

	return mix(higher, lower, cutoff);
}

float fromLinearSRGB(float linearRGB)
{
	bool cutoff = linearRGB < float(0.0031308);
	float higher = float(1.055)*pow(linearRGB, float(1.0/2.4)) - float(0.055);
	float lower = linearRGB * float(12.92);

	return mix(higher, lower, cutoff);
}

in vec3 v_ViewSpacePos;

void main()
{
	vec4 color = texture(u_texture, v_uv);
	if(color.a <= 0){discard;}
	color.rgb = toLinearSRGB(color.rgb);
	vec3 N = normalize(v_normal);
	vec3 computedAmbient = vec3(1,1,1);
	
	vec3 finalColor = vec3(0);

	vec3 V = -v_ViewSpacePos / length(v_ViewSpacePos);

	float dotNVClamped = clamp(dot(N, V), 0.0, 0.99);


	//load material
	float roughness = 0;
	float metallic = 0;
	{
		vec2 materialColor = texture(u_PBR, v_uv).rg;
		
		roughness = pow((1 - materialColor.r),2);
		metallic =pow((materialColor.g),0.5);
		
		//roughness = u_roughness;
		//metallic = u_metallic;

		roughness = clamp(roughness, 0.09, 0.99);
		metallic = clamp(metallic, 0.0, 0.98);
	}

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, color.rgb, vec3(metallic));


	{
		vec3 L = normalize(vec3(-0.1,-0.4,-1));

		finalColor += computePointLightSource(L, 
					metallic, roughness, vec3(1,1,1), V, 
					color.rgb, N, F0);
	
	
	}

	finalColor += computeAmbientTerm(computedAmbient, F0, roughness, metallic,
		color.rgb, dotNVClamped);


	finalColor = ACESFitted(finalColor);
	finalColor = fromLinearSRGB(finalColor);



	outColor.rgb = finalColor;
	outColor.a = 1;

}	