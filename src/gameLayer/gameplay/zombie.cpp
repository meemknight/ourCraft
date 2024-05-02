#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>
#include <gameplay/zombie.h>
#include <multyPlayer/serverChunkStorer.h>
#include <iostream>

static const auto frontHands = glm::rotate(glm::radians(90.f), glm::vec3{1.f,0.f,0.f});

void animatePlayerHandsZombie(glm::mat4 *poseVector, float &currentAngle)
{



	auto a = glm::quatLookAt(glm::normalize(glm::vec3(-sin(currentAngle), cosf(currentAngle), 4)), glm::vec3(0, 1, 0));
	auto b = glm::quatLookAt(glm::normalize(glm::vec3(-sin(currentAngle + 10), cosf(currentAngle + 10), 4)), glm::vec3(0, 1, 0));

	poseVector[2] = poseVector[2] * frontHands * glm::rotate(cosf(currentAngle) * 0.02f, glm::vec3{1.f,0.f,0.f}) * glm::rotate(cosf(currentAngle + 5) * 0.05f, glm::vec3{0.f,0.f,1.f});
	poseVector[3] = poseVector[3] * frontHands * glm::rotate(cosf(currentAngle + 10) * 0.02f, glm::vec3{1.f,0.f,0.f}) * glm::rotate(cosf(currentAngle + 15) * 0.05f, glm::vec3{0.f,0.f,1.f});

}

void Zombie::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{

	updateForces(deltaTime, true);

	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, {0.8, 1.8, 0.8});

}

void ZombieClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	currentHandsAngle += deltaTime;
	if (currentHandsAngle > glm::radians(360.f))
	{
		currentHandsAngle -= glm::radians(360.f);
	}

	entity.update(deltaTime, chunkGetter);
}

void ZombieClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{
	animatePlayerHandsZombie(skinningMatrix, currentHandsAngle);
}


void ZombieServer::appendDataToDisk(std::ofstream &f, std::uint64_t eId)
{
}

bool ZombieServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted)
{

	//waitTime -= deltaTime;
	//
	//if (waitTime < 0)
	//{
	//	moving = getRandomNumber(rng, 0, 1);
	//	waitTime += getRandomNumberFloat(rng, 1, 8);
	//
	//	if (moving)
	//	{
	//		direction = getRandomUnitVector(rng);
	//	}
	//}
	//
	//if (moving)
	//{
	//	auto move = 1.f * deltaTime * direction;
	//	getPosition().x += move.x;
	//	getPosition().z += move.y;
	//}

	getPosition().x += 4 * deltaTime;

	entity.update(deltaTime, chunkGetter);

	return true;
}

