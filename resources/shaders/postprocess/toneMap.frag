#version 330 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;
uniform sampler2D u_color;
uniform int u_tonemapper;
uniform float u_exposure;

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

/////////////////////////////////////////////////////////////////////////////////////////
//^aces^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
//agx
//https://www.shadertoy.com/view/cd3XWr
//https://www.shadertoy.com/view/dlcfRX
/////////////////////////////////////////////////////////////////////////////////////////

#define AGX_LOOK 2

vec3 toLinearAXG(vec3 sRGB) {
  bvec3 cutoff = lessThan(sRGB, vec3(0.04045));
  vec3 higher = pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4));
  vec3 lower = sRGB/vec3(12.92);
  
  return mix(higher, lower, cutoff);
}

float toLinearAXG(float sRGB) {

  bool cutoff = (sRGB < 0.04045f);

  float higher = pow((sRGB + float(0.055))/float(1.055), float(2.4));
  float lower = sRGB/float(12.92);
  
  return mix(higher, lower, cutoff);
}

vec3 AGX(vec3 col)
{
	col = mat3(.842, .0423, .0424, .0784, .878, .0784, .0792, .0792, .879) * col;
	// Log2 space encoding
	col = clamp((log2(col) + 12.47393) / 16.5, vec3(0), vec3(1));
	// Apply sigmoid function approximation
	col = .5 + .5 * sin(((-3.11 * col + 6.42) * col - .378) * col - 1.44);
	// AgX look (optional)
	#if AGX_LOOK == 1
	// Golden
	col = mix(vec3(dot(col, vec3(.216,.7152,.0722))), col * vec3(1.,.9,.5), .8);
	#elif AGX_LOOK == 2
	// Punchy
	col = mix(vec3(dot(col, vec3(.216,.7152,.0722))), pow(col,vec3(1.35)), 1.4);
	#endif
	
	return col;
}

/////////////////////////////////////////////////////////////////////////////////////////
//^agx^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////
//ZCAM
//https://www.shadertoy.com/view/dlGBDD
//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
/////////////////////////////////////////////////////////////////////////////////////////



// eotf_pq parameters
const float Zcam_Lp = 10000.0;
const float Zcam_m1 = 2610.0 / 16384.0;
const float Zcam_m2 = 1.7 * 2523.0 / 32.0;
const float Zcam_c1 = 107.0 / 128.0;
const float Zcam_c2 = 2413.0 / 128.0;
const float Zcam_c3 = 2392.0 / 128.0;

vec3 eotf_pq(vec3 x)
{
	x = sign(x) * pow(abs(x), vec3(1.0 / Zcam_m2));
	x = sign(x) * pow((abs(x) - Zcam_c1) / (Zcam_c2 - Zcam_c3 * abs(x)), vec3(1.0 / Zcam_m1)) * Zcam_Lp;
	return x;
}

vec3 eotf_pq_inverse(vec3 x)
{
	x /= Zcam_Lp;
	x = sign(x) * pow(abs(x), vec3(Zcam_m1));
	x = sign(x) * pow((Zcam_c1 + Zcam_c2 * abs(x)) / (1.0 + Zcam_c3 * abs(x)), vec3(Zcam_m2));
	return x;
}

// XYZ <-> ICh parameters
const float Zcam_W = 140.0;
const float Zcam_b = 1.15;
const float Zcam_g = 0.66;

vec3 XYZ_to_ICh(vec3 XYZ)
{
	XYZ *= Zcam_W;
	XYZ.xy = vec2(Zcam_b, Zcam_g) * XYZ.xy - (vec2(Zcam_b, Zcam_g) - 1.0) * XYZ.zx;
	
	const mat3 XYZ_to_LMS = transpose(mat3(
		 0.41479,   0.579999, 0.014648,
		-0.20151,   1.12065,  0.0531008,
		-0.0166008, 0.2648,   0.66848));
	
	vec3 LMS = XYZ_to_LMS * XYZ;
	LMS = eotf_pq_inverse(LMS);
	
	const mat3 LMS_to_Iab = transpose(mat3(
		0.0,       1.0,      0.0,
		3.524,    -4.06671,  0.542708,
		0.199076,  1.0968,  -1.29588));
	
	vec3 Iab = LMS_to_Iab * LMS;
	
	float I = eotf_pq(vec3(Iab.x)).x / Zcam_W;
	float C = length(Iab.yz);
	float h = atan(Iab.z, Iab.y);
	return vec3(I, C, h);
}

