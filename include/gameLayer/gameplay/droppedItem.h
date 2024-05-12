#pragma once
#include <gameplay/entity.h>
#include <gameplay/items.h>


struct DroppedItem: public PhysicalEntity
{
	unsigned short type = 0;
	unsigned char count = 0;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};


struct DroppedItemClient: public ClientEntity<DroppedItem, DroppedItemClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);
};

struct DroppedItemServer
{

	DroppedItem getDataToSend();

	PhysicalEntity entity;
	Item item;

	glm::dvec3 &getPosition()
	{
		return entity.position;
	}

	float restantTime = 0;

	float stayTimer = 5 * 60;

	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition);

	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);

	bool loadFromDisk(std::ifstream &f);
};

