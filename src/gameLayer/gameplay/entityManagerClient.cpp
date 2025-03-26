#include <gameplay/entityManagerClient.h>
#include <glm/glm.hpp>
#include <chunkSystem.h>
#include <iostream>
#include "multyPlayer/undoQueue.h"
#include <multyPlayer/createConnection.h>
#include <utility>
#include <gameplay/items.h>
#include <platformTools.h>
#include <repeat.h>


template<class T>
void genericDropEntitiesThatAreTooFar(T &container, glm::ivec2 playerPos2D, int playerSquareDistance)
{

	//no need to drop entities that are based on block position because we drop them with the chunk
	if (hasPositionBasedID<decltype((container)[0].entityBuffered)>) { return; }

	for (auto it = container.begin(); it != container.end(); )
	{
		auto entityPos = it->second.getPosition();
		glm::ivec2 entityChunkPos = glm::ivec2(divideChunk(entityPos.x), divideChunk(entityPos.z));

		if (!isChunkInRadius(playerPos2D, entityChunkPos, playerSquareDistance))
		{

			if constexpr (hasCleanup<decltype(it->second)>)
			{
				it->second.cleanup();
			}

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

std::uint64_t ClientEntityManager::consumeId(unsigned short entityType)
{
	auto rez = entityIds[entityType];
	entityIds[entityType]++;
	if (entityIds[entityType] >= RESERVED_CLIENTS_ID)
	{
		entityIds[entityType] = 1;
	}

	rez |= ((std::uint64_t)((unsigned char)entityType)) << 56;

	return rez;
}

bool ClientEntityManager::dropItemByClient(
	glm::dvec3 position,
	unsigned char inventorySlot, UndoQueue &undoQueue
	, glm::vec3 throwForce, std::uint64_t timer, PlayerInventory &inventory, int count)
{

	auto from = inventory.getItemFromIndex(inventorySlot);
	if (!from) { return 0; }

	if (!from->type) { return 0; }
	
	if (count > from->counter) { return 0; }

	if (count == 0) { count = from->counter; }

	std::uint64_t newEntityId = consumeId(EntityType::droppedItems);

	if (!newEntityId) { return 0; }

	MotionState ms = {};
	ms.velocity = throwForce;

	//Task task = {};
	//task.type = Task::droppedItemEntity;
	//task.doublePos = position + glm::dvec3{0,1,0};
	//task.blockType = blockType;
	//task.eventId = undoQueue.currentEventId;
	//task.blockCount = 1;
	//task.entityId = newEntityId;
	//task.motionState = ms;
	//task.timer = timer;
	//submitTaskClient(task);
	Packet p;
	p.cid = getConnectionData().cid;
	p.header = headerClientDroppedItem;

	Packet_ClientDroppedItem packetData = {};
	packetData.position = position + glm::dvec3(0, 1.5, 0);
	packetData.inventorySlot = inventorySlot;
	packetData.count = count;
	packetData.eventId = undoQueue.currentEventId;
	packetData.entityID = newEntityId;
	packetData.motionState = ms;
	packetData.timer = timer;
	packetData.type = from->type;
	packetData.revisionNumberInventory = inventory.revisionNumber;

	//std::cout << "My revision : " << (int)inventory.revisionNumber << "\n";

	sendPacket(getServer(), p, (char *)&packetData, sizeof(packetData), true,
		channelChunksAndBlocks);

	undoQueue.addDropItemFromInventoryEvent(position + glm::dvec3(0, 1.5, 0), newEntityId);

	{
		//todo add meta data here

		DroppedItem newEntity = {};
		newEntity.count = count;
		newEntity.position = position + glm::dvec3(0, 1.5, 0);
		newEntity.lastPosition = position + glm::dvec3(0, 1.5, 0);
		newEntity.type = from->type;
		newEntity.forces = ms;

		droppedItems[newEntityId].entityBuffered = newEntity;
	}

	from->counter -= count;
	if (!from->counter) { *from = {}; }

	return true;
}



template<class T>
bool genericRemoveEntity(T &container, std::uint64_t entityId)
{
	auto found = container.find(entityId);
	if (found != container.end())
	{
		//todo we should check everytime we delete an entity.
		if constexpr (hasCleanup<decltype(found->second)>)
		{
			found->second.cleanup();
		}

		container.erase(found);
		return true;
	}
	else
	{
		return false;
	}
};

#define CASE_REMOVE(x) case x: { genericRemoveEntity(*c.template entityGetter<x>(), entityId); } break;

void callGenericRemoveEntity(ClientEntityManager &c,
	std::uint64_t entityId)
{

	auto entityType = getEntityTypeFromEID(entityId);

	switch (entityType)
	{
		REPEAT_FOR_ALL_ENTITIES(CASE_REMOVE);


	default:;
	}
}
#undef CASE_REMOVE


void ClientEntityManager::removeEntity(std::uint64_t entityId)
{
	unsigned char entityType = getEntityTypeFromEID(entityId);

	permaAssertComment(entityType != 0, "remove entity can't remove players!");
	
	//this can't remove the player
	callGenericRemoveEntity(*this, entityId);
}


#pragma region kill entity

template<class T>
bool genericKillEntity(T &container, std::uint64_t entityId)
{
	auto found = container.find(entityId);
	if (found != container.end())
	{
		if constexpr (hasLife<decltype(found->second)>)
		{
			found->second.life.life = 0;
		}

		found->second.wasKilled = true;
		found->second.wasKilledTimer = 1.5;

		return true;
	}
	else
	{
		return false;
	}
};

#define CASE_KILL(x) case x: { genericKillEntity(*c.template entityGetter<x>(), entityId); } break;

void callGenericKillEntity(ClientEntityManager &c,
	std::uint64_t entityId)
{

	auto entityType = getEntityTypeFromEID(entityId);

	//static_assert(5 == EntitiesTypesCount);

	switch (entityType)
	{
		REPEAT_FOR_ALL_ENTITIES(CASE_KILL);
		//REPEAT(CASE_KILL, 5);
		//case 5: {  }

	default:;
	}
}
#undef CASE_KILL

void ClientEntityManager::killEntity(std::uint64_t entityId)
{
	callGenericKillEntity(*this, entityId);
}

#pragma endregion


void ClientEntityManager::removePlayer(std::uint64_t entityId)
{
	auto found = players.find(entityId);

	if (found != players.end())
	{
		players.erase(found);
	}
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
	float restantTimer, std::uint64_t serverTimer, std::uint64_t timeUpdatedOnServer)
{

	auto found = droppedItems.find(eid);

	if (found == droppedItems.end())
	{	
		droppedItems[eid].entityBuffered = droppedItem;
		droppedItems[eid].restantTime = restantTimer; //todo I think we can safely remove this 
	}
	else
	{
		//found->second.rubberBand
		//	.addToRubberBand(found->second.entityBuffered.position - droppedItem.position);	

		found->second.bufferedEntityData.addElement(droppedItem, serverTimer, timeUpdatedOnServer);
		//found->second.entity = droppedItem;
		found->second.restantTime = restantTimer;

		for (auto &e : undoQueue.events)
		{
			if (e.type == UndoQueueEvent::iDroppedItemFromInventory && e.entityId == eid)
			{
				e.type = UndoQueueEvent::doNothing;
			}
		}

	}

}


template<int TYPE, class T>
void genericUpdateLoop(T &container, float deltaTime, ChunkData *(chunkGetter)(glm::ivec2),
	std::uint64_t serverTimer)
{

	for (auto it = container.begin(); it != container.end();)
	{

		if constexpr (TYPE != EntityType::player)
		{
			if constexpr (hasCanBeKilled<decltype(it->second.entityBuffered)>)
			{
				if (it->second.wasKilled)
				{
					it->second.wasKilledTimer -= deltaTime;
					if (it->second.wasKilledTimer <= 0)
					{

						if constexpr (hasCleanup<decltype(it->second)>)
						{
							it->second.cleanup();
						}

						// Remove the element and advance the iterator
						it = container.erase(it);
						continue; // Skip the increment since erase already moved the iterator
					}
				}
			}
		}
		else
		{
			//we don't remove the player because ther's no need to
			if (it->second.wasKilled)
			{
				it->second.wasKilledTimer -= deltaTime;
				if (it->second.wasKilledTimer <= 0)
				{
					it->second.wasKilledTimer = 0;
					++it;
					//continue;
				}
			}
		}

		// If the element wasn't erased, update it
		it->second.clientEntityUpdate(deltaTime, chunkGetter, serverTimer);
		++it; // Manually increment the iterator
	}
};

template <int... Is>
void callGenericUpdateLoop(std::integer_sequence<int, Is...>, float deltaTime,
	ChunkData *(chunkGetter)(glm::ivec2), ClientEntityManager &c, std::uint64_t serverTimer)
{
	(genericUpdateLoop<Is>(*c.template entityGetter<Is>(), deltaTime, chunkGetter, serverTimer),  ...);
}


void ClientEntityManager::doAllUpdates(float deltaTime, ChunkData *(chunkGetter)(glm::ivec2), std::uint64_t serverTimer)
{
	
	//entityGetter<3>()->at(0).wasKilled = 0;

	callGenericUpdateLoop(std::make_integer_sequence<int, EntitiesTypesCount>(), 
		deltaTime, chunkGetter, *this, serverTimer);


}

void ClientEntityManager::cleanup()
{
	//todo if we keep the cleanup thing on entities, here is where we shold call it!
	//or else remove this

}


template<class T>
std::uint64_t genericIntersectAllAttackableEntities(T &container, glm::dvec3 start,
	glm::dvec3 dir, float maxDistance, float &outIntersectDist, float delta)
{
	if constexpr (!hasCanBeAttacked<decltype(container.begin()->second.entityBuffered)>)
	{
		return 0;
	}

	for (auto &e : container)
	{
		auto collider = e.second.entityBuffered.getColliderSize();

		glm::dvec3 position = {};

		if constexpr (hasPositionBasedID<decltype((container)[0].entityBuffered)>)
		{
			position = fromEntityIDToBlockPos(e.first);
			position.y -= 0.5;
		}
		else
		{
			position = e.second.getRubberBandPosition();
		}

		bool rez = lineIntersectBoxMaxDistance(start, dir, position, collider,
			maxDistance, outIntersectDist, delta);

		if (rez) { return e.first; }
	}

	return 0;
}


template <int... Is>
std::uint64_t callGenericIntersectAllAttackableEntities(std::integer_sequence<int, Is...>, ClientEntityManager &c,
	glm::dvec3 start, glm::dvec3 dir, float maxDistance, float &outIntersectDist,
	float delta
)
{
	std::uint64_t result = 0;

	// Lambda function to evaluate and capture the result
	auto evaluate = [&](auto &entity) -> bool
	{
		result = genericIntersectAllAttackableEntities(entity, start, dir, maxDistance,
			outIntersectDist, delta);
		return result != 0;
	};

	// Fold expression with short-circuiting
	(evaluate(*c.template entityGetter<Is>()) || ...);

	return result;

	//return genericIntersectAllAttackableEntities(c.zombies,
	//	start, dir, maxDistance);
}



std::uint64_t ClientEntityManager::intersectAllAttackableEntities(glm::dvec3 start, 
	glm::dvec3 dir, float maxDistance, float &outDistance, float delta)
{
	return callGenericIntersectAllAttackableEntities(
		std::make_integer_sequence<int, EntitiesTypesCount>(), *this,
		start, dir, maxDistance, outDistance, delta);

}


template<int TYPE, class T>
void genericRenderColliders(T &container,
	PointDebugRenderer &pointDebugRenderer, GyzmosRenderer &gyzmosRenderer, Camera &camera)
{

	auto drawBox = [&](glm::dvec3 pos, glm::vec3 boxSize)
	{
		gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2));

		gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2));

		gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2));
		gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2));
	};

	for (auto &p : container)
	{
		pointDebugRenderer.
			renderPoint(camera, p.second.getRubberBandPosition());

		glm::dvec3 position = {};

		if constexpr (hasPositionBasedID<decltype((container)[0].entityBuffered)>)
		{
			position = fromEntityIDToBlockPos(p.first);
			position.y -= 0.5;
		}
		else
		{
			position = p.second.getRubberBandPosition();
		}

		auto boxSize = p.second.entityBuffered.getColliderSize();

		drawBox(position, boxSize);
	}

}


