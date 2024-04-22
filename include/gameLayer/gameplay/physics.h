#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <chunk.h>

ChunkData *chunkGetterSignature(glm::ivec2 chunkPos);

struct MotionState
{
	glm::vec3 velocity = {};
	glm::vec3 acceleration = {};
	unsigned char colides = {}; //front back top bottom left right

	bool colidesTopBottom() { return colidesTop() || colidesBottom(); }
	bool colidesLeftRight() { return colidesLeft() || colidesRight(); }
	bool colidesFrontBack() { return colidesFront() || colidesBack(); }

	bool colidesFront() { return colides && 0b1; }
	bool colidesBack() { return colides && 0b01; }
	bool colidesTop() { return colides && 0b001; }
	bool colidesBottom() { return colides && 0b0001; }
	bool colidesLeft() { return colides && 0b00001; }
	bool colidesRight() { return colides && 0b000001; }

	void setColidesFront(bool b) { colides ^= 0b1; }
	void setColidesBack(bool b) { colides ^= 0b01; }
	void setColidesTop(bool b) { colides ^= 0b001; }
	void setColidesBottom(bool b) { colides ^= 0b0001; }
	void setColidesLeft(bool b) { colides ^= 0b00001; }
	void setColidesRight(bool b) { colides ^= 0b000001; }

};


//returns false if chunk was not loaded
bool resolveConstrains(
	glm::dvec3 &pos, glm::dvec3 &lastPos,
	decltype(chunkGetterSignature) *chunkGetter,
	MotionState *forces, float deltaTime, glm::vec3 colliderSize);

//returns false if chunk was not loaded
bool checkCollisionBrute(glm::dvec3 &pos, glm::dvec3 lastPos,
	decltype(chunkGetterSignature) *chunkGetter, MotionState *forces, float deltaTime,
	glm::vec3 colliderSize);


glm::dvec3 performCollision(glm::dvec3 pos, glm::dvec3 lastPos, glm::vec3 size, glm::dvec3 delta,
	decltype(chunkGetterSignature) *chunkGetter, bool &chunkLoaded, MotionState *forces, float deltaTime,
	glm::vec3 &drag);

void updateForces(glm::dvec3 &pos, MotionState &forces, float deltaTime, bool applyGravity);

void updateForces(glm::dvec3 &pos, glm::vec3 &velocity, glm::vec3 &acceleration, float deltaTime, bool applyGravity);

void applyImpulse(MotionState &force, glm::vec3 impulse, float mass = 1.f);

