#pragma once 
#include <gameplay/physics.h>

struct DroppedItem
{
	glm::dvec3 position = {};
	glm::dvec3 lastPosition = {};
	MotionState forces = {};
	BlockType type = 0;
	unsigned char count = 0;
};

struct DroppedItemNetworked
{
	float restantTime = 0;
	DroppedItem item = {};

};

void updateDroppedItem(DroppedItem &item, float deltaTime, decltype(chunkGetterSignature) *chunkGetter);