vec3 ICh_to_XYZ(vec3 ICh)
{
	vec3 Iab;
	Iab.x = eotf_pq_inverse(vec3(ICh.x * Zcam_W)).x;
	Iab.y = ICh.y * cos(ICh.z);
	Iab.z = ICh.y * sin(ICh.z);
	
	const mat3 Iab_to_LMS = transpose(mat3(
		1.0, 0.2772,  0.1161,
		1.0, 0.0,     0.0,
		1.0, 0.0426, -0.7538));
	
	vec3 LMS = Iab_to_LMS * Iab;
	LMS = eotf_pq(LMS);
	
	const mat3 LMS_to_XYZ = transpose(mat3(
		 1.92423, -1.00479,  0.03765,
		 0.35032,  0.72648, -0.06538,
		-0.09098, -0.31273,  1.52277));
	
	vec3 XYZ = LMS_to_XYZ * LMS;
	XYZ.x = (XYZ.x + (Zcam_b - 1.0) * XYZ.z) / Zcam_b;
	XYZ.y = (XYZ.y + (Zcam_g - 1.0) * XYZ.x) / Zcam_g;
	return XYZ / Zcam_W;
}

const mat3 XYZ_to_sRGB = transpose(mat3(
	 3.2404542, -1.5371385, -0.4985314,
	-0.9692660,  1.8760108,  0.0415560,
	 0.0556434, -0.2040259,  1.0572252));

const mat3 sRGB_to_XYZ = transpose(mat3(
	0.4124564, 0.3575761, 0.1804375,
	0.2126729, 0.7151522, 0.0721750,
	0.0193339, 0.1191920, 0.9503041));

bool in_sRGB_gamut(vec3 ICh)
{
	vec3 sRGB = XYZ_to_sRGB * ICh_to_XYZ(ICh);
	return all(greaterThanEqual(sRGB, vec3(0.0))) && all(lessThanEqual(sRGB, vec3(1.0)));
}

vec3 Zcam_tonemap(vec3 sRGB)
{	
	vec3 ICh = XYZ_to_ICh(sRGB_to_XYZ * sRGB);
	
	const float s0 = 0.71;
	const float s1 = 1.04;
	const float p = 1.40;
	const float t0 = 0.01;
	float n = s1 * pow(ICh.x / (ICh.x + s0), p);
	ICh.x = clamp(n * n / (n + t0), 0.0, 1.0);
	
	if (!in_sRGB_gamut(ICh))
	{
		float C = ICh.y;
		ICh.y -= 0.5 * C;
		
		for (float i = 0.25; i >= 1.0 / 256.0; i *= 0.5)
		{
			ICh.y += (in_sRGB_gamut(ICh) ? i : -i) * C;
		}
	}
	
	return XYZ_to_sRGB * ICh_to_XYZ(ICh);
}


/////////////////////////////////////////////////////////////////////////////////////////
//^ZCAM^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/////////////////////////////////////////////////////////////////////////////////////////



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

vec4 toLinearSRGB(vec4 sRGB)
{
	bvec4 cutoff = lessThan(sRGB, vec4(0.04045));
	vec4 higher = pow((sRGB + vec4(0.055))/vec4(1.055), vec4(2.4));
	vec4 lower = sRGB/vec4(12.92);
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
	if(u_tonemapper == 0)
	{
		//return pow(a, float(2.2));
		return toLinearSRGB(a);
	}else if(u_tonemapper == 1)
	{
		return toLinearAXG(a);
	}else if(u_tonemapper == 2)
	{
		return toLinearSRGB(a);
	}

}

vec3 toLinear(vec3 a)
{
	return vec3(toLinear(a.r),toLinear(a.g),toLinear(a.b));
}

vec3 tonemapFunction(vec3 c)
{
	if(u_tonemapper == 0)
	{
		return ACESFitted(c);
	}else if(u_tonemapper == 1)
	{
		return AGX(c);
	}else if(u_tonemapper == 2)
	{
		return Zcam_tonemap(c);
	}

}

//(to screen)
vec3 toGammaSpace(vec3 a)
{

	if(u_tonemapper == 0)
	{
		//return pow(a, vec3(1.f/2.2));
		return fromLinearSRGB(a);
	}else if(u_tonemapper == 1)
	{
		return a;
	}else if(u_tonemapper == 2)
	{
		return fromLinearSRGB(a);
	}
}





void main()
{

	//gamma correction + HDR
	
	vec3 out_color = texture(u_color, v_texCoords).rgb;
	out_color.rgb *= u_exposure;
	
	//vec3 purkine = purkineShift(out_color.rgb);
	//out_color.rgb = mix(out_color.rgb, purkine, 0.05);
	
	out_color.rgb = tonemapFunction(out_color.rgb);
	out_color.rgb = toGammaSpace(out_color.rgb);
	
	out_color = clamp(out_color, vec3(0), vec3(1));

	outColor = vec4(out_color, 1);
}

