#include <gameplay/entityManagerClient.h>
#include <glm/glm.hpp>
#include <chunkSystem.h>
#include <iostream>


bool checkIfPlayerShouldGetEntity(glm::ivec2 playerPos2D,
	glm::dvec3 entityPos, int playerSquareDistance, int extraDistance)
{
	glm::ivec2 ientityPos = {entityPos.x, entityPos.z};

	float dist = glm::length(glm::vec2(playerPos2D - ientityPos));

	if (dist > ((playerSquareDistance * CHUNK_SIZE) / 2.f) * std::sqrt(2.f) + 1 + extraDistance)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void ClientEntityManager::dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance)
{


	for (auto it = players.begin(); it != players.end(); )
	{
		if (!checkIfPlayerShouldGetEntity(playerPos2D, it->second.position, playerSquareDistance, 0))
		{
			it = players.erase(it);
			std::cout << "Dropped player\n";
		}
		else
		{
			++it;
		}
	}


	
	
}