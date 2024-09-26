#include "multyPlayer/undoQueue.h"


void UndoQueue::addPlaceBlockEvent(glm::ivec3 pos, Block old, Block newType)
{
	UndoQueueEvent e;
	e.setTimer();
	e.type = UndoQueueEvent::iPlacedBlock;
	e.eventId.counter = currentEventId.counter++;
	e.eventId.revision = currentEventId.revision;

	e.blockPos = pos;
	e.originalBlock = old;
	e.newBlock = newType;

	events.push_back(e);
}

void UndoQueue::addDataToLastBlockEvent(std::vector<unsigned char> &dataToSteal)
{
	events.back().blockData = std::move(dataToSteal);
}

void UndoQueue::addDropItemFromInventoryEvent(glm::dvec3 pos, std::uint64_t entityId)
{
	UndoQueueEvent e;
	e.setTimer();
	e.type = UndoQueueEvent::iDroppedItemFromInventory;
	e.eventId.counter = currentEventId.counter++;
	e.eventId.revision = currentEventId.revision;

	e.entityId = entityId;
	e.doublePos = pos;

	events.push_back(e);
}