template <int... Is>
void callGenericRenderColliders(std::integer_sequence<int, Is...>, ClientEntityManager &c,
	PointDebugRenderer &pointDebugRenderer, GyzmosRenderer &gyzmosRenderer, Camera &camera
)
{
	(genericRenderColliders<Is>(*c.template entityGetter<Is>(), 
		pointDebugRenderer, gyzmosRenderer, camera), ...);

}



void ClientEntityManager::renderColiders(PointDebugRenderer &pointDebugRenderer, GyzmosRenderer &gyzmosRenderer, Camera &c)
{

	return callGenericRenderColliders(
		std::make_integer_sequence<int, EntitiesTypesCount>(), *this,
		pointDebugRenderer, gyzmosRenderer, c);


}

void ClientEntityManager::removeBlockEntity(glm::ivec3 pos, BlockType blockType)
{

	if (blockType == BlockTypes::trainingDummy)
	{
		removeEntityBasedOnBlockPosition<EntityType::trainingDummy>(pos.x, pos.y, pos.z);
	}

}

void ClientEntityManager::addBlockEntity(glm::ivec3 pos, BlockType blockType)
{

	if (blockType == BlockTypes::trainingDummy)
	{
		addEmptyEntityBasedOnBlockPosition<EntityType::trainingDummy>(pos.x, pos.y, pos.z);
	}

}







