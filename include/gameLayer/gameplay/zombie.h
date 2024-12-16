#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <random>


struct Zombie: public PhysicalEntity, public CanPushOthers
	, public HasOrientationAndHeadTurnDirection, public CollidesWithPlacedBlocks,
	public CanBeKilled, public CanBeAttacked
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

	glm::vec3 getColliderSize();

	glm::vec3 getMaxColliderSize();

	Life life{20};
	
	Armour getArmour() { return {1}; };
};


struct ZombieClient: public ClientEntity<Zombie, ZombieClient>
{
	float currentHandsAngle = 0;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);

	int getTextureIndex();
};

struct ZombieServer: public ServerEntity<Zombie>
{

	glm::vec2 direction = {};
	float waitTime = 1;
	float keepJumpingTimer = 0;
	float randomSightBonusTimer = 1;


	std::uint64_t playerLockedOn = 0;

	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);

	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition);

};

void animatePlayerHandsZombie(glm::mat4 *poseVector, float &currentAngle, float deltaTime);
