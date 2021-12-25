#include "renderer.h"

#define GET_UNIFORM(s, n) n = s.getUniform(#n);

float data[] =
{
	0.f, 1.f, -1.f,
	-1.f, -1.f, -1.f,
	1.f, -1.f, -1.f
};

void Renderer::create()
{
	defaultShader.loadShaderProgramFromFile(RESOURCES_PATH "defaultShader.vert", RESOURCES_PATH "defaultShader.frag");
	defaultShader.bind();

	//u_viewProjection = defaultShader.getUniform("u_viewProjection");
	GET_UNIFORM(defaultShader, u_viewProjection);

		

	glCreateBuffers(1, &vertexBuffer);
	glNamedBufferStorage(vertexBuffer, sizeof(data), data, 0);


	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

}


#undef GET_UNIFORM