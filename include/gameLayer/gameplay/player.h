#pragma once
#include <gameplay/entity.h>
#include <multyPlayer/server.h>


//this is the shared data
struct Player : public PhysicalEntity
{

	glm::vec3 lookDirectionAnimation = {0,0,-1};
	glm::vec2 bodyOrientation = {0, -1};

	void flyFPS(glm::vec3 direction, glm::vec3 lookDirection);

	void moveFPS(glm::vec3 direction, glm::vec3 lookDirection);


};



//this is the player struct when playing locally
struct LocalPlayer : public Player
{
	std::uint64_t entityId = 0;

	//dodo add some other data here like inventory

};

struct PlayerClient: public ClientEntity<Player>
{



};


//this is what we send through the network
struct PlayerData
{
	glm::vec3 lookDirection = {0,0,-1};
	glm::vec2 bodyOrientation = {0, -1};

	int chunkDistance = 10; //remove this from here?
	glm::dvec3 position = {};
	//...
};
