#pragma once 
#include <gameplay/physics.h>
#include <glm/glm.hpp>
#include <random>
#include <glm/gtx/rotate_vector.hpp>
#include <gameplay/entity.h>
#include <chunk.h>
#include <gameplay/player.h>	
#include <array>


struct PigDefaultSettings
{
	constexpr static float minSpeed = 0.9;
	constexpr static float maxSpeed = 1.5;
};

template <class E, class SETTINGS>
struct AnimalBehaviour
{

	glm::vec2 direction = {};
	int moving = 0;
	float waitTime = 1;
	float randomJumpTimer = 1;
	float moveSpeed = 1.f;
	float changeHeadTimer = 1;
	std::uint64_t playerFollow = 0;


	void updateAnimalBehaviour(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);


};

inline std::array<glm::ivec2, 9> *getChunkNeighboursOffsets()
{
	static std::array<glm::ivec2, 9> checkOffsets = {
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

	return &checkOffsets;
};

template<class E, class SETTINGS>
inline void AnimalBehaviour<E, SETTINGS>::updateAnimalBehaviour(float deltaTime,
	decltype(chunkGetterSignature) *chunkGetter, 
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng)
{

	auto checkPlayerDistance = [](glm::dvec3 a, glm::dvec3 b)
	{
		return glm::length(a - b) <= 15.f;
	};


	std::vector<std::uint64_t> playersClose;

	
	E *baseEntity = (E *)(this);

	glm::ivec2 chunkPosition = determineChunkThatIsEntityIn(baseEntity->getPosition());

	waitTime -= deltaTime;
	changeHeadTimer -= deltaTime;
	randomJumpTimer -= deltaTime;

	if (waitTime < 0)
	{
		moving = getRandomNumber(rng, 0, 100) % 2;
		waitTime = getRandomNumberFloat(rng, 1, 4);

		if (moving)
		{
			direction = getRandomUnitVector(rng);
			moveSpeed = getRandomNumberFloat(rng, SETTINGS::minSpeed, SETTINGS::maxSpeed);
		}
	}

	if (moving)
	{

		//check falling

		auto pos = baseEntity->getPosition();
		pos.x += direction.x;
		pos.z += direction.y;
		pos.y += 0.6;

		auto chunkPos = determineChunkThatIsEntityIn(pos);
		auto c = chunkGetter(chunkPos);
		if (c)
		{
			auto blockPos = fromBlockPosToBlockPosInChunk(glm::ivec3(pos));
			auto b = c->safeGet(blockPos);

			if (b && b->isCollidable())
			{
				blockPos.y++;
				auto b = c->safeGet(blockPos);
				if (b && b->isCollidable())
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
						baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.5,-1}, 0.2);
						playerFollow = 0;
					}

					baseEntity->entity.forces.jump();
				}

			}
			else if (b)
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
				else if (!b->isCollidable())
				{

					if (glm::dot(baseEntity->entity.lookDirectionAnimation, {0,-0.5,-1}) < 0.7)
					{
						changeHeadTimer = getRandomNumberFloat(rng, 0.5, 1.5); //look down
						baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,-0.5,-1}, 0.2);
						playerFollow = 0;
					}


					//bigger fall under
					blockPos.y--;
					auto b = c->safeGet(blockPos);

					if (!b || !b->isCollidable())
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
		baseEntity->getPosition().x += move.x;
		baseEntity->getPosition().z += move.y;

		baseEntity->entity.bodyOrientation = move;
		baseEntity->entity.movementSpeedForLegsAnimations = 2.f * moveSpeed;
	}
	else
	{
		baseEntity->entity.movementSpeedForLegsAnimations = 0.f;

	}

#pragma region random jump
	if (randomJumpTimer < 0)
	{
		randomJumpTimer += getRandomNumberFloat(rng, 1, 10);
		if (getRandomNumber(rng, 0, 9) == 1)
		{
			baseEntity->entity.forces.jump();

		}
	}
#pragma endregion


#pragma region head orientation

	if (changeHeadTimer < 0)
	{

		playersClose.clear();

		//todo also check distance < 15 blocks
		for (auto offset : *getChunkNeighboursOffsets())
		{
			glm::ivec2 pos = chunkPosition + offset;
			auto c = serverChunkStorer.getChunkOrGetNull(pos.x, pos.y);
			if (c)
			{
				for (auto &p : c->entityData.players)
				{
					if (checkPlayerDistance(p.second.entity.position, baseEntity->getPosition()))
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
			baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1}, 3.14159 / 5.f);
		}
		else if (headRandomDecision == 2 || headRandomDecision == 3)
		{
			//look random
			changeHeadTimer = getRandomNumberFloat(rng, 1, 4);
			baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1});
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
		adjustVectorTowardsDirection(baseEntity->entity.lookDirectionAnimation);

	}

	if (playerFollow)
	{
		PlayerServer *found = 0;
		for (auto offset : *getChunkNeighboursOffsets())
		{
			glm::ivec2 pos = chunkPosition + offset;
			auto c = serverChunkStorer.getChunkOrGetNull(pos.x, pos.y);
			if (c)
			{
				for (auto &p : c->entityData.players)
				{

					if (p.first == playerFollow && checkPlayerDistance(p.second.entity.position, baseEntity->getPosition()))
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

			glm::vec3 vireDirection = found->entity.position - baseEntity->getPosition();
			float l = glm::length(vireDirection);
			if (l > 0.01)
			{
				vireDirection /= l;
				baseEntity->entity.lookDirectionAnimation = vireDirection;
			}

			removeBodyRotationFromHead(baseEntity->entity.bodyOrientation, baseEntity->entity.lookDirectionAnimation);

			//don't break their neck lol
			adjustVectorTowardsDirection(baseEntity->entity.lookDirectionAnimation);
		}

	}

#pragma endregion


}
