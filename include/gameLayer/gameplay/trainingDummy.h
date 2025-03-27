#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <random>
#include <unordered_map>
#include <unordered_set>


struct TrainingDummy: public CanPushOthers,
	public CanBeAttacked, public PositionBasedID, public NotSyncronizedEntity
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

	glm::vec3 getColliderSize();

	glm::vec3 getColliderOffset();

	glm::vec3 getMaxColliderSize();

	Armour getArmour() { return {1}; };
};


struct TrainingDummyClient: public ClientEntity<TrainingDummy, TrainingDummyClient>
{
	glm::dvec3 position = {};


	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);

	int getTextureIndex();
};

struct TrainingDummyServer: public ServerEntity<TrainingDummy>
{
	glm::dvec3 position = {};


	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition,
		std::unordered_map < std::uint64_t, Client *> &allClients);

};


