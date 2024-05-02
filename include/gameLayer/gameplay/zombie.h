#pragma once
#include <gameplay/entity.h>
#include <random>


struct Zombie: public PhysicalEntity
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};


struct ZombieClient: public ClientEntity<Zombie, ZombieClient>
{
	float currentHandsAngle = 0;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);
};

struct ZombieServer: public ServerEntity<Zombie>
{

	glm::vec2 direction = {};
	int moving = 0;
	float waitTime = 1;

	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);
};

void animatePlayerHandsZombie(glm::mat4 *poseVector, float &currentAngle, float deltaTime);
