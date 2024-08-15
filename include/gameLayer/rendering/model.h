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

	void cleanup();
};

struct BoneTransform
{
	glm::quat rotation = {0,0,0,1};
	glm::vec3 position = {};

	glm::mat4 getPoseMatrix();

	bool goTowards(BoneTransform &other, float speed);
};


struct ModelsManager
{

	void loadAllModels(std::string path, bool reportErrors);
	void clearAllModels();

	Model human;

	Model pig;

	Model cat;

	Model rightHand;

	enum TexturesLoaded
	{
		DefaultTexture,
		SteveTexture,
		ZombieTexture,
		PigTexture,
		CatTexture
	};

	std::vector<GLuint64> gpuIds;
	std::vector<GLuint> texturesIds;

	GLuint texturesSSBO = 0;


	void setupSSBO();
};


void animatePlayerLegs(glm::mat4 *poseVector, float &currentAngle, int &direction, float deltaTime);

gl2d::Texture loadPlayerSkin(const char *path);

constexpr static int PLAYER_SKIN_SIZE = 64;