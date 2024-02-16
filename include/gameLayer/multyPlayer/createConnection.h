#pragma once
#include <enet/enet.h>
#include <stdint.h>
#include <vector>
#include "packet.h"

struct ClientEntityManager;

struct Task
{
	enum Type
	{
		none = 0,
		placeBlock,
		generateChunk,
	};

	glm::ivec3 pos = {};
	int type = 0;
	BlockType blockType = 0;
	EventId eventId = {};
	glm::ivec2 playerPosForChunkGeneration = {};

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
	int32_t cid = 0;
	std::vector<Chunk *> recievedChunks = {};
	std::vector<Packet_PlaceBlocks> recievedBlocks = {};
	bool conected = false;
};


std::vector<Chunk *> getRecievedChunks();
std::vector<Packet_PlaceBlocks> getRecievedBlocks();
ConnectionData getConnectionData();
bool createConnection(Packet_ReceiveCIDAndData &playerData);
void clientMessageLoop(EventCounter &validatedEvent, RevisionNumber &invalidateRevision
	,glm::ivec3 playerPosition, int squareDistance, ClientEntityManager& entityManager);

void closeConnection();