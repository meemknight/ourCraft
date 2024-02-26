#include <gameplay/entityManagerClient.h>
#include <glm/glm.hpp>
#include <chunkSystem.h>
#include <iostream>
#include "multyPlayer/undoQueue.h"
#include <multyPlayer/createConnection.h>


bool checkIfPlayerShouldGetEntity(glm::ivec2 playerPos2D,
	glm::dvec3 entityPos, int playerSquareDistance, int extraDistance)
{
	glm::ivec2 ientityPos = {entityPos.x, entityPos.z};

	float dist = glm::length(glm::vec2(playerPos2D - ientityPos));

	if (dist > ((playerSquareDistance * CHUNK_SIZE) / 2.f) * std::sqrt(2.f) + 1 + extraDistance)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void ClientEntityManager::dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance)
{


	for (auto it = players.begin(); it != players.end(); )
	{
		if (!checkIfPlayerShouldGetEntity(playerPos2D, it->second.position, playerSquareDistance, 0))
		{
			it = players.erase(it);
			std::cout << "Dropped player\n";
		}
		else
		{
			++it;
		}
	}


	
	
}

std::uint64_t ClientEntityManager::consumeId()
{

	while (true)
	{
		if (reservedIds.empty()) { return 0; }

		if (reservedIds[0].count > 0)
		{
			auto rez = reservedIds[0].idStart;
			reservedIds[0].count--;
			reservedIds[0].idStart++;
			return rez;
		}
		else
		{
			reservedIds.pop_front();
		}

	}
	
	return 0;
}

bool ClientEntityManager::dropItemByClient(glm::dvec3 position, BlockType blockType, UndoQueue &undoQueue
	, glm::vec3 throwForce)
{

	std::uint64_t newEntityId = consumeId();

	if (!newEntityId) { return 0; }

	MotionState ms = {};
	ms.velocity = throwForce;

	Task task = {};
	task.type = Task::droppedItemEntity;
	task.doublePos = position + glm::dvec3{0,1,0};
	task.blockType = blockType;
	task.eventId = undoQueue.currentEventId;
	task.blockCount = 1;
	task.entityId = newEntityId;
	task.motionState = ms;
	submitTaskClient(task);


	undoQueue.addDropItemFromInventoryEvent(position + glm::dvec3{0,1,0}, position, newEntityId);

	{
		DroppedItem newEntity = {};
		newEntity.count = 1;
		newEntity.position = position;
		newEntity.lastPosition = position;
		newEntity.type = blockType;
		newEntity.forces = ms;

		droppedItems[newEntityId] = newEntity;
	}

	return true;
}

void ClientEntityManager::removeDroppedItem(std::uint64_t entityId)
{
	auto f = droppedItems.find(entityId);

	if(f != droppedItems.end())
	{
		droppedItems.erase(f);
	}
}
