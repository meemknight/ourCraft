#pragma once
#include <gameplay/entity.h>
#include <multyPlayer/server.h>

//this is the shared data
struct Player_ : public PhysicalEntity
{

	//todo will probably remove the idea of a rigid body
	glm::vec3 lookDirection = {0,0,-1};

	void moveFPS(glm::vec3 direction);



};

struct LocalPlayer : public Player_
{
	std::uint64_t entityId = 0;

	//dodo add some other data here like inventory

};

struct PlayerNetworked
{
	//todo remove from here?
	std::uint64_t entityId = 0;

	glm::dvec3 position = {};
	//todo rotation and others
	CID cid = 0;
};


struct PlayerClient: public CleintEntity<Player_>
{
	
	CID cid = 0;
};