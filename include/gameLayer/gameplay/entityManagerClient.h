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


struct ClientEntityManager : public EntityDataClient
{




	LocalPlayer localPlayer;

	void dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance);

	std::uint64_t idCounter = 1;

	std::uint64_t consumeId();

	//leave count 0 to drop the full stack
	bool dropItemByClient(glm::dvec3 position, 
		unsigned char inventorySlot, UndoQueue &undoQueue, glm::vec3 throwForce, std::uint64_t timer,
		PlayerInventory &inventory, int count);

	void removeEntity(std::uint64_t entityId);

	void removeDroppedItem(std::uint64_t entityId);
	
	void addOrUpdateDroppedItem(std::uint64_t eid, DroppedItem droppedItem, UndoQueue &undoQueue, float restantTimer);

	void addOrUpdateZombie(std::uint64_t eid, Zombie entity, float restantTimer);

	void addOrUpdatePig(std::uint64_t eid, Pig entity, float restantTimer);

	void doAllUpdates(float deltaTime, ChunkData *(chunkGetter)(glm::ivec2));
};


bool checkIfPlayerShouldGetEntity(glm::ivec2 playerPos2D,
	glm::dvec3 entityPos, int playerSquareDistance, int extraDistance);


