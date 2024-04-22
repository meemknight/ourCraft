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
	glm::ivec2 chunkPosition = determineChunkThatIsEntityIn(getPosition());

	waitTime -= deltaTime;
	changeHeadTimer -= deltaTime;
	randomJumpTimer -= deltaTime;

	if (waitTime < 0)
	{
		moving = getRandomNumber(rng, 0, 100)%2;
		waitTime = getRandomNumberFloat(rng, 1, 4);

		if (moving)
		{
			direction = getRandomUnitVector(rng);
			moveSpeed = getRandomNumberFloat(rng, 0.9, 1.5);
		}
	}

	if (moving)
	{

		//check falling

		auto pos = getPosition();
		pos.x += direction.x;
		pos.z += direction.y;
		pos.y += 0.6;

		auto chunkPos = determineChunkThatIsEntityIn(pos);
		auto c = chunkGetter(chunkPos);
		if (c)
		{
			auto blockPos = fromBlockPosToBlockPosInChunk(glm::ivec3(pos));
			auto b = c->safeGet(blockPos);

			if (b && b->isColidable())
			{
				blockPos.y++;
				auto b = c->safeGet(blockPos);
				if (b && b->isColidable())
				{
					//wall
					direction = {};
					waitTime = 0;
				}
				else
				{
					if (getRandomNumber(rng, 0, 10) % 2)
					{
						changeHeadTimer = getRandomNumberFloat(rng, 0.5, 1.5); //look up
						entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.5,-1}, 0.2);
						playerFollow = 0;
					}

					entity.forces.jump();
				}

			}
			else if(b)
			{

				//block under
				blockPos.y--;
				auto b = c->safeGet(blockPos);

				//void
				if (!b)
				{
					direction = {};
					waitTime = 0;
				}
				else if(!b->isColidable())
				{
					
					if (glm::dot(entity.lookDirectionAnimation, {0,-0.5,-1}) < 0.7)
					{
						changeHeadTimer = getRandomNumberFloat(rng, 0.5, 1.5); //look down
						entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,-0.5,-1}, 0.2);
						playerFollow = 0;
					}
					
				
					//bigger fall under
					blockPos.y--;
					auto b = c->safeGet(blockPos);

					if (!b || !b->isColidable())
					{
						direction = {};
						waitTime = 0;
					}
				
				}


			}



		}
		else
		{
			direction = {};
			waitTime = 0;
		}


		auto move = moveSpeed * deltaTime * direction;
		getPosition().x += move.x;
		getPosition().z += move.y;

		entity.bodyOrientation = move;
		entity.movementSpeedForLegsAnimations = 2.f * moveSpeed;
	}
	else
	{
		entity.movementSpeedForLegsAnimations = 0.f;

	}

#pragma region random jump
	if (randomJumpTimer < 0)
	{
		randomJumpTimer += getRandomNumberFloat(rng, 1, 10);
		if (getRandomNumber(rng, 0, 9) == 1)
		{
			entity.forces.jump();

		}
	}
#pragma endregion


#pragma region head orientation

	if (changeHeadTimer < 0)
	{

		playersClose.clear();

		//todo also check distance < 15 blocks
		for (auto offset : checkOffsets)
		{
			glm::ivec2 pos = chunkPosition + offset;
			auto c = serverChunkStorer.getChunkOrGetNull(pos.x, pos.y);
			if (c)
			{
				for (auto &p : c->entityData.players)
				{
					if (checkPlayerDistance(p.second.position, getPosition()))
					{
						playersClose.push_back(p.first);
					}
				}
			}
		}

		int headRandomDecision = 0;

		if (!playersClose.empty())
		{
			headRandomDecision = getRandomNumber(rng, 0, 100) % 5;
		}
		else
		{
			headRandomDecision = getRandomNumber(rng, 0, 100) % 4;
		}

		if (headRandomDecision == 0 || headRandomDecision == 1)
		{
			//look forward
			changeHeadTimer = getRandomNumberFloat(rng, 1, 8);
			entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1}, 3.14159 / 5.f);
		}
		else if(headRandomDecision == 2 || headRandomDecision == 3)
		{
			//look random
			changeHeadTimer = getRandomNumberFloat(rng, 1, 4);
			entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1});
		}
		else
		{
			int playerIndex = 0; 
			if (playersClose.size() > 1)
			{
				playerIndex = getRandomNumber(rng, 0, playersClose.size() - 1);
				playerFollow = playersClose[playerIndex];
			}
			else
			{
				playerFollow = playersClose[0];
			}
			changeHeadTimer = getRandomNumberFloat(rng, 1, 6);

		}
		

		//don't break their neck lol
		adjustVectorTowardsDirection(entity.lookDirectionAnimation);

	}

	if (playerFollow)
	{
		PlayerData *found = 0;
		for (auto offset : checkOffsets)
		{
			glm::ivec2 pos = chunkPosition + offset;
			auto c = serverChunkStorer.getChunkOrGetNull(pos.x, pos.y);
			if (c)
			{
				for (auto &p : c->entityData.players)
				{
					
					if (p.first == playerFollow && checkPlayerDistance(p.second.position, getPosition()))
					{
						found = &p.second;
						break;
					}
				}
				if (found) { break; }
			}
		}


		if (!found)
		{
			playerFollow = 0;
			changeHeadTimer -= 1;
		}
		else
		{

			glm::vec3 vireDirection = found->position - getPosition();
			float l = glm::length(vireDirection);
			if (l > 0.01)
			{
				vireDirection /= l;
				entity.lookDirectionAnimation = vireDirection;
			}

			removeBodyRotationFromHead(entity.bodyOrientation, entity.lookDirectionAnimation);

			//don't break their neck lol
			adjustVectorTowardsDirection(entity.lookDirectionAnimation);
		}

	}

#pragma endregion


	entity.update(deltaTime, chunkGetter);

}
