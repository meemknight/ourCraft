#include "multyPlayer/undoQueue.h"


void UndoQueue::addPlaceBlockEvent(glm::ivec3 pos, Block old, Block newType, 
	std::vector<unsigned char> data)
{
	UndoQueueEvent e;
	e.setTimer();
	e.type = UndoQueueEvent::iPlacedBlock;
	e.eventId.counter = currentEventId.counter++;
	e.eventId.revision = currentEventId.revision;

	e.blockPos = pos;
	e.originalBlock = old;
	e.newBlock = newType;
	e.blockData = std::move(data);

	events.push_back(e);
}

void UndoQueue::changedBlockDataEvent(glm::ivec3 pos, Block block, 
	std::vector<unsigned char> &dataToSteal)
{
	UndoQueueEvent e;
	e.setTimer();
	e.type = UndoQueueEvent::changedBlockData;
	e.eventId.counter = currentEventId.counter++;
	e.eventId.revision = currentEventId.revision;

	e.blockPos = pos;
	e.originalBlock = block;
	e.blockData = std::move(dataToSteal);

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
