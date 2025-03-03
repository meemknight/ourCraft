#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <gameplay/cat.h>
#include <iostream>
#include <glm/glm.hpp>
#include <multyPlayer/tick.h>
#include <chunkSystem.h>
#include <rendering/model.h>

void Cat::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	PhysicalSettings ps;
	ps.gravityModifier = 0.9;
	updateForces(deltaTime, true, ps);

	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, getColliderSize());

}

glm::vec3 Cat::getColliderSize()
{
	return getMaxColliderSize();
}

glm::vec3 Cat::getMaxColliderSize()
{
	return glm::vec3(0.5, 0.6, 0.5);
}

void CatClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	entityBuffered.update(deltaTime, chunkGetter);

}

void CatClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{



	skinningMatrix[2] = skinningMatrix[2] * glm::rotate(getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[3] = skinningMatrix[3] * glm::rotate(-getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[4] = skinningMatrix[4] * glm::rotate(getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[5] = skinningMatrix[5] * glm::rotate(-getLegsAngle(), glm::vec3{1,0,0});

}


int CatClient::getTextureIndex()
{
	return ModelsManager::TexturesLoaded::CatTexture;
}


bool CatServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding
	, std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition,
	std::unordered_map < std::uint64_t, Client *> &allClients
)
{

	updateAnimalBehaviour(deltaTime, chunkGetter, serverChunkStorer, rng);


	doCollisionWithOthers(getPosition(), entity.getMaxColliderSize(), entity.forces,
		serverChunkStorer, yourEID);

	entity.update(deltaTime, chunkGetter);

	return true;
}

void CatServer::appendDataToDisk(std::ofstream &f, std::uint64_t eId)
{
}

void CatServer::configureSpawnSettings(std::minstd_rand &rng)
{
	AnimalBehaviour::configureSpawnSettings(rng);
}
