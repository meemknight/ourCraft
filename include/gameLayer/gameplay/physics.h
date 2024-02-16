#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <chunk.h>

ChunkData *chunkGetterSignature(glm::ivec2 chunkPos);

struct RigidBody
{

	glm::dvec3 pos = {};
	glm::dvec3 lastPos = {};
	glm::vec3 colliderSize = {};

	void resolveConstrains(decltype(chunkGetterSignature) *chunkGetter);

	void checkCollisionBrute(glm::dvec3 &pos, glm::dvec3 lastPos, 
		decltype(chunkGetterSignature) *chunkGetter);

	glm::dvec3 performCollision(glm::dvec3 pos, glm::dvec3 lastPos, glm::vec3 size, glm::vec3 delta,
		decltype(chunkGetterSignature) *chunkGetter);

	void updateMove();

};

//this is the local player
struct Player
{
	glm::vec3 lookDirection = {0,0,-1};
	RigidBody body = {};
	std::uint64_t entityId = 0;

	void moveFPS(glm::vec3 direction);

	

};