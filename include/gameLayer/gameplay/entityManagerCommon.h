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


struct RubberBand
{
	glm::dvec3 startPosition = {};
	glm::dvec3 position = {};
	float timer = 0; //the timer should be one for now

	void computeRubberBand(glm::dvec3 &position, float deltaTime);
};


void computeRubberBand(
	RubberBand &rubberBand,
	glm::dvec3 &position, float deltaTime);


struct DroppedItemNetworked
{
	RubberBand rubberBand = {};
	float restantTime = 0;
	DroppedItem item = {};

};

constexpr static int simulationDistance = 6;

void updateDroppedItem(DroppedItem &item, float deltaTime, decltype(chunkGetterSignature) *chunkGetter);