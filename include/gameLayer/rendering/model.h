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

	std::int8_t headIndex = -1;
	std::int8_t bodyIndex = -1;
	std::int8_t rLegIndex = -1;
	std::int8_t lLefIndex = -1;
	std::int8_t rArmIndex = -1;
	std::int8_t lArmIndex = -1;
	std::int8_t pupilsIndex = -1;
	std::int8_t lEyeIndex = -1;
	std::int8_t rEyeIndex = -1;

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

	Model goblin;

	enum TexturesLoaded
	{
		DefaultTexture,
		SteveTexture,
		ZombieTexture,
		PigTexture,
		CatTexture,
		GoblinTexture
	};

	std::vector<GLuint64> gpuIds;
	std::vector<GLuint> texturesIds;

	GLuint texturesSSBO = 0;


	void setupSSBO();
};


void animatePlayerLegs(glm::mat4 *poseVector, float &currentAngle, int &direction, float deltaTime);

gl2d::Texture loadPlayerSkin(const char *path);

constexpr static int PLAYER_SKIN_SIZE = 128;