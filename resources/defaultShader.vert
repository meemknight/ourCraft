#version 330 core

layout(location = 0) in int in_faceOrientation;
layout(location = 1) in ivec3 in_facePosition;

uniform mat4 u_viewProjection;

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


void main()
{
	
	vec4 pos = vec4(in_facePosition.xyz,1);
	pos.x += vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 0];
	pos.y += vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 1];
	pos.z += vertexData[in_faceOrientation * 3 * 6 + gl_VertexID * 3 + 2];

	pos = u_viewProjection * pos;

	gl_Position = pos;

}