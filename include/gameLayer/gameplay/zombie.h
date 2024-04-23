#pragma once
#include <gameplay/entity.h>
#include <random>


struct Zombie: public PhysicalEntity
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};


struct ZombieClient: public ClientEntity<Zombie, ZombieClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};

struct ZombieServer: public ServerEntity<Zombie>
{

	glm::vec2 direction = {};
	int moving = 0;
	float waitTime = 1;



	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);
};

