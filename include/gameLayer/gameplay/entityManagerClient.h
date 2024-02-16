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

	


};