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

	Model pig;

	Model rightHand;

	enum TexturesLoaded
	{
		DefaultTexture,
		SteveTexture,
		ZombieTexture,
		PigTexture,
	};

	std::vector<GLuint64> gpuIds;
	std::vector<GLuint> texturesIds;

	GLuint texturesSSBO = 0;


	void setupSSBO();
};


void animatePlayerLegs(glm::mat4 *poseVector, float &currentAngle, int &direction, float deltaTime);
