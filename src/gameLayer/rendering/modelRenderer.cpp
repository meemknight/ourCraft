#include "rendering/modelRenderer.h"
#include <iostream>

void ModelRenderer::create()
{

	shader.loadShaderProgramFromFile(RESOURCES_PATH "model.vert", RESOURCES_PATH "model.frag");

}

void ModelRenderer::render(Model &m)
{
	shader.bind();

	glBindVertexArray(m.vao);

	if (m.indexDataBuffer)
	{
		glDrawElements(GL_TRIANGLES, m.primitivesCount, GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, m.primitivesCount);
	}

	glBindVertexArray(0);

}


void Model::loadFromComputedData(size_t vertexSize, const float *vercies, size_t indexSize, const unsigned int *indexes)
{

	if (indexSize % 3 != 0)
	{
		std::cout << "error loading vertex data, index was not multiple of 3";
		return;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexSize, vercies, GL_STATIC_DRAW);

	//if (noTexture)
	//{
	//	glEnableVertexAttribArray(0);
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	//	glEnableVertexAttribArray(1);
	//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	//}
	//else
	{
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

	}

	if (indexSize && indexes)
	{
		glGenBuffers(1, &indexDataBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexDataBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexes, GL_STATIC_DRAW);

		primitivesCount = indexSize / sizeof(*indexes);

	}
	else
	{
		primitivesCount = vertexSize / sizeof(float);
	}

	glBindVertexArray(0);

}
