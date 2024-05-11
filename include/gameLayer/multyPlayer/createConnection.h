#pragma once
#include <enet/enet.h>
#include <stdint.h>
#include <vector>
#include "packet.h"
#include <gameplay/physics.h>

struct ClientEntityManager;
struct UndoQueue;

struct Task
{
	enum Type
	{
		none = 0,
		placeBlock,
		droppedItemEntity,
		clientMovedItem,
		clientOverwriteItem,
		clientSwapItems,
		generateChunk
	};

	glm::ivec3 pos = {};
	glm::dvec3 doublePos = {};
	int type = 0;
	BlockType blockType = 0;
	EventId eventId = {};
	glm::ivec2 playerPosForChunkGeneration = {};
	unsigned char blockCount = 0;
	std::uint64_t entityId;
	MotionState motionState;
	std::uint64_t timer;
	unsigned char from;
	unsigned char to;
	unsigned short itemType = 0;

};

void submitTaskClient(Task &t);
void submitTaskClient(std::vector<Task> &t);

Packet formatPacket(int header);
ENetPeer *getServer();


struct Chunk;

struct ConnectionData
{
	ENetHost *client = 0;
	ENetPeer *server = 0;
	std::uint64_t cid = 0;
	std::vector<Chunk *> recievedChunks = {};
	std::vector<Packet_PlaceBlocks> recievedBlocks = {};
	bool conected = false;
};



std::vector<Chunk *> getRecievedChunks();
std::vector<Packet_PlaceBlocks> getRecievedBlocks();
ConnectionData getConnectionData();
bool createConnection(Packet_ReceiveCIDAndData &playerData, const char *c);

//0 for all
bool placeItem(PlayerInventory &inventory, int from, int to, int counter = 0);

//0 for all
bool grabItem(PlayerInventory &inventory, int from, int to, int counter = 0);
bool forceOverWriteItem(PlayerInventory &inventory, int index, Item &item);
void clientMessageLoop(EventCounter &validatedEvent, RevisionNumber &invalidateRevision
	,glm::ivec3 playerPosition, int squareDistance, ClientEntityManager& entityManager,
	UndoQueue &undoQueue, std::uint64_t &serverTimer, bool &disconnect);

void closeConnection();