#pragma once
#include <gameplay/entity.h>
#include <multyPlayer/server.h>


//this is the shared data
struct Player : public PhysicalEntity
{

	//todo will probably remove the idea of a rigid body
	glm::vec3 lookDirection = {0,0,-1};

	void moveFPS(glm::vec3 direction);



};

//this is the player struct when playing locally
struct LocalPlayer : public Player
{
	std::uint64_t entityId = 0;

	//dodo add some other data here like inventory

};

struct PlayerClient: public CleintEntity<Player>
{
	//todo remove from here?
	std::uint64_t entityId = 0;

	CID cid = 0;
};


//this is what we send through the network
struct PlayerData
{
	int chunkDistance = 10; //remove this from here ?
	glm::dvec3 position = {};
	//...
};
