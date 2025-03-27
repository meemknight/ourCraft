#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <random>
#include <gameplay/ai.h>
#include <unordered_map>
#include <unordered_set>

struct BasicEnemyBehaviourOtherSettings
{

	float searchForPlayerTimer = 1; //how much time will it take untill the entity searches for another target, (while it doesn't have a target)
	float searchDistance = 40; //players that get to this close will start getting targeted
	float hearBonus = 0.1;
	float sightBonus = 0.1;

};


struct BasicEnemyBehaviour
{

	glm::vec2 direction = {};
	float waitTime = 1;
	float keepJumpingTimer = 0;
	std::uint64_t playerLockedOn = 0;
	float worriedTimer = 0;
	
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
		float changeLookDirectionTime = 0;
	}stateWalkingRandomlyData;

	struct
	{

	}stateTargetedPlayerData;

	bool isUnaware()
	{
		if (worriedTimer > 0) { return false; }

		if (currentState == stateStaying || currentState == stateWalkingRandomly)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	template<typename BaseEntity>
	void signalHit(glm::vec3 direction, BaseEntity *baseEntity)
	{
		if (currentState != stateTargetedPlayer)
		{
			worriedTimer = 60;

			float len = glm::length(direction);
			if (len > 0)
			{
				direction /= len;
				baseEntity->wantToLookDirection = direction;
			}

			searchForPlayerTimer = 0;

			if (currentState == stateStaying)
			{
				stateWalkingRandomlyData = {};
				currentState = stateWalkingRandomly;
				stateWalkingRandomlyData.changeDirectionTime = 0;
			}

		}
	}

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

		worriedTimer -= deltaTime;
		if (worriedTimer < 0) { worriedTimer = 0; }

		glm::vec3 realLookDirection = baseEntity->entity.getLookDirection();
		
		auto changeStateStaying = [&]()
		{
			stateStayingData = {};

			if (worriedTimer)
			{
				stateStayingData.timerChangeLookDirection = stateStayingData.timerChangeLookDirection = getRandomNumberFloat(rng, 1, 3)
					+ getRandomNumberFloat(rng, 1, 2);
			}
			else
			{
				stateStayingData.timerChangeLookDirection = stateStayingData.timerChangeLookDirection = getRandomNumberFloat(rng, 1, 10)
					+ getRandomNumberFloat(rng, 1, 10);
			}


			currentState = stateStaying;
		};

		//used to look at things that grab atention, will add a random bias
		auto manuallySetLookAtDirection = [&](glm::vec3 newLookDirection)
		{
			if (currentState == stateStaying || currentState == stateWalkingRandomly)
			{

				if (worriedTimer)
				{
					stateStayingData.timerChangeLookDirection = getRandomNumberFloat(rng, 1, 2) + getRandomNumberFloat(rng, 1, 2);
				}
				else
				{
					stateStayingData.timerChangeLookDirection = getRandomNumberFloat(rng, 2, 3) + getRandomNumberFloat(rng, 1, 2);
				}

				stateWalkingRandomlyData.changeDirectionTime = stateStayingData.timerChangeLookDirection;
			}

			auto newLookDir = moveVectorRandomly(newLookDirection,
				rng, glm::radians(20.f), glm::radians(10.f));

			if (currentState == stateWalkingRandomly)
			{
				if (glm::dot(newLookDir, realLookDirection) < 0.8)
				{
					changeStateStaying();
				}
			}

			baseEntity->wantToLookDirection = newLookDir;
		};

		auto searchForPlayerLogicIfNeeded = [&]()
		{

			if (playerLockedOn != 0) { return; }
			if (playersPositionSurvival.empty()) { return; }

			//searchForPlayerTimer -= deltaTime;
			//if (searchForPlayerTimer < 0)
			{

				//todo temporary allocator
				std::vector< std::uint64_t> searches;
				searches.reserve(playersPositionSurvival.size());
				for (auto &p : playersPositionSurvival) { searches.push_back(p.first); }
				std::shuffle(searches.begin(), searches.end(), rng);

				for (auto playerID :searches)
				{
					auto positionPlayer = playersPositionSurvival[playerID];
					
					float distance = glm::distance(positionPlayer, currentPosition);
					if (distance <= otherSettings.searchDistance)
					{
						//this player is close enough

						//if (distance <= otherSettings.searchDistance / 10.f)
						//{
						//	//instant target!
						//	playerLockedOn = playerID;
						//	break;
						//}
						//else
						{
							//percentage from approaching from begind
							float percentage = distance;
							//percentage -= otherSettings.searchDistance / 10.f;
							//percentage /= (otherSettings.searchDistance * ( 9.f / 10.f));
							percentage /= otherSettings.searchDistance;
							float distancePercentage = percentage;

							percentage = 1.f - glm::clamp(percentage, 0.f, 1.f);


							//std::cout << "Distance Calculated: " << percentage << "; final percentage: " 
							//	<< 100*pow(percentage, 4.f) << "%\n";
							percentage = pow(percentage, 3.f);
							percentage += otherSettings.hearBonus;
							if (worriedTimer) { percentage += 0.2; }
							percentage = glm::clamp(percentage, 0.f, 1.f);


							glm::vec3 lookDirection = realLookDirection;
							glm::vec3 vectorToPlayer = normalize(positionPlayer - currentPosition);

							float viewFactor = glm::clamp(glm::dot(lookDirection, vectorToPlayer), 0.f, 1.f);
							//std::cout << "viewFactor Calculated: " << viewFactor << "; final viewFactor: "
							//	<< 100*pow(viewFactor, 8.f) << "%\n";
							//std::cout << "vectorToPlayer: " << vectorToPlayer.x << " " << vectorToPlayer.y << " " << vectorToPlayer.z << "\n";
							//std::cout << "lookDirection: " << lookDirection.x << " " << lookDirection.y << " " << lookDirection.z << "\n";
							viewFactor = pow(viewFactor, 4.f);
							
							//if the enemy looks at least slightly towards the player we add the view factor
							if (viewFactor > 0.1)
							{ 
								viewFactor += otherSettings.sightBonus;
								if (worriedTimer) { viewFactor += 0.2; }
							}

							viewFactor = glm::clamp(viewFactor, 0.f, 1.f);

							float finalPercentage = percentage;
							//if we are close and the enemy looks directly at us we have a big chance of being targeted.
							//if (distance <= otherSettings.searchDistance / 1.5f)
							//{
							//	finalPercentage += viewFactor * 0.7;
							//}
							//else
							//{
							//	finalPercentage += viewFactor * 0.3;
							//}

							//finalPercentage = lerp(std::max(percentage, viewFactor),  percentage * viewFactor, 0.5f);
							finalPercentage = viewFactor - (distancePercentage/3.f); //if you are far it will have a contribution

							//boost the chance of being seen if you are close and the enemy looks at you directly
							if (viewFactor > 0.5 && percentage > 0.5)
							{
								finalPercentage += 0.2;
							}

							//boost the chance even more if we are very close
							if (percentage > 0.5 && viewFactor > 0.001)
							{

								finalPercentage += 0.1;

								if (percentage > 0.8)
								{
									finalPercentage += 0.3;

									if (percentage > 0.8)
									{
										finalPercentage += 0.3;

										if (percentage > 0.9)
										{
											finalPercentage += 0.3;
										}
									}
								}
								
							}


							

							//std::cout << "Dist perc: " << percentage << "  , view Perc: " << viewFactor <<  "  ,Final Percentage: " << finalPercentage << "\n";

							//if(0)
							//we have to do this because we check this probability 20 tiems per seccond
							float probabilityAdjusted = 1.f - std::pow(1.f - finalPercentage, 1 / 20.f);
							float probabilityAdjustedHear = 1.f - std::pow(1.f - percentage, 1/20.f);

							if (probabilityAdjustedHear >= 0.99999 || getRandomChance(rng, probabilityAdjustedHear))
							{
								glm::vec3 newDir = positionPlayer - currentPosition;
								float l = glm::length(baseEntity->wantToLookDirection);
								if (l <= 0.000001)
								{
									newDir = {0,0,-1};
								}
								else
								{
									newDir /= l;
								}

								manuallySetLookAtDirection(newDir);
							}

							if (finalPercentage >= 0.99999 || getRandomChance(rng, probabilityAdjusted))
							{
								std::cout << "Locked!\n";
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
				baseEntity->wantToLookDirection = found->second - currentPosition;
				float l = glm::length(baseEntity->wantToLookDirection);
				if (l <= 0.000001)
				{
					baseEntity->wantToLookDirection = {0,0,-1};
				}
				else
				{
					baseEntity->wantToLookDirection /= l;
				}
				
				//lookAtPosition(found->second, baseEntity->entity.lookDirectionAnimation,
				//	currentPosition, baseEntity->entity.bodyOrientation,
				//	glm::radians(65.f));

			}
		};

		auto lookAtARandomDirection = [&]()
		{
			auto direction = getRandomUnitVector(rng);
			float up = getRandomNumberFloat(rng, -glm::radians(10.f), glm::radians(20.f));
			glm::vec3 finalVector = glm::normalize(glm::vec3(direction.x, up, direction.y));

			baseEntity->wantToLookDirection = finalVector;

			//todo remove
			//if (glm::dot(glm::vec2(finalVector.x, finalVector.z), baseEntity->entity.bodyOrientation) > 0.5f)
			//{
			//	lookAtDirection(finalVector, baseEntity->entity.lookDirectionAnimation,
			//		 baseEntity->entity.bodyOrientation,
			//		glm::radians(65.f));
			//}
			//else
			//{
			//	lookAtDirectionWithBodyOrientation(finalVector, baseEntity->entity.lookDirectionAnimation,
			//		baseEntity->entity.bodyOrientation,
			//		glm::radians(65.f));
			//}
	
		};

		auto changeRandomWalkDirectionIfNeeded = [&]()
		{
			//random walk
			stateWalkingRandomlyData.changeDirectionTime -= deltaTime;
			stateWalkingRandomlyData.changeLookDirectionTime -= deltaTime;
			bool relook = false;

			if (stateWalkingRandomlyData.changeDirectionTime <= 0)
			{
				if (worriedTimer)
				{
					stateWalkingRandomlyData.changeDirectionTime = getRandomNumberFloat(rng, 1, 3);
					stateWalkingRandomlyData.changeDirectionTime += getRandomNumberFloat(rng, 1, 3);
				}
				else
				{
					stateWalkingRandomlyData.changeDirectionTime = getRandomNumberFloat(rng, 1, 5);
					stateWalkingRandomlyData.changeDirectionTime += getRandomNumberFloat(rng, 1, 5);
				}


				direction = getRandomUnitVector(rng);

				baseEntity->wantToLookDirection = glm::vec3(direction.x, 0, direction.y);
				relook = true;

			}

			if (stateWalkingRandomlyData.changeLookDirectionTime <= 0 || relook)
			{
				if (worriedTimer)
				{
					stateWalkingRandomlyData.changeLookDirectionTime = getRandomNumberFloat(rng, 1, 3);
					stateWalkingRandomlyData.changeLookDirectionTime += getRandomNumberFloat(rng, 1, 3);
				}
				else
				{
					stateWalkingRandomlyData.changeLookDirectionTime = getRandomNumberFloat(rng, 1, 4);
					stateWalkingRandomlyData.changeLookDirectionTime += getRandomNumberFloat(rng, 1, 4);
				}



				glm::vec3 viewVector = glm::vec3(direction.x, 0, direction.y);

				baseEntity->wantToLookDirection = moveVectorRandomlyBiasKeepCenter(viewVector, 
					rng, glm::radians(50.f), glm::radians(20.f));


			}
		};

		auto changeStateRandomWalking = [&]()
		{
			stateWalkingRandomlyData = {};


			currentState = stateWalkingRandomly;

			stateWalkingRandomlyData.changeDirectionTime = 0;
			changeRandomWalkDirectionIfNeeded();

		};

		auto changeRandomState = [&](float stayingChance = 0.5)
		{

			stateChangeTimer = getRandomNumberFloat(rng, 1, 5)
				+ getRandomNumberFloat(rng, 1, 5) + getRandomNumberFloat(rng, 1, 5);

			if (getRandomChance(rng, 0.5f))
			{
				if (currentState != stateStaying)
				{
					changeStateStaying();

					if (worriedTimer)
					{
						stateChangeTimer = getRandomNumberFloat(rng, 1, 2)
							+ getRandomNumberFloat(rng, 1, 2);
					}
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

			if (direction.x != 0 || direction.y != 0)
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
			if (direction.x != 0 || direction.y != 0)
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
				changeRandomState(0.8);
			}
			else
			{
				float distance = glm::distance(found->second, currentPosition);

				if (distance > otherSettings.searchDistance + 5)
				{
					playerLockedOn = 0;
					changeRandomState(0.8);
				}
				else
				{
					//todo raymarch through the world.

					playerLockedOnPosition = found->second;
				}

			}

		};

		auto followPlayer = [&]() -> bool
		{
			if (!playerLockedOn) { return false; }

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

				//move in the direction of the player if path finding failed
				if (1)
					if (!pathFindingSucceeded)
					{
						std::cout << "path finding failed!\n";

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

				return pathFindingSucceeded;

			};
		};

		searchForPlayerLogicIfNeeded();
		checkIfStillSeingPlayer();
		lookAtTargetedPlayer();

		if (playerLockedOn != 0)
		{
			currentState = stateTargetedPlayer;
		}

		//if(0)
		switch (currentState)
		{

			case stateStaying:
			{

				stateStayingData.timerChangeLookDirection -= deltaTime;
				if (stateStayingData.timerChangeLookDirection < 0)
				{

					if (worriedTimer)
					{
						stateStayingData.timerChangeLookDirection = getRandomNumberFloat(rng, 1, 4)
							+ getRandomNumberFloat(rng, 1, 4);
					}
					else
					{
						stateStayingData.timerChangeLookDirection = getRandomNumberFloat(rng, 1, 10)
							+ getRandomNumberFloat(rng, 1, 10);
					}

					lookAtARandomDirection();
				}


				stateChangeTimer -= deltaTime;
				if(stateChangeTimer < 0)
				{
					changeRandomState();
				}
				break;
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
				break;
			}
			
			case stateTargetedPlayer:
			{
				if (!followPlayer())
				{
					dontFallIntoGaps();
				}

				jumpIfNeeded();

				applyMoveDirection();

				//will remain worried after just fighting
				worriedTimer = std::max(worriedTimer, 60.f);

				break;
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
