#version 430 core

layout(location = 0) in int in_faceOrientation; //up down left etc
layout(location = 1) in int in_textureIndex; //dirt grass stone etc
layout(location = 2) in ivec3 in_facePosition; // int x y z
layout(location = 3) in int in_skyLight; 


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
out float v_color;
out flat int v_skyLight;

out flat uvec2 v_textureSampler;

out flat ivec3 fragmentPositionI;
out vec3 fragmentPositionF;

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

	vec3 pos = fragmentPositionF.xyz;
	vec3 vertexShape;


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

	int ambientLight = max(in_skyLight - (15 - u_skyLightIntensity), 0);

	v_skyLight = ambientLight;

	fragmentPositionI = in_facePosition - u_positionInt;
	fragmentPositionF = fragmentPositionI - u_positionFloat;
	
	vec4 pos = vec4(calculateVertexPos(gl_VertexID),1);
	
	v_uv = calculateUVs(gl_VertexID);

	//calculate normals	
	{
		vec3 pos1;
		vec3 pos2;
		vec3 pos3;
	
		if(gl_VertexID == 0 || gl_VertexID == 3)
		{
			pos1 = pos.xyz;
			pos2 = calculateVertexPos(gl_VertexID + 1);
			pos3 = calculateVertexPos(gl_VertexID + 2);
	
			
		}else if(gl_VertexID == 1 || gl_VertexID == 4)
		{
			pos1 = calculateVertexPos(gl_VertexID - 1);
			pos2 = pos.xyz;
			pos3 = calculateVertexPos(gl_VertexID + 1);
		}else if(gl_VertexID == 2 || gl_VertexID == 5)
		{
			pos1 = calculateVertexPos(gl_VertexID - 2);
			pos2 = calculateVertexPos(gl_VertexID - 1);
			pos3 = pos.xyz;
		}
		
		vec3 a = (pos3-pos2);
		vec3 b = (pos1-pos2);
		v_normal = normalize(cross(a, b));
	}


	pos = u_viewProjection * pos;

	gl_Position = pos;
	
	v_color = (vertexColor[in_faceOrientation] * (ambientLight/15.f)) * 0.7 + 0.3;
	//v_color = vertexColor[in_faceOrientation];



	v_textureSampler = textureSamplerers[in_textureIndex];

}