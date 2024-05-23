#pragma once
#include <gameplay/entity.h>
#include <random>
#include <gameplay/ai.h>



struct Cat: public PhysicalEntity, public HasOrientationAndHeadTurnDirection,
	public MovementSpeedForLegsAnimations, public CanPushOthers
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

	glm::vec3 getColliderSize();

	static glm::vec3 getMaxColliderSize();
};


struct CatClient: public ClientEntity<Cat, CatClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);

};



struct CatServer: public ServerEntity<Cat>,
	public AnimalBehaviour < CatServer, CatDefaultSettings >
{
	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition
	);

	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);

	void configureSpawnSettings(std::minstd_rand &rng);

};






