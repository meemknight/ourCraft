#pragma once 
#include <gameplay/physics.h>
#include <glm/glm.hpp>
#include <random>
#include <glm/gtx/rotate_vector.hpp>
#include <gameplay/entity.h>
#include <chunk.h>
#include <gameplay/player.h>	
#include <array>



struct Personality
{
	unsigned short fearfull = fromFloatToUShort(0.5f);
	unsigned short curious = fromFloatToUShort(0.5f);
	unsigned short playfull = fromFloatToUShort(0.5f);

	constexpr Personality() {};


	constexpr Personality(float f, float c, float e, float p)
	{
		fearfull = fromFloatToUShort(f);
		curious = fromFloatToUShort(c);
		playfull = fromFloatToUShort(p);
	};

};



struct PigDefaultSettings
{
	constexpr static float minSpeed = 4.0;
	constexpr static float maxSpeed = 7.0;

	constexpr static unsigned short minFearfull = fromFloatToUShort(0.3f);
	constexpr static unsigned short maxFearfull = fromFloatToUShort(0.6f);

	constexpr static unsigned short minCurious = fromFloatToUShort(0.1f);
	constexpr static unsigned short maxCurious = fromFloatToUShort(0.6f);

	constexpr static unsigned short minPlayfull = fromFloatToUShort(0.0f);
	constexpr static unsigned short maxPlayfull = fromFloatToUShort(0.6f);
};

struct CatDefaultSettings
{
	constexpr static float minSpeed = 9.0;
	constexpr static float maxSpeed = 13.0;

	constexpr static unsigned short minFearfull = fromFloatToUShort(0.4f);
	constexpr static unsigned short maxFearfull = fromFloatToUShort(0.8f);

	constexpr static unsigned short minCurious = fromFloatToUShort(0.5f);
	constexpr static unsigned short maxCurious = fromFloatToUShort(0.9f);

	constexpr static unsigned short minPlayfull = fromFloatToUShort(0.1f);
	constexpr static unsigned short maxPlayfull = fromFloatToUShort(0.7f);
};


template <class E, class SETTINGS>
struct AnimalBehaviour
{

	Personality personalityBase;


	unsigned short fearLevel = fromFloatToUShort(0.5f);

	float speedBase = 1.f;


	glm::vec2 direction = {};
	int moving = 0;
	float currentMoveSpeed = 0;

	float waitTime = 1;
	float randomJumpTimer = 1;
	float changeHeadTimer = 1;
	std::uint64_t playerFollow = 0;
	std::uint64_t approachingPlayer = 0;


	void updateAnimalBehaviour(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);


	inline void configureSpawnSettings(std::minstd_rand &rng);

};


