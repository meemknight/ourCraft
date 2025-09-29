#pragma once
#include <glad/glad.h>
#include <vector>
#include <gl2d/gl2d.h>


struct KeyFrame
{
	float timestamp = 0;

	glm::vec3 pos = {};
	glm::vec3 scale = {1,1,1};
	glm::quat rotation = {0,0,0,1};

	glm::mat4 getMatrix() const
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 rotationMat = glm::mat4_cast(rotation);
		//glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));
		return translation * rotationMat * scaleMat;
	}

};

inline KeyFrame interpolateKeyFrames(const KeyFrame &kf1, const KeyFrame &kf2, float time)
{
	if (kf1.timestamp == kf2.timestamp) return kf1; // No blending needed

	float t = (time - kf1.timestamp) / (kf2.timestamp - kf1.timestamp);
	t = glm::clamp(t, 0.0f, 1.0f);

	KeyFrame result;
	result.timestamp = time;
	result.pos = glm::mix(kf1.pos, kf2.pos, t);
	result.scale = glm::mix(kf1.scale, kf2.scale, t);
	result.rotation = glm::slerp(kf1.rotation, kf2.rotation, t);

	return result;
}


struct Animation
{
	float animationLength = 0;
	std::vector<std::vector<KeyFrame>> kayFrames;

	enum AnimationType
	{
		none,
		idle,
		running,
		falling,
		meleHit,

		ANIMATIONS_COUNT
	};
};


//
struct AnimationStateClient
{

	int currentAnimation = 0;

	float attackTimer = 0;
	bool isAttacking = false;

	void signalAttack()
	{
		if (!isAttacking)
		{
			attackTimer = 0;
			isAttacking = true;
		}
	}

	void setAnimation(int type)
	{
		currentAnimation = type;
	};

	float animationTime = 0;

	void update(float deltaTime) { animationTime += deltaTime; if (isAttacking) { attackTimer += deltaTime; } };
};

struct AnimationStateServer
{
	float runningTime = 0;
	bool attacked = 0;
	
	void update(float deltaTime) { runningTime -= deltaTime; runningTime = std::max(runningTime, 0.f); };
};

//used to be inharented by an entity
struct Animatable
{
	AnimationStateServer animationStateServer;
};


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

	std::vector<Animation> animations;
	std::int8_t animationsIndex[Animation::ANIMATIONS_COUNT] = {-1};
		

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

//modelsloader modelloader
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

	Model scareCrow;


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
		furnaceModel,
		goblinWorkBenchModel,
		goblinChairModel,
		goblinTableModel,
		goblinStitchingPostModel,
		fence,

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
		scareCrowTexture,
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
