#pragma once
#include "packet.h"
#include <deque>
#include <vector>
#include <blocks.h>


struct UndoQueueEvent
{
	EventId eventId = {};

	glm::ivec3 blockPos = {};
	glm::dvec3 doublePos = {};
	Block originalBlock = {};
	Block newBlock = {};
	std::uint64_t entityId = 0;
	std::uint64_t createTime = 0;

	std::vector<unsigned char> blockData;

	int type = 0;

	enum
	{
		doNothing = 0, //this happens when the server overwrides your block placement
		iPlacedBlock,
		iDroppedItemFromInventory,
		changedBlockData,
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

	void addPlaceBlockEvent(glm::ivec3 pos, Block old, Block newType);
	void changedBlockDataEvent(glm::ivec3 pos, Block block, std::vector<unsigned char> &dataToSteal);
	void addDataToLastBlockEvent(std::vector<unsigned char> &dataToSteal);

	void addDropItemFromInventoryEvent(glm::dvec3 pos, std::uint64_t entityId);

};


