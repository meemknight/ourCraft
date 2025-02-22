#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <random>
#include <gameplay/ai.h>

struct BasicEnemyBehaviour
{

	glm::vec2 direction = {};
	float waitTime = 1;
	float keepJumpingTimer = 0;
	float randomSightBonusTimer = 1;
	std::uint64_t playerLockedOn = 0;

	template<typename BaseEntity>
	void update(
		BaseEntity *baseEntity,
		float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition, const glm::dvec3 currentPosition)
	{


		float followDistance = 22;
		float keepFollowDistance = 33;
		float randomSightBonus = 10;
		glm::dvec3 playerLockedOnPosition = currentPosition;

		randomSightBonusTimer -= deltaTime;
		if (randomSightBonusTimer <= 0)
		{
			randomSightBonusTimer = getRandomNumberFloat(rng, 3, 12);
			randomSightBonusTimer += getRandomNumberFloat(rng, 3, 12);
			randomSightBonusTimer += getRandomNumberFloat(rng, 3, 12);

			followDistance += randomSightBonus;
		}

		if (!playerLockedOn)
		{

			//todo temporary allocator

			std::vector<std::uint64_t> close;

			for (auto &p : playersPosition)
			{
				if (glm::distance(p.second, currentPosition) <= followDistance)
				{
					close.push_back(p.first);
				}
			}

			if (!close.empty())
			{
				playerLockedOn = close[rng() % close.size()];
			}

		}

		//look at player
		{
			auto found = playersPosition.find(playerLockedOn);

			if (found == playersPosition.end())
			{
				playerLockedOn = 0;
			}
			else
			{
				lookAtPosition(found->second, baseEntity->entity.lookDirectionAnimation,
					currentPosition, baseEntity->entity.bodyOrientation,
					glm::radians(65.f));

				if (glm::distance(found->second, currentPosition) <= keepFollowDistance)
				{
					playerLockedOnPosition = found->second;
				}
				else
				{
					playerLockedOn = 0;
				}
			}
		}

		if (!playerLockedOn)
		{
			//todo look randomly
		}


		//playerLockedOn = 0;

		//auto playeerPos2D = playerLockedOnPosition;
		//playeerPos2D.y = 0;
		//auto pos2D = getPosition();
		//pos2D.y = 0;

		auto vectorToPlayer = playerLockedOnPosition - currentPosition;
		vectorToPlayer.y = 0;
		bool closeToPlayer = playerLockedOn && (glm::length(vectorToPlayer) > 1.2f);

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

		//disable following
		if (1)
			if (!pathFindingSucceeded && playerLockedOn && closeToPlayer)
			{

				auto path = pathFinding.find(playerLockedOn);

				if (path != pathFinding.end())
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

		//random walk
		if (1)
			//if (!playerLockedOn)
		{
			waitTime -= deltaTime;

			if (waitTime < 0)
			{
				int moving = getRandomChance(rng, 0.5);
				waitTime += getRandomNumberFloat(rng, 1, 8);

				if (moving)
				{
					direction = getRandomUnitVector(rng);
					waitTime += getRandomNumberFloat(rng, 0, 2);
				}
			}
		};

		//jump
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

		//don't fall into gaps when not following player
		if (1)
			if (!pathFindingSucceeded)
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
					waitTime = 0;
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


			}


		auto move = 2.f * deltaTime * direction;

		baseEntity->entity.move(move);
		baseEntity->entity.bodyOrientation = direction;


	}



};