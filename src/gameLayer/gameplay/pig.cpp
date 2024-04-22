#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <gameplay/pig.h>
#include <iostream>
#include <glm/glm.hpp>


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

	skinningMatrix[0] = skinningMatrix[0] * glm::toMat4(
		glm::quatLookAt(glm::normalize(getRubberBandLookDirection()), glm::vec3(0, 1, 0)));


	skinningMatrix[2] = skinningMatrix[2] * glm::rotate(getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[3] = skinningMatrix[3] * glm::rotate(-getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[4] = skinningMatrix[4] * glm::rotate(getLegsAngle(), glm::vec3{1,0,0});
	skinningMatrix[5] = skinningMatrix[5] * glm::rotate(-getLegsAngle(), glm::vec3{1,0,0});

}



void PigServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng)
{


	waitTime -= deltaTime;

	if (waitTime < 0)
	{
		moving = getRandomNumber(rng, 0, 100)%2;
		waitTime = getRandomNumberFloat(rng, 1, 4);

		if (moving)
		{
			direction = getRandomUnitVector(rng);
		}
	}

	if (moving)
	{

		float moveSpeed = 1;

		auto move = moveSpeed * deltaTime * direction;
		getPosition().x += move.x;
		getPosition().z += move.y;

		entity.bodyOrientation = move;
		entity.lookDirectionAnimation;
		entity.movementSpeedForLegsAnimations = 2.f * moveSpeed;
	}
	else
	{
		entity.movementSpeedForLegsAnimations = 0.f;
	}


	entity.update(deltaTime, chunkGetter);

}
