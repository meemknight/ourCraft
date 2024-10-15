#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <chunk.h>



struct ColidableEntry
{
	std::uint64_t eid = 0;
	glm::dvec3 position = {};
	glm::vec3 collider = {};
};


struct PhysicalSettings
{

	float gravityModifier = 1;
	float sideFriction = 0;

};

constexpr static float MAX_AIR_DRAG = 12.f;
constexpr static float AIR_DRAG_COEFICIENT = 0.1f;
constexpr static float GRAVITY = -9.8 * 2.9;
constexpr static float MAX_ACCELERATION = 1000;
constexpr static float MAX_VELOCITY = 1000;
constexpr static float BASIC_JUMP_IMPULSE = 9.4;
constexpr static float OTHERS_PUSHING_YOU_FORCE = 0.7f;
constexpr static float OTHERS_PUSHING_YOU_BUAS_Y_DOWN = 0.1f;



ChunkData *chunkGetterSignature(glm::ivec2 chunkPos);

struct MotionState
{
	glm::vec3 velocity = {};
	glm::vec3 acceleration = {};
	unsigned char colides = {}; //front back top bottom left right

	bool colidesTopBottom() { return colidesTop() || colidesBottom(); }
	bool colidesLeftRight() { return colidesLeft() || colidesRight(); }
	bool colidesFrontBack() { return colidesFront() || colidesBack(); }

	bool colidesFront() { return colides & 0b100000; }
	bool colidesBack() { return colides &  0b010000; }
	bool colidesTop() { return colides &   0b001000; }
	bool colidesBottom() { return colides &0b000100; }
	bool colidesLeft() { return colides &  0b000010; }
	bool colidesRight() { return colides & 0b000001; }

	void setColidesFront(bool b) { colides ^= 0b100000; }
	void setColidesBack(bool b) { colides ^=  0b010000; }
	void setColidesTop(bool b) { colides ^=   0b001000; }
	void setColidesBottom(bool b) { colides ^=0b000100; }
	void setColidesLeft(bool b) { colides ^=  0b000010; }
	void setColidesRight(bool b) { colides ^= 0b000001; }

	void jump(float impulse = BASIC_JUMP_IMPULSE);
};


//returns false if chunk was not loaded
bool resolveConstrains(
	glm::dvec3 &pos, glm::dvec3 &lastPos,
	decltype(chunkGetterSignature) *chunkGetter,
	MotionState *forces, float deltaTime, glm::vec3 colliderSize, PhysicalSettings physicalSettings = {});

//returns false if chunk was not loaded
bool checkCollisionBrute(glm::dvec3 &pos, glm::dvec3 lastPos,
	decltype(chunkGetterSignature) *chunkGetter, MotionState *forces, float deltaTime,
	glm::vec3 colliderSize, PhysicalSettings physicalSettings);


glm::dvec3 performCollision(glm::dvec3 pos, glm::dvec3 lastPos, glm::vec3 size, glm::dvec3 delta,
	decltype(chunkGetterSignature) *chunkGetter, bool &chunkLoaded, MotionState *forces, float deltaTime,
	glm::vec3 &drag, PhysicalSettings physicalSettings);

void updateForces(glm::dvec3 &pos, MotionState &forces, float deltaTime, bool applyGravity, PhysicalSettings physicalSettings = {});

void updateForces(glm::dvec3 &pos, glm::vec3 &velocity, glm::vec3 &acceleration, float deltaTime, bool applyGravity, PhysicalSettings physicalSettings = {});

void applyImpulse(MotionState &force, glm::vec3 impulse, float mass = 1.f);


//others are already supposed to be colliding
void colideWithOthers(glm::dvec3 &pos, glm::vec3 collider, MotionState &forces, std::vector<ColidableEntry> &others);

bool boxColide(glm::dvec3 p1, glm::vec3 s1,
	glm::dvec3 p2, glm::vec3 s2);

bool pointInsideBox(glm::dvec3 p,
	glm::dvec3 box, glm::vec3 size, float delta);

bool boxColideBlock(glm::dvec3 p1, glm::vec3 s1, glm::ivec3 b);

bool boxColideBlockWithCollider(glm::dvec3 p1, glm::vec3 s1, glm::ivec3 b, BlockCollider collider);

bool lineIntersectBoxGetPos(glm::dvec3 start, glm::dvec3 dir, glm::dvec3 box, glm::dvec3 size,
	glm::dvec3 &outPos, float &outDist, int &outFace
);

bool lineIntersectBox(glm::dvec3 start, glm::dvec3 dir,
	glm::dvec3 box, glm::dvec3 size);

bool lineIntersectBoxMaxDistance(glm::dvec3 start, glm::dvec3 dir,
	glm::dvec3 box, glm::dvec3 size, float maxDistance, float &outIntersectDist);