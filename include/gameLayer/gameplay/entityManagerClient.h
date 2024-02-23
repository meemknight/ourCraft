#pragma once
#include <gameplay/physics.h>
#include <unordered_map>
#include <deque>

struct UndoQueue;

struct PlayerNetworked
{
	glm::dvec3 position = {};

	//todo rotation and others

	
};

struct DroppedItem
{
	glm::dvec3 position = {};
	glm::dvec3 lastPosition = {};
	BlockType type = 0;
	unsigned char count = 0;
};

struct ReservedIDsRange
{
	std::uint64_t idStart = 0;
	size_t count = 0;
};

struct ClientEntityManager
{

	std::unordered_map<std::uint64_t, PlayerNetworked> players;
	
	std::unordered_map<std::uint64_t, DroppedItem> droppedItems;

	
	Player localPlayer;

	void dropEntitiesThatAreTooFar(glm::ivec2 playerPos2D, int playerSquareDistance);

	std::deque<ReservedIDsRange> reservedIds;

	std::uint64_t consumeId();

	bool dropItemByClient(glm::dvec3 position, BlockType blockType, UndoQueue &undoQueue);

	void removeDroppedItem(std::uint64_t entityId);

};


bool checkIfPlayerShouldGetEntity(glm::ivec2 playerPos2D,
	glm::dvec3 entityPos, int playerSquareDistance, int extraDistance);


