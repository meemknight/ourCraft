#pragma once
#include <gameplay/entity.h>
#include <multyPlayer/server.h>




struct PlayerNetworked
{
	glm::dvec3 position = {};
	//todo rotation and others
	CID cid = 0;
};

struct PlayerClient
{
	glm::dvec3 position = {};
	CID cid = 0;
};