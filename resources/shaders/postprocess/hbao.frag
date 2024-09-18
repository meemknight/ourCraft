#version 330 core

vec2 hash12(vec2 p) {
	vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
	p3 += dot(p3, p3.yzx + 19.19);
	vec2 rez = fract((p3.xx + p3.yz)*p3.zy);
	rez *= 2;
	rez -= 1;
	rez = normalize(rez);
	return rez;
}

vec3 hash123(vec2 p) {
	vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
	p3 += dot(p3, p3.yzx + 19.19);
	vec2 rez = fract((p3.xx + p3.yz) * p3.zy);
	rez *= 2.0;
	rez -= 1.0;
	return normalize(vec3(rez.x, rez.y, p3.x));
}


/*

//https://developer.download.nvidia.com/presentations/2008/SIGGRAPH/HBAO_SIG08b.pdf

layout(location = 0) out vec4 fragColor;

noperspective in vec2 v_texCoords;

uniform sampler2D u_gPosition;
uniform isampler2D u_gNormal;
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

const float RAD = 0.4;
const float bias = (3.141592/360)*40.f;


vec2 computeAO(vec3 normal, vec2 direction, vec2 screenSize, vec3 fragPos)
{
	float RAD_FOR_DIRECTION = length( direction*vec2(10.0) / (vec2(abs(fragPos.z))*screenSize));

	vec3 viewVector = normalize(fragPos);

	vec3 leftDirection = cross(viewVector, vec3(direction,0));
	vec3 projectedNormal = normal - dot(leftDirection, normal) * leftDirection;
	float projectedLen = length(projectedNormal);
	projectedNormal /= projectedLen;

	vec3 tangent = cross(projectedNormal, leftDirection);


	//vec3 leftDirection = cross(normal, vec3(direction, 0));
	//vec3 tangent = normalize( cross(leftDirection, normal) );


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

	vec3 fragPos   = texture(u_gPosition, v_texCoords).xyz; //view space

	if(fragPos.z == -INFINITY){fragColor.a = 1; return;}

	vec3 normal    = normalize( vec3( 
	transpose(inverse(mat3(u_view))) * 
		fromuShortToFloat(texture(u_gNormal, v_texCoords).xyz)
		));

	
	vec3 fragPosMarch = texture(u_gPosition, v_texCoords).xyz;
	//vec2 randomVec = vec2(0,1);
	vec2 randomVec = hash12(fragPosMarch.xz);

	vec2 rez = vec2(0,0);

	vec3 viewVector = normalize(fragPos);


	rez += computeAO(normal, vec2(randomVec), screenSize, fragPos);
	rez += computeAO(normal, -vec2(randomVec), screenSize, fragPos);
	rez += computeAO(normal, vec2(-randomVec.y,randomVec.x), screenSize, fragPos);
	rez += computeAO(normal, vec2(randomVec.y, -randomVec.x), screenSize, fragPos);
	
	rez.x /= rez.y;
	
	fragColor.r = rez.x;
}

*/



layout(location = 0) out vec4 fragColor;


noperspective in highp vec2 v_texCoords;

uniform sampler2D u_gPosition; //position view space
uniform isampler2D u_gNormal;

uniform mat4 u_view; // camera view matrix
uniform mat4 u_projection; // 


const int kernelSize = 64;

float radius = 0.5;
float bias = 0.200;
int samplesTestSize = 32; // should be less than kernelSize

