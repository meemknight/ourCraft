#pragma once
#include "packet.h"
#include <deque>


struct Event
{
	EventId eventId = {};

	//todo this will be changed to player stats
	glm::dvec3 playerPos = {};

	glm::ivec3 blockPos = {};
	uint16_t originalBlock = 0;
	uint16_t newBlock = 0;

	int type = 0;

	enum
	{
		doNothing = 0, //this happens when the server overwrides your block placement
		iPlacedBlock
	};
};


struct UndoQueue
{

	EventId currentEventId = {1, 1};

	std::deque<Event> events;

	void addPlaceBlockEvent(glm::ivec3 pos, uint16_t old, uint16_t newType, glm::dvec3 playerPos);

};


