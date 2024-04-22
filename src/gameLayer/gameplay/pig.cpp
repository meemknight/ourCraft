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

glm::ivec2 checkOffsets[9] = {
	glm::ivec2(0,0),
	glm::ivec2(1,0),
	glm::ivec2(-1,0),
	glm::ivec2(0,1),
	glm::ivec2(0,-1),
	glm::ivec2(1,-1),
	glm::ivec2(-1,-1),
	glm::ivec2(1,1),
	glm::ivec2(-1,1),
};

static thread_local std::vector<std::uint64_t> playersClose;

bool checkPlayerDistance(glm::dvec3 a, glm::dvec3 b)
{
	return glm::length(a - b) <= 15.f;
}

void PigServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng)
{

	updateAnimalBehaviour(deltaTime, chunkGetter, serverChunkStorer, rng);


	entity.update(deltaTime, chunkGetter);

}
