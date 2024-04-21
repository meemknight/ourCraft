#include <gameplay/entityManagerClient.h>
#include <glm/glm.hpp>
#include <chunkSystem.h>
#include <iostream>
#include "multyPlayer/undoQueue.h"
#include <multyPlayer/createConnection.h>



bool checkIfPlayerShouldGetEntity(glm::ivec2 playerPos2D,
	glm::dvec3 entityPos, int playerSquareDistance, int extraDistance)
{

	glm::ivec2 pos(divideChunk(playerPos2D.x),
		divideChunk(playerPos2D.y));

	int distance = (playerSquareDistance/2) + extraDistance;

	glm::ivec2 entityPosI(divideChunk(entityPos.x),
		divideChunk(entityPos.z));
	
	glm::dvec2 difference = glm::dvec2(entityPosI - pos);

	return glm::length(difference) <= distance;
	
}

void ClientEntityManager::dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance)
{

	auto doChecking = [&](auto &container)
	{
		for (auto it = container.begin(); it != container.end(); )
		{
			if (!checkIfPlayerShouldGetEntity(playerPos2D, it->second.getPosition(),
				playerSquareDistance, 0))
			{
				it = container.erase(it);
			}
			else
			{
				++it;
			}
		}
	};

	doChecking(players);
	doChecking(droppedItems);
	doChecking(zombies);
	doChecking(pigs);

	
}

std::uint64_t ClientEntityManager::consumeId()
{
	auto rez = idCounter;
	idCounter++;
	if (idCounter >= RESERVED_CLIENTS_ID)
	{
		idCounter = 1;
	}
	return rez;
}

bool ClientEntityManager::dropItemByClient(glm::dvec3 position, BlockType blockType, UndoQueue &undoQueue
	, glm::vec3 throwForce, std::uint64_t timer)
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
	task.timer = timer;
	submitTaskClient(task);


	undoQueue.addDropItemFromInventoryEvent(position + glm::dvec3{0,1,0}, position, newEntityId);

	{
		DroppedItem newEntity = {};
		newEntity.count = 1;
		newEntity.position = position;
		newEntity.lastPosition = position;
		newEntity.type = blockType;
		newEntity.forces = ms;

		droppedItems[newEntityId].entity = newEntity;
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

void ClientEntityManager::addOrUpdateDroppedItem(std::uint64_t eid,
	DroppedItem droppedItem, UndoQueue &undoQueue,
	float restantTimer)
{

	auto found = droppedItems.find(eid);

	if (found == droppedItems.end())
	{	
		droppedItems[eid].entity = droppedItem;
		droppedItems[eid].restantTime = restantTimer;
	}
	else
	{
		found->second.rubberBand
			.add(found->second.entity.position - droppedItem.position);

		found->second.entity = droppedItem;
		found->second.restantTime = restantTimer;

		for (auto &e : undoQueue.events)
		{
			if (e.type == Event::iDroppedItemFromInventory && e.entityId == eid)
			{
				e.type = Event::doNothing;
			}
		}

	}

}

void ClientEntityManager::addOrUpdateZombie(std::uint64_t eid, Zombie entity,
	float restantTimer)
{
	auto found = zombies.find(eid);

	if (found == zombies.end())
	{
		zombies[eid].entity = entity;
		zombies[eid].restantTime = restantTimer;
	}
	else
	{
		found->second.rubberBand
			.add(found->second.entity.position - entity.position);

		found->second.entity = entity;
		found->second.restantTime = restantTimer;
	}
}

void ClientEntityManager::addOrUpdatePig(std::uint64_t eid, Pig entity, float restantTimer)
{
	auto found = pigs.find(eid);

	if (found == pigs.end())
	{
		pigs[eid].entity = entity;
		pigs[eid].restantTime = restantTimer;
	}
	else
	{
		found->second.rubberBand
			.add(found->second.entity.position - entity.position);

		found->second.entity = entity;
		found->second.restantTime = restantTimer;
	}
}

void ClientEntityManager::doAllUpdates(float deltaTime, ChunkData *(chunkGetter)(glm::ivec2))
{

	auto genericUpdate = [&](auto &entity)
	{
		float timer = deltaTime + entity.second.restantTime;

		if (timer > 0)
		{
			entity.second.update(timer, chunkGetter);
		}

		entity.second.rubberBand.computeRubberBand(entity.second.entity.position, deltaTime);

		entity.second.restantTime = 0;
	};

	auto genericUpdateLoop = [&](auto &container)
	{
		for (auto &e : container)
		{
			genericUpdate(e);
		}
	};


	for (auto &player : players)
	{

		player.second.rubberBand.computeRubberBand(
			player.second.entity.position, deltaTime);

	}

	genericUpdateLoop(droppedItems);
	genericUpdateLoop(zombies);
	genericUpdateLoop(pigs);

}
