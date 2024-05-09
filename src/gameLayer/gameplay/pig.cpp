#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <gameplay/pig.h>
#include <iostream>
#include <glm/glm.hpp>
#include <multyPlayer/tick.h>
#include <chunkSystem.h>

void Pig::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	updateForces(deltaTime, true);


	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, {0.8,0.8,0.8});
}

void PigClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	entity.update(deltaTime, chunkGetter);

}

void PigClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{

	skinningMatrix[1] = skinningMatrix[1] * glm::toMat4(
		glm::quatLookAt(glm::normalize(getRubberBandLookDirection()), glm::vec3(0, 1, 0)));


	skinningMatrix[2] = skinningMatrix[2] * glm::rotate(getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[3] = skinningMatrix[3] * glm::rotate(-getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[4] = skinningMatrix[4] * glm::rotate(getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[5] = skinningMatrix[5] * glm::rotate(-getLegsAngle(), glm::vec3{1,0,0});

}



bool PigServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding
	,std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition
	)
{

	updateAnimalBehaviour(deltaTime, chunkGetter, serverChunkStorer, rng);


	entity.update(deltaTime, chunkGetter);


	return true;
}

void PigServer::appendDataToDisk(std::ofstream &f, std::uint64_t eId)
{
}

void PigServer::configureSpawnSettings(std::minstd_rand &rng)
{

	AnimalBehaviour::configureSpawnSettings(rng);


}
