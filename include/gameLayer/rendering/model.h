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

	std::int8_t headArmourIndex = -1;
	std::int8_t bodyArmourIndex = -1;
	std::int8_t rLegArmourIndex = -1;
	std::int8_t lLefArmourIndex = -1;
	std::int8_t rArmArmourIndex = -1;
	std::int8_t lArmArmourIndex = -1;



	void cleanup();
};

struct BoneTransform
{
	glm::quat rotation = {0,0,0,1};
	glm::vec3 position = {};

	glm::mat4 getPoseMatrix();

	bool goTowards(BoneTransform &other, float speed);
};



struct BlockModel
{
	std::vector<float> vertices;
	std::vector<float> uvs;

	glm::vec3 minPos = {};
	glm::vec3 maxPos = {};

	glm::vec3 getDimensions() { return maxPos - minPos; }

	void cleanup() { *this = {}; }
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

	Model trainingDummy;


	enum BlockModels
	{
		chairModel,
		mugModel,
		gobletModel,
		wineBottleModel,
		skullModel,
		skullTorchModel,
		booksModel,
		candleHolderModel,
		potModel,
		jarModel,
		globeModel,
		keg,
		workBenchModel,
		tableModel,
		workItemsModel,
		chairBigModel,
		cookingPotModel,
		chickenCaracasModel,
		chickenWingsPlateModel,
		fishPlateModel,
		ladderModel,
		vinesModel,
		smallRockModel,
		chestModel,
		crateModel,
		torchModel,
		torchHolderModel,
		lampModel,
		lampHolderModel,
		slabModel,
		stairsModel,
		wallModel,
		trainingDummyBaseModel,
		targetModel,

		BLOCK_MODELS_COUNT


	};

	BlockModel blockModels[BLOCK_MODELS_COUNT];


	enum TexturesLoaded
	{
		DefaultTexture,
		SteveTexture,
		ZombieTexture,
		PigTexture,
		CatTexture,
		GoblinTexture,
		TrainingDummyTexture,
		HelmetTestTexture,
	};

	std::vector<GLuint64> gpuIds;
	std::vector<GLuint> texturesIds;

	GLuint texturesSSBO = 0;

	gl2d::Texture temporaryPlayerHandTexture = {};
	GLuint64 temporaryPlayerHandBindlessTexture = 0;

	void setupSSBO();
};


void animatePlayerLegs(glm::mat4 *poseVector, float &currentAngle, int &direction, float deltaTime);

gl2d::Texture loadPlayerSkin(const char *path);

constexpr static int PLAYER_SKIN_SIZE = 128;

int getDefaultBlockShapeForFurniture(unsigned int b);
