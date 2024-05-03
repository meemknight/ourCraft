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
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding
)
{

	direction = {0,0};
	if (!pathFinding.empty())
	{

		auto &path = *pathFinding.begin();

		auto pos = from3DPointToBlock(getPosition());

		auto foundNode = path.second.find(pos);

		if (foundNode != path.second.end())
		{

			std::pair<glm::ivec3, PathFindingNode> interPolateNode = *foundNode;
			std::pair<glm::ivec3, PathFindingNode> interPolateNodeNotGood = *foundNode;

			int originalHeight = pos.y;

			int maxCounterLoop = 100;
			while (maxCounterLoop > 0) 
			{
				maxCounterLoop--;

				if (originalHeight != interPolateNodeNotGood.second.returnPos.y)
				{
					break;
				}

				auto newFoundNode = path.second.find(interPolateNodeNotGood.second.returnPos);

				if (newFoundNode != path.second.end())
				{
					if (newFoundNode->second.level > 0)
					{
						
						glm::dvec3 direction = glm::dvec3(newFoundNode->second.returnPos) - getPosition();

						direction.y = 0;
						direction = glm::normalize(direction);

						bool problems = 0;

						//try interpolation
						int maxCounter = 100;//so we don't get infinite loops
						for (glm::dvec3 start = getPosition(); maxCounter>0;maxCounter--)
						{
							start += direction * 0.6;

							glm::dvec3 direction2 = glm::dvec3(newFoundNode->second.returnPos) - start;
							direction2.y = 0;

							if (glm::dot(direction, direction2) < 0.f) { break; }

							glm::ivec3 blockPos = fromBlockPosToBlockPosInChunk(start);
							blockPos.y += 1;

							auto b = serverChunkStorer.getBlockSafe(blockPos);
							if (b && b->isColidable())
							{
								problems = true;
								break;
							}
							
							//blockPos.y -= 2;
							//auto b2 = serverChunkStorer.getBlockSafe(blockPos);
							//if (!b2 || !b2->isColidable())
							//{
							//	problems = true;
							//	break;
							//}

						}
						
						if (problems)
						{
							interPolateNodeNotGood = *newFoundNode;
						}
						else
						{
							interPolateNodeNotGood = *newFoundNode;
							interPolateNode = *newFoundNode;
						}

						if (maxCounter <= 0) { break; }

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
			}
		}

	}


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

	auto move = 2.f * deltaTime * direction;
	getPosition().x += move.x;
	getPosition().z += move.y;

	entity.update(deltaTime, chunkGetter);

	return true;
}

