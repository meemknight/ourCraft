#pragma once
#include "packet.h"
#include <deque>
#include <vector>


struct UndoQueueEvent
{
	EventId eventId = {};

	glm::ivec3 blockPos = {};
	glm::dvec3 doublePos = {};
	BlockType originalBlock = 0;
	BlockType newBlock = 0;
	std::uint64_t entityId = 0;
	std::uint64_t createTime = 0;

	std::vector<unsigned char> blockData;

	int type = 0;

	enum
	{
		doNothing = 0, //this happens when the server overwrides your block placement
		iPlacedBlock,
		iDroppedItemFromInventory,
	};

	void setTimer()
	{
		createTime = getTimer();
	}
};


struct UndoQueue
{

	EventId currentEventId = {1, 1};

	std::deque<UndoQueueEvent> events;

	void addPlaceBlockEvent(glm::ivec3 pos, BlockType old, BlockType newType);
	void addDataToLastBlockEvent(std::vector<unsigned char> &dataToSteal);

	void addDropItemFromInventoryEvent(glm::dvec3 pos, std::uint64_t entityId);

};