template<class E, class SETTINGS>
inline void AnimalBehaviour<E, SETTINGS>::updateAnimalBehaviour(float deltaTime,
	decltype(chunkGetterSignature) *chunkGetter, 
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng)
{
	E *baseEntity = (E *)(this);
	glm::ivec2 chunkPosition = determineChunkThatIsEntityIn(baseEntity->getPosition());

	auto checkPlayerDistance = [](glm::dvec3 a, glm::dvec3 b)
	{
		return glm::length(a - b) <= 15.f;
	};


#pragma region get close players
	std::vector<std::pair<std::uint64_t, glm::dvec3>> playersClose;

	glm::dvec3 closestPlayerPosition = {};
	std::uint64_t closestPlayer = {};
	bool thereIsClosestPlayer = false;
	float closesestDist = 1000000000000.f;

	for (auto offset : *getChunkNeighboursOffsets())
	{
		glm::ivec2 pos = chunkPosition + offset;
		auto c = serverChunkStorer.getChunkOrGetNull(pos.x, pos.y);
		if (c)
		{
			for (auto &p : c->entityData.players)
			{
				float distance = glm::length(p.second->entity.position - baseEntity->getPosition());

				if (distance <= 15)
				{
					playersClose.push_back({p.first, p.second->entity.position});
					thereIsClosestPlayer = true;

					if (distance < closesestDist)
					{
						closesestDist = distance;
						closestPlayerPosition = p.second->entity.position;
						closestPlayer = p.first;
					}
				}
			}
		}
	}
#pragma endregion


	waitTime -= deltaTime;
	changeHeadTimer -= deltaTime;
	randomJumpTimer -= deltaTime;

	bool fleeing = false;
	bool carefull = false;


	auto stopOrChangeDirectionIfFlee = [&]()
	{
		if (fleeing)
		{
			if (getRandomNumber(rng, 0, 10) % 2)
			{
				direction = {direction.y, -direction.x};
			}
			else
			{
				direction = {-direction.y, direction.x};
			}
		}
		else
		{
			direction = {};
			waitTime = 0;
		}
	};

	//todo refactor and use position
	auto lookAtPlayer = [&](std::uint64_t player)
	{
		bool foundPlayer = 0;
		glm::dvec3 foundPosition = {};
		for (auto &p : playersClose)
		{
			if (p.first == player)
			{
				foundPlayer = true;
				foundPosition = p.second;
				break;
			}
		}

		if (!foundPlayer)
		{
			return false;
		}
		else
		{

			glm::vec3 vireDirection = foundPosition + glm::dvec3(0, 1.0, 0) - baseEntity->getPosition();
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

		return true;
	};


	if (fromUShortToFloat(fearLevel) >= 0.9)
	{
		fleeing = true;
	}
	else if (fromUShortToFloat(fearLevel) >= 0.75)
	{
		if (thereIsClosestPlayer && closesestDist < 7)
		{
			fleeing = true;
		}
	}
	else if (fromUShortToFloat(fearLevel) >= 0.60)
	{
		carefull = true;
	}

	//approachingPlayer = closestPlayer; //todo

	if (fleeing) { approachingPlayer = false; }
	if (fleeing)
	{
		currentMoveSpeed = speedBase;
		moving = true;

		if (direction == glm::vec2{0, 0})
		{ waitTime = 0; }

		if (waitTime <= 0)
		{
			waitTime = getRandomNumberFloat(rng, 0.5, 2.0);


			if (thereIsClosestPlayer)
			{
				glm::vec3 newDir = baseEntity->getPosition() - closestPlayerPosition;
				newDir.y = 0;

				float l = glm::length(newDir);
				if (l)
				{
					newDir /= l;
					direction.x = newDir.x;
					direction.y = newDir.z;
				}
				else
				{
					direction = {1,0};
				}
			}
			else
			{
				direction = getRandomUnitVector(rng);
			}

		};
	}
	else if (approachingPlayer)
	{
		bool foundPlayer = 0;
		glm::dvec3 foundPosition = {};
		for (auto &p : playersClose)
		{
			if (p.first == approachingPlayer)
			{
				foundPlayer = true;
				foundPosition = p.second;
				break;
			}
		}

		if (!foundPlayer)
		{
			approachingPlayer = 0;
			moving = 0;
			waitTime = 0;
		}
		else
		{
			moving = 1;
			
			glm::vec3 newDir = closestPlayerPosition - baseEntity->getPosition();
			newDir.y = 0;

			float l = glm::length(newDir);
			if (l > 1.9f)
			{
				newDir /= l;
				direction.x = newDir.x;
				direction.y = newDir.z;
			}
			else
			{
				moving = 0;
			}
		}

		if (waitTime < 0) { approachingPlayer = false; }

	}else
	{

		if (waitTime < 0)
		{

			if (playersClose.size() && getRandomChance(rng, 0.4 * 
				fromUchartoFloat(personalityBase.curious)))
			{
				//follow player.
				waitTime = getRandomNumberFloat(rng, 4, 12); 
				approachingPlayer = playersClose[getRandomNumber(rng, 0, playersClose.size())].first;
			}
			else
			{
				moving = getRandomChance(rng, 0.4 + fromUShortToFloat(personalityBase.playfull)*0.3 );
				waitTime = getRandomNumberFloat(rng, 1, 7);

				if (!moving)
				{
					waitTime += getRandomNumberFloat(rng, 0, 10);
					waitTime += getRandomNumberFloat(rng, 0, 5);
				}

				if (moving)
				{
					direction = getRandomUnitVector(rng);

					currentMoveSpeed = getRandomNumberFloat(rng, speedBase / 3.f, speedBase / 2.f);

					//not playfull entities run less
					if (fromUShortToFloat(personalityBase.playfull) < 0.5)
					{
						currentMoveSpeed = std::min(getRandomNumberFloat(rng, speedBase / 3.f, speedBase / 2.f), currentMoveSpeed);
					}

					//playfull entities run more
					currentMoveSpeed *= (fromUShortToFloat(personalityBase.playfull) / 2.f + 0.9f);
				}
			}

			if (waitTime > 5.f)
			{
				waitTime -= personalityBase.playfull * 2;
			}

		
		}
	}

	
	if (direction == glm::vec2{0,0})
	{
		moving = 0;
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

			if (b && b->isColidable())
			{
				blockPos.y++;
				auto b = c->safeGet(blockPos);
				if (b && b->isColidable())
				{
					//wall
					stopOrChangeDirectionIfFlee();
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
					stopOrChangeDirectionIfFlee();
				}
				else if (!b->isColidable())
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

					if(!fleeing)
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

		auto move = currentMoveSpeed * deltaTime * direction;

		if (carefull)
		{
			move *= 0.6;
		}

		if (carefull && approachingPlayer)
		{
			move *= 0.5;
		}

		baseEntity->getPosition().x += move.x;
		baseEntity->getPosition().z += move.y;

		baseEntity->entity.bodyOrientation = move;
		baseEntity->entity.movementSpeedForLegsAnimations = 1.5f * currentMoveSpeed;
	}
	else
	{
		baseEntity->entity.movementSpeedForLegsAnimations = 0.f;

	}

#pragma region random jump

	if (fleeing)
	{
		randomJumpTimer = std::min(randomJumpTimer, 4.f);
	}

	if (randomJumpTimer < 0)
	{
		if (fleeing)
		{
			randomJumpTimer += getRandomNumberFloat(rng, 0.5, 4);
		}
		else
		{
			randomJumpTimer += 
				getRandomNumberFloat(rng, 1, (10 - fromUShortToFloat(personalityBase.playfull) * 6.f));
		}

		if (fleeing || getRandomNumber(rng, 0, 9 - fromUShortToFloat(personalityBase.playfull) * 5) == 1)
		{
			baseEntity->entity.forces.jump();

		}
	}
#pragma endregion


#pragma region head orientation

	if (fleeing)
	{
		changeHeadTimer = std::min(changeHeadTimer, 1.5f);
	}

	if (carefull)
	{
		changeHeadTimer = std::min(changeHeadTimer, 2.5f);
	}

	if (approachingPlayer)
	{
		lookAtPlayer(approachingPlayer);
	}
	else
	{
		if (changeHeadTimer < 0)
		{

			if (fleeing)
			{

				if (thereIsClosestPlayer)
				{
					if (getRandomNumber(rng, 0, 100) % 2)
					{
						baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.3,-1}, 3.14159 / 5.f);
					}
					else
					{
						playerFollow = closestPlayer;
					}
				}
				else
				{
					baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.3,-1}, 3.14159 / 5.f);
				}

				changeHeadTimer = getRandomNumberFloat(rng, 0.5, 1.5);
			}
			else
			{

				//if carefull they will lickely look at the closest player
				if (carefull)
				{
					changeHeadTimer = getRandomNumberFloat(rng, 1, 2.5);

					if (closestPlayer)
					{
						if (getRandomNumber(rng, 0, 100) % 2)
						{
							//look at player
							playerFollow = closestPlayer;
						}
						else
						{
							//look random
							baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1}, 3.14159 / 5.f);
						}
					}
					else
					{
						//look random
						baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1});
					}


				}
				else
				{

					bool lookAtPlayerFlag = getRandomChance(rng,
						0.5 * fromUShortToFloat(personalityBase.curious) + 0.2 + 0.2 * carefull);
					if (playersClose.empty()) 
					{
						lookAtPlayerFlag = 0;
					}
					
					if (lookAtPlayerFlag)
					{
						//look at player
						int playerIndex = 0;
						if (playersClose.size() > 1)
						{
							playerIndex = getRandomNumber(rng, 0, playersClose.size() - 1);
							playerFollow = playersClose[playerIndex].first;
						}
						else
						{
							playerFollow = playersClose[0].first;
						}
						changeHeadTimer = getRandomNumberFloat(rng, 1, 8);
					}
					else
					{
						if (getRandomChance(rng, 0.5 + (moving)*0.1 - 
							fromUShortToFloat(personalityBase.curious) * 0.3
							))
						{
							//look forward
							changeHeadTimer = getRandomNumberFloat(rng, 1, 8);
							baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1}, 3.14159 / 5.f);
						}
						else
						{
							//look random

							if (moving)
							{
								changeHeadTimer = getRandomNumberFloat(rng, 1, 4);
								baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1});
							}
							else
							{
								changeHeadTimer = getRandomNumberFloat(rng, 1, 6);
								changeHeadTimer += getRandomNumberFloat(rng, 1, 6);

								if (getRandomChance(rng, 0.5))
								{
									direction = getRandomUnitVector(rng);
								}

								baseEntity->entity.lookDirectionAnimation = getRandomUnitVector3Oriented(rng, {0,0.1,-1});
							}
						}

					}

					

					//curious entities look around more often.
					if (changeHeadTimer > 2.f)
					{
						changeHeadTimer -= fromUShortToFloat(personalityBase.curious);
					}
				}

				
			}



			//don't break their neck lol
			adjustVectorTowardsDirection(baseEntity->entity.lookDirectionAnimation);

		}

		if (playerFollow)
		{
			if(!lookAtPlayer(playerFollow))
			{
				playerFollow = 0;
				changeHeadTimer -= 1;
			}
		}
	}

	