vec3 samples[64] = vec3[64](
	vec3(0.40380573, 0.62488124, 0.66817989),
	vec3(0.94561738, -0.19045641, 0.26369323),
	vec3(-0.36234207, 0.53053288, 0.76631788),
	vec3(0.65091370, -0.18482486, 0.73630912),
	vec3(-0.94016065, 0.04093710, 0.33826337),
	vec3(0.77078830, 0.48419549, 0.41405329),
	vec3(0.04710537, 0.80822038, 0.58699310),
	vec3(0.64122709, -0.59576237, 0.48362695),
	vec3(0.84031517, -0.51174415, 0.17885284),
	vec3(0.55245552, -0.65680562, 0.51322439),
	vec3(-0.19991502, 0.97309764, 0.11452063),
	vec3(-0.11788207, -0.52548180, 0.84259878),
	vec3(0.54185599, 0.79889750, 0.26106486),
	vec3(-0.95743544, -0.10821882, 0.26759310),
	vec3(-0.63843815, -0.00817195, 0.76962975),
	vec3(0.74838251, 0.51509018, 0.41785849),
	vec3(-0.12723010, 0.97333021, 0.19089472),
	vec3(0.66155893, 0.66285931, 0.35065272),
	vec3(0.77054173, -0.49873825, 0.39689495),
	vec3(-0.78277062, 0.18149567, 0.59525580),
	vec3(0.47278898, -0.02632285, 0.88078243),
	vec3(0.52695586, -0.82310353, 0.21170286),
	vec3(0.58856750, 0.39199484, 0.70705611),
	vec3(-0.51261935, 0.69005005, 0.51093281),
	vec3(-0.82765341, 0.56071328, 0.02429934),
	vec3(0.64003532, -0.33541669, 0.69126727),
	vec3(0.69955083, 0.20104806, 0.68571737),
	vec3(-0.72598904, 0.66346620, 0.18097654),
	vec3(0.04473761, -0.37769375, 0.92484916),
	vec3(0.41045231, 0.33813962, 0.84687101),
	vec3(-0.63402933, -0.67922671, 0.36967268),
	vec3(-0.78910733, 0.25068410, 0.56077367),
	vec3(-0.74993517, 0.64387653, 0.15172427),
	vec3(0.54654687, -0.82690816, 0.13232314),
	vec3(-0.66887472, 0.02596586, 0.74292152),
	vec3(-0.87615884, -0.44461154, 0.18618880),
	vec3(0.80378114, 0.54236414, 0.24449342),
	vec3(0.72960392, 0.45071996, 0.51432444),
	vec3(0.28063699, -0.52537512, 0.80325828),
	vec3(-0.50052699, 0.61190699, 0.61240719),
	vec3(0.86255932, -0.46208887, 0.20607111),
	vec3(0.75146215, 0.56325201, 0.34358668),
	vec3(0.75086698, 0.38444143, 0.53703218),
	vec3(0.18571530, 0.57723136, 0.79518161),
	vec3(-0.62424428, 0.19478579, 0.75655639),
	vec3(-0.06590893, -0.35829729, 0.93127819),
	vec3(-0.73172439, -0.32697947, 0.59805003),
	vec3(-0.79665503, -0.54631260, 0.25861806),
	vec3(-0.49077047, -0.79799555, 0.34978200),
	vec3(0.66881283, 0.16013874, 0.72597864),
	vec3(0.28083172, -0.82150569, 0.49624787),
	vec3(-0.83698848, -0.26547377, 0.47851223),
	vec3(0.26841042, -0.12709124, 0.95488411),
	vec3(0.76857828, 0.60661267, 0.20324489),
	vec3(0.47113559, -0.64279223, 0.60402765),
	vec3(-0.61194093, -0.71949289, 0.32841783),
	vec3(0.83730626, -0.44741140, 0.31423122),
	vec3(0.68976052, 0.29939057, 0.65923873),
	vec3(0.43111384, -0.65729056, 0.61815044),
	vec3(0.36139406, 0.50921011, 0.78108859),
	vec3(0.22887078, -0.80934594, 0.54090416),
	vec3(-0.36041362, -0.77991302, 0.51170080),
	vec3(-0.61836036, 0.50771348, 0.59988123),
	vec3(-0.57649771, -0.03205226, 0.81646987)
);

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


float sigmoid(float x, float disp, float scale)
{
	return 1.f / (1 + exp(- ((x + disp) * scale) ));
}


void main()
{

	vec2 screenSize = textureSize(u_gPosition, 0).xy/2.f; //smaller rez
	vec2 noiseScale = vec2(screenSize.x/4.0, screenSize.y/4.0);
	vec2 noisePos = v_texCoords * noiseScale;

	vec3 fragPos = texture(u_gPosition, v_texCoords).xyz;

	vec3 normal    = normalize( vec3(transpose(inverse(mat3(u_view))) * 
		fromuShortToFloat(texture(u_gNormal, v_texCoords).xyz)) );

	vec3 fragPosMarch = texture(u_gPosition, v_texCoords).xyz;

	//remove sky
	if(fragPosMarch.z == 0)
	{	fragColor.rgba = vec4(0,0,0,1); return; };

	//vec3 randomVec = vec2(0,1);
	vec3 randomVec = hash123(fragPosMarch.xz);

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 


	float occlusion = 0.0;


	int begin = int((kernelSize - samplesTestSize) * abs(randomVec.x));

	for(int i = begin; i < begin + samplesTestSize; ++i)
	{
		vec3 samplePos = TBN * samples[i]; // from tangent to view-space
		//vec3 samplePos = TBN * normalize(vec3(0.5, 0.3, 0.4)); 
		samplePos = fragPos + samplePos * radius; 
		
		vec4 offset = vec4(samplePos, 1.0);
		offset = u_projection * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
		
		//if(dot(normal, normalize(offset.xyz)) > 0.02) //1.14 degrees
		{
			// get sample depth
			//float sampleDepth = vec3( u_view * vec4(texture(u_gPosition, offset.xy).xyz,1) ).z;
			float sampleDepth = texture(u_gPosition, offset.xy).z; // get depth value of kernel sample
			
			// range check & accumulate
			float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
			occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
		}

	}  

	occlusion = (occlusion / kernelSize);

	occlusion = pow(occlusion,0.9);
	//occlusion = sigmoid(occlusion, -0.5, 10);

	fragColor.rgba = vec4(occlusion,0,0,1);

	//fragColor = v_texCoords.y;
	//fragColor = normal.z;
	//fragColor = sqrt(abs(randomVec.x));

}

