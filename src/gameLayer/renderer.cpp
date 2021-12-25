#include "renderer.h"

#define GET_UNIFORM(s, n) n = s.getUniform(#n);

int data[] =
{
	0,0,0,0,
	1,0,0,0,
	2,0,0,0,
	3,0,0,0,
	4,0,0,0,
	5,0,0,0,

	0,2,0,0,
	1,2,0,0,
	2,2,0,0,
	3,2,0,0,
	4,2,0,0,
	5,2,0,0,
};

//data format:

// int orientation
// int x
// int y
// int z


void Renderer::create()
{
	defaultShader.loadShaderProgramFromFile(RESOURCES_PATH "defaultShader.vert", RESOURCES_PATH "defaultShader.frag");
	defaultShader.bind();

	//u_viewProjection = defaultShader.getUniform("u_viewProjection");
	GET_UNIFORM(defaultShader, u_viewProjection);
	GET_UNIFORM(defaultShader, u_position);


	glCreateBuffers(1, &vertexBuffer);
	glNamedBufferStorage(vertexBuffer, sizeof(data), data, 0);


	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_INT, 4*sizeof(int), 0);
		glVertexAttribDivisor(0, 1);

		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 3, GL_INT, 4 * sizeof(int), (void*)(1 * sizeof(int)));
		glVertexAttribDivisor(1, 1);

	glBindVertexArray(0);

}


#undef GET_UNIFORM