#pragma endregion


#pragma region update feelings


	//add fear if players close
	{

		{
			if (closesestDist <= 7.f)
			{
				addFear(fearLevel, personalityBase.fearfull, 0.1);
			}
		}


	}

	//std::cout << fromUShortToFloat(fearLevel) << "\n";

	//bring fear down
	{

		int changePerSeccond = ((USHORT_MAX / 2) / 20) / targetTicksPerSeccond;


		if (fearLevel < personalityBase.fearfull)
		{
			if (int(fearLevel) + changePerSeccond > personalityBase.fearfull)
			{
				fearLevel = personalityBase.fearfull;
			}
			else
			{
				fearLevel += changePerSeccond;
			}
		}
		else if (fearLevel > personalityBase.fearfull)
		{
			if (int(fearLevel) - changePerSeccond < (int)personalityBase.fearfull)
			{
				fearLevel = personalityBase.fearfull;
			}
			else
			{
				fearLevel -= changePerSeccond;
			}
		}

	}
#pragma endregion



}

template<class E, class SETTINGS>
inline void AnimalBehaviour<E, SETTINGS>::configureSpawnSettings(std::minstd_rand &rng)
{
	//std::cout << "YES \n";

	personalityBase.curious = getRandomNumber(rng, SETTINGS::minCurious, SETTINGS::maxCurious);
	personalityBase.fearfull = getRandomNumber(rng, SETTINGS::minFearfull, SETTINGS::maxFearfull);
	personalityBase.playfull = getRandomNumber(rng, SETTINGS::minPlayfull, SETTINGS::maxPlayfull);
	
	
	fearLevel = personalityBase.fearfull;
	speedBase = getRandomNumberFloat(rng, SETTINGS::minSpeed, SETTINGS::maxSpeed);

}
