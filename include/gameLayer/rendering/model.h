#pragma once
#include <glad/glad.h>
#include <vector>
#include <gl2d/gl2d.h>




struct Model
{

	GLuint geometry = 0;
	GLuint indexBuffer = 0;
	GLuint vao = 0;
	size_t vertexCount = 0;
	std::vector<glm::mat4> transforms;

};




struct ModelsManager
{


	void loadAllModels();

	Model human;

	gl2d::Texture steveTexture;
	GLuint64 steveTextureHandle;
};