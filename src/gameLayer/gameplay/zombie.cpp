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

//todo temporary allocator
bool ZombieServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition
)
{

	constexpr float followDistance = 32;
	constexpr float keepFollowDistance = 42;
	glm::dvec3 playerLockedOnPosition = getPosition();

	if (!playerLockedOn)
	{

		//std temporary allocator
		std::vector<std::uint64_t> close;

		for (auto &p : playersPosition)
		{
			if (glm::distance(p.second, getPosition()) <= followDistance)
			{
				close.push_back(p.first);
			}
		}

		if (!close.empty())
		{
			playerLockedOn = close[rng() % close.size()];
		}
			
	}
	
	{
		auto found = playersPosition.find(playerLockedOn);

		if (found == playersPosition.end())
		{
			playerLockedOn = 0;
		}
		else
		{
			if (glm::distance(found->second, getPosition()) <= keepFollowDistance)
			{
				playerLockedOnPosition = found->second;
			}
			else
			{
				playerLockedOn = 0;
			}
		}
	}


	direction = {0,0};

	bool pathFindingSucceeded = 0;

	if (playerLockedOn)
	{



		auto path = pathFinding.find(playerLockedOn);

		if (path != pathFinding.end())
		{


			auto pos = from3DPointToBlock(getPosition());

			auto foundNode = path->second.find(pos);

			if (foundNode != path->second.end())
			{

				std::pair<glm::ivec3, PathFindingNode> interPolateNode = *foundNode;
				std::pair<glm::ivec3, PathFindingNode> interPolateNodeNotGood = *foundNode;

				int originalHeight = pos.y;

				//interpolate
				int maxCounterLoop = 100;
				if(1) 
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
							
							glm::dvec3 direction = glm::dvec3(newFoundNode->second.returnPos) - getPosition();

							direction.y = 0;

							if (glm::length(direction))
							{


								direction = glm::normalize(direction);

								bool problems = 0;

								//try interpolation
								int maxCounter = 300;//so we don't get infinite loops

								glm::dvec3 start = getPosition();

								glm::dvec3 leftVector = -glm::cross(direction, glm::dvec3(0, 1, 0));

								glm::dvec3 start2 = start - leftVector * 0.15;
								start += leftVector * 0.15;

								for (; maxCounter > 0; maxCounter--)
								{
									start += direction * 0.1;
									start2 += direction * 0.1;

									glm::dvec3 direction2 = glm::dvec3(newFoundNode->second.returnPos) - start;
									direction2.y = 0;

									if (glm::dot(direction, direction2) < 0.f) { break; }

									auto checkOneDirection = [&](glm::ivec3 blockPos)
									{
										blockPos.y += 1;
										auto b = serverChunkStorer.getBlockSafe(blockPos);
										if (b && b->isCollidable())
										{
											problems = true;
											return true;
										}
										return false;
									};

									auto pos1 = fromBlockPosToBlockPosInChunk(start);
									auto pos2 = fromBlockPosToBlockPosInChunk(start2);

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
									//if (!b2 || !b2->isCollidable())
									//{
									//	problems = true;
									//	break;
									//}

								}

								if (maxCounter <= 0) { break; }

								if (problems)
								{
									interPolateNodeNotGood = *newFoundNode;
									//break; //worse interpolation but less lickely to have problems
								}
								else
								{
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


		     	glm::vec3 d = glm::dvec3(interPolateNode.second.returnPos)
					- getPosition();
				d.y = 0;

				direction.x = d.x;
				direction.y = d.z;

				if (glm::length(direction) != 0)
				{
					direction = glm::normalize(direction);

					pathFindingSucceeded = true;
				}
			}

		};


		if(1)
		if (!pathFindingSucceeded)
		{
			//std::cout << "yess ";

			glm::vec3 d = playerLockedOnPosition
				- getPosition();
			d.y = 0;

			direction.x = d.x;
			direction.y = d.z;

			if (glm::length(direction) != 0)
			{
				direction = glm::normalize(direction);
			}

		}

	};


	if (!playerLockedOn)
	{
		waitTime -= deltaTime;

		if (waitTime < 0)
		{
			moving = getRandomNumber(rng, 0, 1);
			waitTime += getRandomNumberFloat(rng, 1, 8);

			if (moving)
			{
				direction = getRandomUnitVector(rng);
			}
		}
	};

	//jump
	{
		auto blockPos = getPosition();
		blockPos.x += direction.x;
		blockPos.z += direction.y;

		auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos));

		if (b && b->isCollidable())
		{

			//don't jump too tall walls lol
			auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos) + glm::ivec3(0,1,0));
			if (!b || !b->isCollidable())
			{
				auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos) + glm::ivec3(0, 2, 0));
				if (!b || !b->isCollidable())
				{
					entity.forces.jump();
				}
			}

		}
	}

	//don't fall into gaps when not following player
	if(0)
	if(!pathFindingSucceeded)
	{
		auto blockPos = getPosition();
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
		else if (!b->isCollidable())
		{

			//bigger fall under?
			blockPos.y--;
			auto b = serverChunkStorer.getBlockSafe(from3DPointToBlock(blockPos));

			if (!b || !b->isCollidable())
			{
				direction = {};
				waitTime = 0;
			}
		}


	}


	auto move = 2.f * deltaTime * direction;
	getPosition().x += move.x;
	getPosition().z += move.y;

	entity.update(deltaTime, chunkGetter);

	return true;
}

