#version 430 core

layout(location = 0) in int in_faceOrientation; //up down left etc
layout(location = 1) in int in_textureIndex; //dirt grass stone etc
layout(location = 2) in ivec3 in_facePosition; // int x y z
layout(location = 3) in int in_skyAndNormalLights; 


uniform mat4 u_viewProjection;
uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;
uniform float u_time;
uniform int u_skyLightIntensity;

float vertexColor[] = float[](
		
	//front
	0.9,
	//back
	0.8,
	//top
	1.0,
	//bottom
	0.7,
	//left
	0.8,
	//right
	0.9,

	//grass
	0.9,
	0.9,
	0.9,
	0.9,

	// leaves
	//front
	0.9,
	//back
	0.8,
	//top
	1.0,
	//bottom
	0.7,
	//left
	0.8,
	//right
	0.9


);

//geometry
readonly restrict layout(std430) buffer u_vertexData
{
	float vertexData[];
};

readonly restrict layout(std430) buffer u_vertexUV
{
	float vertexUV[];
};

readonly restrict layout(std430) buffer u_textureSamplerers
{
	uvec2 textureSamplerers[];
};


out vec2 v_uv;
out float v_ambient;
out flat int v_ambientInt;

out flat uvec2 v_textureSampler;
out flat uvec2 v_normalSampler;
out flat uvec2 v_materialSampler;

out flat ivec3 fragmentPositionI;
out vec3 fragmentPositionF;

out flat int v_skyLight;
out flat int v_normalLight;

out flat vec3 v_normal;

vec2 calculateUVs(int vertexId)
{	
	vec2 uvs;
	uvs.x = vertexUV[(in_faceOrientation) * 2 * 6 + vertexId * 2 + 0];
	uvs.y = vertexUV[(in_faceOrientation) * 2 * 6 + vertexId * 2 + 1];
	return uvs;
}

vec3 calculateVertexPos(int vertexId)
{

	//vec3 pos = fragmentPositionF.xyz;
	vec3 pos = vec3(0);
	vec3 vertexShape = vec3(0);


	vertexShape.x = vertexData[in_faceOrientation * 3 * 6 + vertexId * 3 + 0];
	vertexShape.y = vertexData[in_faceOrientation * 3 * 6 + vertexId * 3 + 1];
	vertexShape.z = vertexData[in_faceOrientation * 3 * 6 + vertexId * 3 + 2];


	if(in_faceOrientation >= 10) //animated
	{

		if(in_facePosition.y % 2 == 0)
		{
			vertexShape.x = -vertexShape.x;
			vertexShape.z = -vertexShape.z;
			//v_uv.x = 1.f-v_uv.x;
		}

		vertexShape.x += vertexData[(in_faceOrientation-10) * 3 * 6 + vertexId * 3 + 0];
		vertexShape.y += vertexData[(in_faceOrientation-10) * 3 * 6 + vertexId * 3 + 1];
		vertexShape.z += vertexData[(in_faceOrientation-10) * 3 * 6 + vertexId * 3 + 2];

		//if(in_faceOrientation == 6)
		//{//front
		//
		//}else if(in_faceOrientation == 7)
		//{//back
		//
		//}else if(in_faceOrientation == 8)
		//{//top
		//
		//}else if(in_faceOrientation == 9)
		//{//bottom
		//
		//}else if(in_faceOrientation == 10)
		//{//left
		//
		//}else if(in_faceOrientation == 11)
		//{//right
		//
		//}
		//vertexShape *= 0.8+((sin(u_time)+1)/2.f)*0.2;
	}

	pos.xyz += vertexShape;

	return pos;
}


void main()
{

	v_skyLight = (in_skyAndNormalLights & 0xf0) >> 4;
	v_normalLight = (in_skyAndNormalLights & 0xf);

	v_skyLight = max(v_skyLight - (15 - u_skyLightIntensity), 0);

	v_ambientInt = max(v_skyLight, v_normalLight);

	vec3 diffI = in_facePosition - u_positionInt;
	vec3 diffF = diffI - u_positionFloat;
	

	fragmentPositionF = calculateVertexPos(gl_VertexID);
	fragmentPositionI = in_facePosition;
	
	vec4 posView = vec4(fragmentPositionF + diffF,1);
	

	v_uv = calculateUVs(gl_VertexID);

	//calculate normals	
	{
		vec3 pos1 = vec3(0);
		vec3 pos2 = vec3(0);
		vec3 pos3 = vec3(0);
	
		if(gl_VertexID == 0 || gl_VertexID == 3)
		{
			pos1 = posView.xyz;
			pos2 = calculateVertexPos(gl_VertexID + 1) + diffF;
			pos3 = calculateVertexPos(gl_VertexID + 2) + diffF;
	
			
		}else if(gl_VertexID == 1 || gl_VertexID == 4)
		{
			pos1 = calculateVertexPos(gl_VertexID - 1) + diffF;
			pos2 = posView.xyz;
			pos3 = calculateVertexPos(gl_VertexID + 1) + diffF;
		}else if(gl_VertexID == 2 || gl_VertexID == 5)
		{
			pos1 = calculateVertexPos(gl_VertexID - 2) + diffF;
			pos2 = calculateVertexPos(gl_VertexID - 1) + diffF;
			pos3 = posView.xyz;
		}
		
		vec3 a = (pos3-pos2);
		vec3 b = (pos1-pos2);
		v_normal = normalize(cross(a, b));

	}


	posView = u_viewProjection * posView;

	gl_Position = posView;
	
	v_ambient = (vertexColor[in_faceOrientation] * (v_ambientInt/15.f)) * 0.8 + 0.2;
	//v_color = vertexColor[in_faceOrientation];



	v_textureSampler = textureSamplerers[in_textureIndex];
	v_normalSampler = textureSamplerers[in_textureIndex+1];
	v_materialSampler = textureSamplerers[in_textureIndex+2];

}