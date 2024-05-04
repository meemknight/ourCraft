#include <gameplay/entityManagerClient.h>
#include <glm/glm.hpp>
#include <chunkSystem.h>
#include <iostream>
#include "multyPlayer/undoQueue.h"
#include <multyPlayer/createConnection.h>
#include <utility>



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


template<class T>
void genericDropEntitiesThatAreTooFar(T &container, glm::ivec2 playerPos2D, int playerSquareDistance)
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

template <int... Is>
void callGenericDropEntitiesThatAreTooFar(std::integer_sequence<int, Is...>, ClientEntityManager &c,
	glm::ivec2 playerPos2D, int playerSquareDistance)
{
	(genericDropEntitiesThatAreTooFar(*c.template entityGetter<Is>(), playerPos2D, playerSquareDistance), ...);
}

void ClientEntityManager::dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance)
{

	//auto doChecking = [&](auto &container)
	//{
	//	for (auto it = container.begin(); it != container.end(); )
	//	{
	//		if (!checkIfPlayerShouldGetEntity(playerPos2D, it->second.getPosition(),
	//			playerSquareDistance, 0))
	//		{
	//			it = container.erase(it);
	//		}
	//		else
	//		{
	//			++it;
	//		}
	//	}
	//};
	//
	//doChecking(players);
	//doChecking(droppedItems);
	//doChecking(zombies);
	//doChecking(pigs);

	callGenericDropEntitiesThatAreTooFar(std::make_integer_sequence<int, EntitiesTypesCount>(),
		*this,
		playerPos2D, playerSquareDistance);

	
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



template<class T>
bool genericRemoveEntity(T &container, std::uint64_t entityId)
{
	auto found = container.find(entityId);
	if (found != container.end())
	{
		container.erase(found);
		return true;
	}
	else
	{
		return false;
	}
};

//this can't remove the player
template <int... Is>
void callGenericRemoveEntity(std::integer_sequence<int, Is...>, ClientEntityManager &c,
	std::uint64_t entityId)
{
	bool stopCalling = false;
	((stopCalling = genericRemoveEntity(*c.template entityGetter<Is+1>(), entityId)) || ...);
}

void ClientEntityManager::removeEntity(std::uint64_t entityId)
{

	//auto tryRemove = [&](auto &container)
	//{
	//	auto found = container.find(entityId);
	//	if (found != container.end())
	//	{
	//		container.erase(found);
	//		return true;
	//	}
	//	else
	//	{
	//		return false;
	//	}
	//};
	//
	//if (!tryRemove(droppedItems))
	//if (!tryRemove(zombies))
	//if (!tryRemove(pigs))
	//{}

	//this can't remove the player
	callGenericRemoveEntity(std::make_integer_sequence<int, EntitiesTypesCount-1>(), 
		*this, entityId);


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
			.addToRubberBand(found->second.entity.position - droppedItem.position);

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
			.addToRubberBand(found->second.entity.position - entity.position);

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
			.addToRubberBand(found->second.entity.position - entity.position);

		found->second.entity = entity;
		found->second.restantTime = restantTimer;
	}
}



template<class T>
void genericUpdateLoop(T &container, float deltaTime, ChunkData *(chunkGetter)(glm::ivec2))
{
	for (auto &e : container)
	{
		e.second.clientEntityUpdate(deltaTime, chunkGetter);
	}
};

template <int... Is>
void callGenericUpdateLoop(std::integer_sequence<int, Is...>, float deltaTime,
	ChunkData *(chunkGetter)(glm::ivec2), ClientEntityManager &c)
{
	(genericUpdateLoop(*c.template entityGetter<Is>(), deltaTime, chunkGetter),  ...);
}


void ClientEntityManager::doAllUpdates(float deltaTime, ChunkData *(chunkGetter)(glm::ivec2))
{


	callGenericUpdateLoop(std::make_integer_sequence<int, EntitiesTypesCount>(), 
		deltaTime, chunkGetter, *this);



}







