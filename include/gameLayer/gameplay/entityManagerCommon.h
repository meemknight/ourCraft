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