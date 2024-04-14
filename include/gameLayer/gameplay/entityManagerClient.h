#pragma once
#include <gameplay/entity.h>
#include <gameplay/physics.h>
#include <unordered_map>
#include <deque>
#include <multyPlayer/server.h>
#include <gameplay/droppedItem.h>
#include <gameplay/player.h>

struct UndoQueue;


struct ReservedIDsRange
{
	std::uint64_t idStart = 0;
	size_t count = 0;
};

struct ClientEntityManager
{

	std::unordered_map<std::uint64_t, PlayerClient> players;
	
	std::unordered_map<std::uint64_t, DroppedItemClient> droppedItems;

	LocalPlayer localPlayer;

	void dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance);

	std::deque<ReservedIDsRange> reservedIds;

	std::uint64_t consumeId();

	bool dropItemByClient(glm::dvec3 position, 
		BlockType blockType, UndoQueue &undoQueue, glm::vec3 throwForce, std::uint64_t timer);

	void removeDroppedItem(std::uint64_t entityId);
	
	void addOrUpdateDroppedItem(std::uint64_t eid, DroppedItem droppedItem, UndoQueue &undoQueue, float restantTimer);

	void doAllUpdates(float deltaTime, ChunkData *(chunkGetter)(glm::ivec2));
};


bool checkIfPlayerShouldGetEntity(glm::ivec2 playerPos2D,
	glm::dvec3 entityPos, int playerSquareDistance, int extraDistance);


