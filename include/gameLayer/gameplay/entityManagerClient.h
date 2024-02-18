#pragma once
#include <gameplay/physics.h>
#include <unordered_map>



struct PlayerNetworked
{
	glm::dvec3 position = {};

	//todo rotation and others

	
};


struct ClientEntityManager
{

	std::unordered_map<std::uint64_t, PlayerNetworked> players;
	Player localPlayer;

	void dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance);


};


bool checkIfPlayerShouldGetEntity(glm::ivec2 playerPos2D,
	glm::dvec3 entityPos, int playerSquareDistance, int extraDistance);


