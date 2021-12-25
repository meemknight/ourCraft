#version 330 core

layout(location = 0) in int in_faceOrientation;
layout(location = 1) in int in_faceType;
layout(location = 2) in ivec3 in_facePosition;

uniform mat4 u_viewProjection;
uniform ivec3 u_positionInt;
uniform vec3 u_positionFloat;

float vertexData[] = float[](
		//front
		0.5, 0.5, 0.5,
		-0.5, 0.5, 0.5,
		-0.5, -0.5, 0.5,
		-0.5, -0.5, 0.5,
		0.5, -0.5, 0.5,
		0.5, 0.5, 0.5,

		//back
		-0.5, -0.5, -0.5,
		-0.5, 0.5, -0.5,
		0.5, 0.5, -0.5,
		0.5, 0.5, -0.5,
		0.5, -0.5, -0.5,
		-0.5, -0.5, -0.5,

		//top
		-0.5, 0.5, -0.5,
		-0.5, 0.5, 0.5,
		0.5, 0.5, 0.5,
		0.5, 0.5, 0.5,
		0.5, 0.5, -0.5,
		-0.5, 0.5, -0.5,

		//bottom
		0.5, -0.5, 0.5,
		-0.5, -0.5, 0.5,
		-0.5, -0.5, -0.5,
		-0.5, -0.5, -0.5,
		0.5, -0.5, -0.5,
		0.5, -0.5, 0.5,
	
		//left
		-0.5, -0.5, 0.5,
		-0.5, 0.5, 0.5,
		-0.5, 0.5, -0.5,
		-0.5, 0.5, -0.5,
		-0.5, -0.5, -0.5,
		-0.5, -0.5, 0.5,
	
		//right
		0.5, 0.5, -0.5,
		0.5, 0.5, 0.5,
		0.5, -0.5, 0.5,
		0.5, -0.5, 0.5,
		0.5, -0.5, -0.5,
		0.5, 0.5, -0.5
);

float vertexUV[] = float[](
		//front
		1,1,
		0,1,
		0,0,
		0,0,
		1,0,
		1,1,

		//back
		0,0,
		0,1,
		1,1,
		1,1,
		1,0,
		0,0,

		//bottom
		1,1,
		0,1,
		0,0,
		0,0,
		1,0,
		1,1,

		//top
		1,1,
		0,1,
		0,0,
		0,0,
		1,0,
		1,1,

		//left
		1,0,
		1,1,
		0,1,
		0,1,
		0,0,
		1,0,

		//right
		1,1,
		0,1,
		0,0,
		0,0,
		1,0,
		1,1
);

const int typesCount = 5;

int atlasPositions[] = int[](
	//front
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	
	//back
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice

	//top
	0,0,
	0, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice

	//bottom
	0, 0,
	2, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice

	//left
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	
	//right
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11 //ice

);


out vec2 v_uv;

void main()
{
	ivec3 intPosition = in_facePosition - u_positionInt;
	vec3 floatPosition = intPosition - u_positionFloat;
	
	vec4 pos = vec4(floatPosition.xyz,1);
	pos.x += vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 0];
	pos.y += vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 1];
	pos.z += vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 2];

	pos = u_viewProjection * pos;

	gl_Position = pos;
		
	v_uv.x = vertexUV[in_faceOrientation * 2 * 6 + gl_VertexID * 2 + 0];
	v_uv.y = vertexUV[in_faceOrientation * 2 * 6 + gl_VertexID * 2 + 1];

	ivec2 uvInAtlas;
	uvInAtlas.x = atlasPositions[in_faceOrientation * 2 * typesCount + in_faceType * 2 + 0];
	uvInAtlas.y = atlasPositions[in_faceOrientation * 2 * typesCount + in_faceType * 2 + 1];

	v_uv += uvInAtlas;
	v_uv *= 1.f/16.f;

}