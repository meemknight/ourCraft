#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <random>
#include <gameplay/ai.h>

struct BasicEnemyBehaviourOtherSettings
{

	float searchForPlayerTimer = 1; //how much time will it take untill the entity searches for another target, (while it doesn't have a target)
	float searchDistance = 40; //players that get to this close will start getting targeted

};


struct BasicEnemyBehaviour
{

	glm::vec2 direction = {};
	float waitTime = 1;
	float keepJumpingTimer = 0;
	std::uint64_t playerLockedOn = 0;
	
	enum states
	{
		stateStaying = 0, //just stays in place and looks around.
		stateWalkingRandomly,
		stateTargetedPlayer,

	};
	
	float stateChangeTimer = 1;

	struct
	{
		float timerChangeLookDirection = 1;
	}stateStayingData;

	struct
	{
		float changeDirectionTime = 1;
	}stateWalkingRandomlyData;

	struct
	{

	}stateTargetedPlayerData;


	unsigned char currentState = stateStaying;
	float searchForPlayerTimer = 1;

	template<typename BaseEntity>
	void update(
		BaseEntity *baseEntity,
		float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFindingSurvival,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPositionSurvival, const glm::dvec3 currentPosition,
		BasicEnemyBehaviourOtherSettings otherSettings)
	{

		glm::vec3 realLookDirection = normalize(computeLookDirection(baseEntity->entity.bodyOrientation,
			baseEntity->entity.lookDirectionAnimation));
		
		auto searchForPlayerLogicIfNeeded = [&]()
		{

			if (playerLockedOn != 0) { return; }
			if (playersPositionSurvival.empty()) { return; }

			searchForPlayerTimer -= deltaTime;
			if (searchForPlayerTimer < 0)
			{

				//todo temporary allocator
				std::vector< std::uint64_t> searches;
				searches.reserve(playersPositionSurvival.size());
				for (auto &p : playersPositionSurvival) { searches.push_back(p.first); }
				std::shuffle(searches.begin(), searches.end(), rng);

				for (auto playerID :searches)
				{
					auto position = playersPositionSurvival[playerID];
					
					float distance = glm::distance(position, currentPosition);
					if (distance <= otherSettings.searchDistance)
					{
						//this player is close enough

						if (distance <= otherSettings.searchDistance / 10.f)
						{
							//instant target!
							playerLockedOn = playerID;
							break;
						}
						else
						{
							//percentage from approaching from begind
							float percentage = distance;
							percentage -= otherSettings.searchDistance / 10.f;
							percentage /= (otherSettings.searchDistance * ( 9.f / 10.f));
							percentage = 1.f - glm::clamp(percentage, 0.f, 1.f);

							//std::cout << "Distance Calculated: " << percentage << "; final percentage: " 
							//	<< 100*pow(percentage, 4.f) << "%\n";
							percentage = pow(percentage, 4.f);

							glm::vec3 lookDirection = realLookDirection;
							glm::vec3 vectorToPlayer = normalize(position - currentPosition);

							float viewFactor = glm::clamp(glm::dot(lookDirection, vectorToPlayer), 0.f, 1.f);
							//std::cout << "viewFactor Calculated: " << viewFactor << "; final viewFactor: "
							//	<< 100*pow(viewFactor, 8.f) << "%\n";
							//std::cout << "vectorToPlayer: " << vectorToPlayer.x << " " << vectorToPlayer.y << " " << vectorToPlayer.z << "\n";
							//std::cout << "lookDirection: " << lookDirection.x << " " << lookDirection.y << " " << lookDirection.z << "\n";
							viewFactor = pow(viewFactor, 8.f);

							percentage += viewFactor * 0.7;

							if (getRandomChance(rng, percentage))
							{
								playerLockedOn = playerID;
								break;
							}
						}

					}
				}

				//remove lock on player!
				//playerLockedOn = 0;

				searchForPlayerTimer = otherSettings.searchForPlayerTimer;
			}

		};

		glm::dvec3 playerLockedOnPosition = currentPosition;

		auto lookAtTargetedPlayer = [&]()
		{
			auto found = playersPositionSurvival.find(playerLockedOn);

			if (found == playersPositionSurvival.end())
			{
				playerLockedOn = 0;
			}
			else
			{
				lookAtPosition(found->second, baseEntity->entity.lookDirectionAnimation,
					currentPosition, baseEntity->entity.bodyOrientation,
					glm::radians(65.f));

			}
		};

		auto lookAtARandomDirection = [&]()
		{
			auto direction = getRandomUnitVector(rng);
			float up = getRandomNumberFloat(rng, -glm::radians(10.f), glm::radians(20.f));
			glm::vec3 finalVector = glm::normalize(glm::vec3(direction.x, up, direction.y));

			if (glm::dot(glm::vec2(finalVector.x, finalVector.z), baseEntity->entity.bodyOrientation) > 0.5f)
			{
				lookAtDirection(finalVector, baseEntity->entity.lookDirectionAnimation,
					currentPosition, baseEntity->entity.bodyOrientation,
					glm::radians(65.f));
			}
			else
			{
				lookAtDirectionWithBodyOrientation(finalVector, baseEntity->entity.lookDirectionAnimation,
					currentPosition, baseEntity->entity.bodyOrientation,
					glm::radians(65.f));
			}
	
		};

		auto changeStateStaying = [&]()
		{
			stateStayingData = {};
			stateStayingData.timerChangeLookDirection = stateStayingData.timerChangeLookDirection = getRandomNumberFloat(rng, 1, 10)
				+ getRandomNumberFloat(rng, 1, 10);
			currentState = stateStaying;
		};

		auto changeRandomWalkDirectionIfNeeded = [&]()
		{
			//random walk
			stateWalkingRandomlyData.changeDirectionTime -= deltaTime;

			if (stateWalkingRandomlyData.changeDirectionTime <= 0)
			{
				stateWalkingRandomlyData.changeDirectionTime = getRandomNumberFloat(rng, 1, 5);
				stateWalkingRandomlyData.changeDirectionTime += getRandomNumberFloat(rng, 1, 5);

				direction = getRandomUnitVector(rng);

			}
		};

		auto changeStateRandomWalking = [&]()
		{
			stateWalkingRandomlyData = {};


			currentState = stateWalkingRandomly;

			stateWalkingRandomlyData.changeDirectionTime = 0;
			changeRandomWalkDirectionIfNeeded();

		};

		auto changeRandomState = [&]()
		{
			stateChangeTimer = getRandomNumberFloat(rng, 1, 5)
				+ getRandomNumberFloat(rng, 1, 5) + getRandomNumberFloat(rng, 1, 5);

			if (getRandomChance(rng, 0.5f))
			{
				if (currentState != stateStaying)
				{
					changeStateStaying();
				}
			}
			else
			{
				if (currentState != stateWalkingRandomly)
				{
					changeStateRandomWalking();
				}
			}
		};

		auto dontFallIntoGaps = [&]()
		{
			auto blockPos = currentPosition;
			blockPos.x += direction.x;
			blockPos.z += direction.y;

			//block under
			blockPos.y--;
			auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos));

