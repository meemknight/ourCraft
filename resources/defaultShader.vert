#version 430 core

layout(location = 0) in int in_faceOrientation; //up down left etc
layout(location = 1) in int in_faceType; //dirt grass stone etc
layout(location = 2) in ivec3 in_facePosition; // int x y z

uniform mat4 u_viewProjection;
uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;
uniform float u_time;

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



readonly restrict layout(std430) buffer u_atlasPositions
{
	int atlasPositions[];
};
uniform int u_typesCount;

readonly restrict layout(std430) buffer u_vertexData
{
	float vertexData[];
};

readonly restrict layout(std430) buffer u_vertexUV
{
	float vertexUV[];
};


out vec2 v_uv;
out float v_color;

void main()
{
	ivec3 intPosition = in_facePosition - u_positionInt;
	vec3 floatPosition = intPosition - u_positionFloat;
	
	vec4 pos = vec4(floatPosition.xyz,1);
	vec3 vertexShape;

	vertexShape.x = vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 0];
	vertexShape.y = vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 1];
	vertexShape.z = vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 2];

	v_uv.x = vertexUV[(in_faceOrientation) * 2 * 6 + gl_VertexID * 2 + 0];
	v_uv.y = vertexUV[(in_faceOrientation) * 2 * 6 + gl_VertexID * 2 + 1];

	if(in_faceOrientation >= 10)
	{

		if(in_facePosition.y % 2 == 0)
		{
			vertexShape.x = -vertexShape.x;
			vertexShape.z = -vertexShape.z;
			v_uv.x = 1.f-v_uv.x;
		}

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

	pos = u_viewProjection * pos;

	gl_Position = pos;
	
	v_color = vertexColor[in_faceOrientation];


	ivec2 uvInAtlas;
	uvInAtlas.x = atlasPositions[(in_faceOrientation) * 2 * u_typesCount + in_faceType * 2 + 0];
	uvInAtlas.y = atlasPositions[(in_faceOrientation) * 2 * u_typesCount + in_faceType * 2 + 1];

	v_uv += uvInAtlas;
	v_uv *= 1.f/16.f;

}