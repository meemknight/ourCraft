#include "multyPlayer/undoQueue.h"


void UndoQueue::addPlaceBlockEvent(glm::ivec3 pos, uint16_t old, uint16_t newType, glm::dvec3 playerPos)
{
	Event e;
	e.type = Event::iPlacedBlock;
	e.eventId.counter = currentEventId.counter++;
	e.eventId.revision = currentEventId.revision;

	e.blockPos = pos;
	e.originalBlock = old;
	e.newBlock = newType;

	e.playerPos = playerPos;

	events.push_back(e);
}
