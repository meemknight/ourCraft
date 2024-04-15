#pragma once
#include <gameplay/entity.h>


struct Zombie: public PhysicalEntity
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};


struct ZombieClient: public ClientEntity<Zombie>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};

struct ZombieServer: public ServerEntity<Zombie>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};

