#include "multyPlayer/undoQueue.h"


void UndoQueue::addPlaceBlockEvent(glm::ivec3 pos, BlockType old, BlockType newType, glm::dvec3 playerPos)
{
	Event e;
	e.setTimer();
	e.type = Event::iPlacedBlock;
	e.eventId.counter = currentEventId.counter++;
	e.eventId.revision = currentEventId.revision;

	e.blockPos = pos;
	e.originalBlock = old;
	e.newBlock = newType;

	e.playerPos = playerPos;

	events.push_back(e);
}

void UndoQueue::addDropItemFromInventoryEvent(glm::dvec3 pos, glm::dvec3 playerPos, std::uint64_t entityId)
{
	Event e;
	e.setTimer();
	e.type = Event::iDroppedItemFromInventory;
	e.eventId.counter = currentEventId.counter++;
	e.eventId.revision = currentEventId.revision;

	e.entityId = entityId;
	e.doublePos = pos;
	e.playerPos = playerPos;

	events.push_back(e);
}
