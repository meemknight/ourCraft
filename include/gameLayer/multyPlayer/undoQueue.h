#pragma once
#include "packet.h"
#include <deque>


struct Event
{
	EventId eventId = {};

	//todo this will be changed to player stats
	glm::dvec3 playerPos = {};

	glm::ivec3 blockPos = {};
	glm::dvec3 doublePos = {};
	BlockType originalBlock = 0;
	BlockType newBlock = 0;
	std::uint64_t entityId = 0;

	int type = 0;

	enum
	{
		doNothing = 0, //this happens when the server overwrides your block placement
		iPlacedBlock,
		iDroppedItemFromInventory,
	};
};


struct UndoQueue
{

	EventId currentEventId = {1, 1};

	std::deque<Event> events;

	void addPlaceBlockEvent(glm::ivec3 pos, BlockType old, BlockType newType, glm::dvec3 playerPos);

	void addDropItemFromInventoryEvent(glm::dvec3 pos, glm::dvec3 playerPos, std::uint64_t entityId);

};


