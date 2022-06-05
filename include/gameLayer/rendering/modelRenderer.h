#pragma once

#include "rendering/shader.h"
#include <gl2d/gl2d.h>

struct Model
{
	GLuint vao = 0;
	GLuint vertexDataBuffer = 0;
	GLuint indexDataBuffer = 0;
	int primitivesCount = 0;

	gl2d::Texture t;

	void loadFromComputedData(size_t vertexSize, const float *vercies, 
		size_t indexSize = 0, const unsigned int *indexes = nullptr);


};


struct ModelRenderer
{

	Shader shader;

	void create();
	void render(Model &m);

};