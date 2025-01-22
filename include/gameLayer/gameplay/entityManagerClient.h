#pragma once
#include <gameplay/entity.h>
#include <gameplay/physics.h>
#include <unordered_map>
#include <deque>
#include <multyPlayer/server.h>
#include <gameplay/droppedItem.h>
#include <gameplay/player.h>
#include <gameplay/zombie.h>
#include <gameplay/pig.h>
#include <gameplay/allentities.h>


struct UndoQueue;
struct PlayerInventory;
struct PointDebugRenderer;
struct GyzmosRenderer;
struct Camera;

struct ClientEntityManager : public EntityDataClient
{

	ClientEntityManager()
	{
		for (int i = 0; i < EntitiesTypesCount; i++)
		{
			entityIds[i] = 1;
		}
	}

	LocalPlayer localPlayer;

	void dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance);

	std::uint64_t entityIds[EntitiesTypesCount] = {};
	std::uint64_t consumeId(unsigned short entityType);

	//leave count 0 to drop the full stack
	bool dropItemByClient(glm::dvec3 playerPos, 
		unsigned char inventorySlot, UndoQueue &undoQueue, glm::vec3 throwForce, std::uint64_t timer,
		PlayerInventory &inventory, int count);

	//todo this can't remove the player, maybe we should refactor
	void removeEntity(std::uint64_t entityId);

	void killEntity(std::uint64_t entityId);

	void removePlayer(std::uint64_t entityId);

	void removeDroppedItem(std::uint64_t entityId);
	
	//the server timer is only from updates from server
	void addOrUpdateDroppedItem(std::uint64_t eid, DroppedItem droppedItem, 
		UndoQueue &undoQueue, float restantTimer, std::uint64_t serverTimer, std::uint64_t timeUpdatedOnServer);

	template<int I, typename T>
	void addOrUpdateGenericEntity(std::uint64_t eid, T entity, UndoQueue &undoQueue, float restantTimer,
		std::uint64_t serverTimer, std::uint64_t timeUpdatedOnServer);

	template<int I, typename T>
	void genericCallAddOrUpdateEntity(std::uint64_t eid, T entity, float restantTimer);

	//void addOrUpdateZombie(std::uint64_t eid, Zombie entity, float restantTimer);
	//
	//void addOrUpdatePig(std::uint64_t eid, Pig entity, float restantTimer);
	//
	//void addOrUpdateCat(std::uint64_t eid, Cat entity, float restantTimer);

	void doAllUpdates(float deltaTime, ChunkData *(chunkGetter)(glm::ivec2), std::uint64_t serverTimer);

	void cleanup();

	std::uint64_t intersectAllAttackableEntities(glm::dvec3 start,
		glm::dvec3 dir, float maxDistance, float &outDistance, float delta);

	void renderColiders(PointDebugRenderer &pointDebugRenderer, GyzmosRenderer &gyzmosRenderer, Camera &c);
};




template<>
inline void ClientEntityManager::addOrUpdateGenericEntity<EntityType::droppedItems, DroppedItem>(
	std::uint64_t eid,
	DroppedItem entity,
	UndoQueue &undoQueue,
	float restantTimer, std::uint64_t serverTimer, std::uint64_t timeUpdatedOnServer)
{
	addOrUpdateDroppedItem(eid, entity, undoQueue, restantTimer, serverTimer, timeUpdatedOnServer);
	// Specialized implementation for DroppedItem
}


template<int I, typename T>
inline void ClientEntityManager::addOrUpdateGenericEntity(std::uint64_t eid, T entity, UndoQueue &undoQueue, 
	float restantTimer, std::uint64_t serverTimer, std::uint64_t timeUpdatedOnServer)
{

	auto &container = *entityGetter<I>();


	auto found = container.find(eid);

	if (found == container.end())
	{
		container[eid].entityBuffered = entity;
		container[eid].restantTime = restantTimer;
	}
	else
	{

		//found->second.rubberBand
		//	.addToRubberBand(found->second.entityBuffered.position - entity.position);


		//if (restantTimer > 0)
		//{
		//	found->second.oldPositionForRubberBand = found->second.entity.position;
		//}
		//else
		//{
		//	found->second.rubberBand
		//		.addToRubberBand(found->second.entity.position - entity.position);
		//}

		//found->second.entity = entity;
		found->second.bufferedEntityData.addElement(entity, serverTimer, timeUpdatedOnServer);
		found->second.restantTime = restantTimer; //todo this can be removed from here?
	}



}
