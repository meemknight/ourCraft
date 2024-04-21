#pragma once
#include <gameplay/entity.h>


struct DroppedItem: public PhysicalEntity
{
	BlockType type = 0;
	unsigned char count = 0;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};


struct DroppedItemClient: public ClientEntity<DroppedItem>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};

struct DroppedItemServer : public ServerEntity<DroppedItem>
{
	float restantTime = 0;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);
};

