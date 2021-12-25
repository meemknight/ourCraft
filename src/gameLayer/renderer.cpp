#include "renderer.h"

#define GET_UNIFORM(s, n) n = s.getUniform(#n);



int data[] =
{
	mergeShorts(0, 1),0,0,0,
	mergeShorts(1, 1),0,0,0,
	mergeShorts(2, 1),0,0,0,
	mergeShorts(3, 1),0,0,0,
	mergeShorts(4, 1),0,0,0,
	mergeShorts(5, 1),0,0,0,

	mergeShorts(0, 2),2,0,0,
	mergeShorts(1, 2),2,0,0,
	mergeShorts(2, 2),2,0,0,
	mergeShorts(3, 2),2,0,0,
	mergeShorts(4, 2),2,0,0,
	mergeShorts(5, 2),2,0,0,

	mergeShorts(0, 3),-2,0,0,
	mergeShorts(1, 3),-2,0,0,
	mergeShorts(2, 3),-2,0,0,
	mergeShorts(3, 3),-2,0,0,
	mergeShorts(4, 3),-2,0,0,
	mergeShorts(5, 3),-2,0,0,

	mergeShorts(0, 4),0,0,-2,
	mergeShorts(1, 4),0,0,-2,
	mergeShorts(2, 4),0,0,-2,
	mergeShorts(3, 4),0,0,-2,
	mergeShorts(4, 4),0,0,-2,
	mergeShorts(5, 4),0,0,-2,
};

//data format:

// short orientation
// short type
// int x
// int y
// int z


void Renderer::create()
{
	defaultShader.loadShaderProgramFromFile(RESOURCES_PATH "defaultShader.vert", RESOURCES_PATH "defaultShader.frag");
	defaultShader.bind();

	GET_UNIFORM(defaultShader, u_viewProjection);
	GET_UNIFORM(defaultShader, u_position);
	GET_UNIFORM(defaultShader, u_positionInt);
	GET_UNIFORM(defaultShader, u_positionFloat);
	GET_UNIFORM(defaultShader, u_texture);

	glCreateBuffers(1, &vertexBuffer);
	glNamedBufferData(vertexBuffer, sizeof(data), data, GL_DYNAMIC_DRAW);


	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_SHORT, 4*sizeof(int), 0);
		glVertexAttribDivisor(0, 1);

		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 1, GL_SHORT, 4 * sizeof(int), (void*)(1 * sizeof(short)));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_INT, 4 * sizeof(int), (void*)(1 * sizeof(int)));
		glVertexAttribDivisor(2, 1);

	glBindVertexArray(0);

}


#undef GET_UNIFORM