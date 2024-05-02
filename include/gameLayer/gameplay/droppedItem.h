#pragma once
#include <gameplay/entity.h>


struct DroppedItem: public PhysicalEntity
{
	BlockType type = 0;
	unsigned char count = 0;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};


struct DroppedItemClient: public ClientEntity<DroppedItem, DroppedItemClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);
};

struct DroppedItemServer : public ServerEntity<DroppedItem>
{
	float restantTime = 0;

	float stayTimer = 5;

	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted);

	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);

	bool loadFromDisk(std::ifstream &f);
};

