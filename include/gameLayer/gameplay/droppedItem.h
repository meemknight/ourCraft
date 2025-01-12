#pragma once
#include <gameplay/entity.h>
#include <gameplay/items.h>


struct DroppedItem: public PhysicalEntity
{
	unsigned short type = 0;
	unsigned char count = 0;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);


	static glm::vec3 getMaxColliderSize();
	glm::vec3 getColliderSize();

};


struct DroppedItemClient: public ClientEntity<DroppedItem, DroppedItemClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);
};

struct DroppedItemServer
{

	static glm::vec3 getMaxColliderSize();
	glm::vec3 getColliderSize();

	DroppedItem getDataToSend();

	PhysicalEntity entity;
	Item item;

	glm::dvec3 &getPosition()
	{
		return entity.position;
	}

	void applyHitForce(glm::vec3 force) {};

	float restantTime = 0;

	float stayTimer = 5 * 60;
	float dontPickTimer = 1;

	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition);

	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);

	bool loadFromDisk(std::ifstream &f);

	bool hasUpdatedThisTick = 0;

	glm::ivec2 lastChunkPositionWhenAnUpdateWasSent = {};

};