			//void
			if (!b)
			{
				direction = {};
				stateWalkingRandomlyData.changeDirectionTime = 0;

				if (getRandomChance(rng, 0.4))
				{
					changeStateStaying();
				}
				else
				{
					changeRandomWalkDirectionIfNeeded();
				}

			}
			else if (!b->isColidable())
			{

				//bigger fall under?
				blockPos.y--;
				auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos));

				if (!b || !b->isColidable())
				{
					direction = {};
					waitTime = 0;
				}
			}

		};

		auto jumpIfNeeded = [&]()
		{

			if (direction.x != 0 && direction.y != 0)
			{
				auto blockPos = currentPosition;
				blockPos.x += direction.x;
				blockPos.z += direction.y;

				auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos));

				if (b && b->isColidable())
				{

					//don't jump too tall walls lol
					auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos) + glm::ivec3(0, 1, 0));
					if (!b || !b->isColidable())
					{
						auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos) + glm::ivec3(0, 2, 0));
						if (!b || !b->isColidable())
						{
							baseEntity->entity.forces.jump();
							keepJumpingTimer = 0.4;
						}
					}

				}
			}

		};

		auto applyMoveDirection = [&]()
		{
			if (direction.x != 0 && direction.y != 0)
			{
				auto move = 2.f * deltaTime * direction;

				baseEntity->entity.move(move);
				baseEntity->entity.bodyOrientation = direction;
			}
		};

		auto checkIfStillSeingPlayer = [&]()
		{
			if (playerLockedOn == 0) { return; }

			auto found = playersPositionSurvival.find(playerLockedOn);

			if (found == playersPositionSurvival.end())
			{
				playerLockedOn = 0;
				changeRandomState();
			}
			else
			{
				float distance = glm::distance(found->second, currentPosition);

				if (distance > otherSettings.searchDistance + 5)
				{
					playerLockedOn = 0;
					changeRandomState();
				}
				else
				{
					//todo raymarch through the world.

					playerLockedOnPosition = found->second;
				}

			}

		};

		auto followPlayer = [&]()
		{
			if (!playerLockedOn) { return; }

			auto vectorToPlayer = playerLockedOnPosition - currentPosition;
			vectorToPlayer.y = 0;
			bool closeToPlayer = (glm::length(vectorToPlayer) > 1.2f);

			bool pathFindingSucceeded = 0;
			if (keepJumpingTimer >= 0)
			{
				keepJumpingTimer -= deltaTime;
				if (direction != glm::vec2(0, 0))
				{
					pathFindingSucceeded = true;
				}
				//else
				//{
				//	direction = {0,0};
				//}
			}
			//else
			//{
			//	direction = {0,0};
			//}

			if(!pathFindingSucceeded && closeToPlayer)
			{

				auto path = pathFindingSurvival.find(playerLockedOn);

				if (path != pathFindingSurvival.end())
				{

					auto pos = from3DPointToBlock(currentPosition);

					auto foundNode = path->second.find(pos);

					if (foundNode != path->second.end())
					{

						std::pair<glm::ivec3, PathFindingNode> interPolateNode = *foundNode;
						std::pair<glm::ivec3, PathFindingNode> interPolateNodeNotGood = *foundNode;

						int originalHeight = pos.y;

						//interpolate
						int maxCounterLoop = 100;
						int interpolateSteps = 0;
						if (1)
							while (maxCounterLoop-- > 0)
							{

								if (originalHeight != interPolateNodeNotGood.second.returnPos.y)
								{
									break;
								}

								auto newFoundNode = path->second.find(interPolateNodeNotGood.second.returnPos);

								if (newFoundNode != path->second.end())
								{
									if (newFoundNode->second.level > 0)
									{

										glm::dvec3 direction = glm::dvec3(newFoundNode->second.returnPos) + glm::dvec3(0, 0.5, 0)
											- currentPosition;

										direction.y = 0;

										float checklength = glm::length(direction);

										if (checklength)
										{
											//normalize
											direction /= checklength;

											bool problems = 0;

											//try interpolation
											int maxCounter = 300;//so we don't get infinite loops

											glm::dvec3 start = currentPosition;
											start.y += 0.1;

											glm::dvec3 leftVector = -glm::cross(direction, glm::dvec3(0, 1, 0));

											glm::dvec3 start2 = start;
											start += leftVector * 0.20;
											start2 -= leftVector * 0.20;

											for (int i = 0; i < (checklength * 10) - 2; i++)
											{
												start += direction * 0.1;
												start2 += direction * 0.1;

												//glm::dvec3 direction2 = glm::dvec3(newFoundNode->second.returnPos) - start;
												//direction2.y = 0;
												//if (glm::dot(direction, direction2) < 0.f) { break; }

												auto checkOneDirection = [&](glm::ivec3 blockPos)
												{
													auto b = serverChunkStorer.getBlockSafe(blockPos + glm::ivec3(0, 1, 0));
													if (b && b->isColidable())
													{
														problems = true;
														return true;
													}

													b = serverChunkStorer.getBlockSafe(blockPos + glm::ivec3(0, -1, 0));
													if (!b || !b->isColidable())
													{
														problems = true;
														return true;
													}

													return false;
												};

												auto pos1 = from3DPointToBlock(start);
												auto pos2 = from3DPointToBlock(start2);

												if (pos1 != pos2)
												{
													if (checkOneDirection(pos1))
													{
														break;
													}
													if (checkOneDirection(pos2))
													{
														break;
													}
												}
												else
												{
													if (checkOneDirection(pos1))
													{
														break;
													}
												}

												//blockPos.y -= 2;
												//auto b2 = serverChunkStorer.getBlockSafe(blockPos);
												//if (!b2 || !b2->isColidable())
												//{
												//	problems = true;
												//	break;
												//}

											}

											if (maxCounter <= 0) { break; }

											if (problems)
											{
												//std::cout << "Problems! ";
												interPolateNodeNotGood = *newFoundNode;
												//break; //worse interpolation but less lickely to have problems
											}
											else
											{
												interpolateSteps++;
												interPolateNodeNotGood = *newFoundNode;
												interPolateNode = *newFoundNode;
											}


										};

									}
									else
									{
										break;
									}
								}
								else
								{
									break;
								}

							}

						//if (interpolateSteps <= 1 && 
						//	glm::length(glm::dvec2(pos.x + 0.5,pos.z + 0.5) - glm::dvec2(getPosition().x, getPosition().z))
						//	< 0.5
						//	)
						//{
						//	glm::vec3 d = glm::dvec3(pos.x + 0.5, 0, pos.z + 0.5)
						//		- getPosition();
						//	d.y = 0;
						//	std::cout << "yes";
						//	direction.x = d.x;
						//	direction.y = d.z;
						//}
						//else
							{
								glm::vec3 d = glm::dvec3(interPolateNode.second.returnPos)
									- currentPosition;
								d.y = 0;

								direction.x = d.x;
								direction.y = d.z;
							}

							if (glm::length(direction) != 0)
							{
								direction = glm::normalize(direction);

								pathFindingSucceeded = true;
							}
					}

				};


				if (1)
					if (!pathFindingSucceeded)
					{
						//std::cout << "yess ";

						glm::vec3 d = playerLockedOnPosition
							- currentPosition;
						d.y = 0;

						direction.x = d.x;
						direction.y = d.z;

						if (glm::length(direction) != 0)
						{
							direction = glm::normalize(direction);
						}

					}

			};
		};

		searchForPlayerLogicIfNeeded();
		checkIfStillSeingPlayer();
		lookAtTargetedPlayer();

		if (playerLockedOn != 0)
		{
			currentState = stateTargetedPlayer;
		}

		switch (currentState)
		{

			case stateStaying:
			{
				
				stateStayingData.timerChangeLookDirection -= deltaTime;
				if (stateStayingData.timerChangeLookDirection < 0)
				{
					stateStayingData.timerChangeLookDirection = getRandomNumberFloat(rng, 1, 10)
						+ getRandomNumberFloat(rng, 1, 10);

					lookAtARandomDirection();
				}


				stateChangeTimer -= deltaTime;
				if(stateChangeTimer < 0)
				{
					changeRandomState();
				}

			}

			case stateWalkingRandomly:
			{

				changeRandomWalkDirectionIfNeeded();

				dontFallIntoGaps();

				jumpIfNeeded();

				applyMoveDirection();

				stateChangeTimer -= deltaTime;
				if (stateChangeTimer < 0)
				{
					changeRandomState();
				}
			}
			
			case stateTargetedPlayer:
			{
				followPlayer();

				jumpIfNeeded();

				applyMoveDirection();
			}


		}

		float followDistance = 22;
		float keepFollowDistance = 33;
		float randomSightBonus = 10;


		//if (!playerLockedOn)
		//{
		//
		//	//todo temporary allocator
		//
		//	std::vector<std::uint64_t> close;
		//
		//	for (auto &p : playersPosition)
		//	{
		//		if (glm::distance(p.second, currentPosition) <= followDistance)
		//		{
		//			close.push_back(p.first);
		//		}
		//	}
		//
		//	if (!close.empty())
		//	{
		//		playerLockedOn = close[rng() % close.size()];
		//	}
		//
		//}
		




		//playerLockedOn = 0;

		//auto playeerPos2D = playerLockedOnPosition;
		//playeerPos2D.y = 0;
		//auto pos2D = getPosition();
		//pos2D.y = 0;



	






	}



};