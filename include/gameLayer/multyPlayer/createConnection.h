#pragma once
#include <enet/enet.h>
#include <stdint.h>
#include <vector>
#include "packet.h"


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
	uint16_t blockType = 0;

};

void submitTaskClient(Task &t);
void submitTaskClient(std::vector<Task> &t);

struct Chunk;

struct ConnectionData
{
	ENetHost *client = 0;
	ENetPeer *server = 0;
	int32_t cid = 0;
	std::vector<Chunk *> recievedChunks = {};
	std::vector<Packet_PlaceBlock> recievedBlocks = {};
	bool conected = false;
};


std::vector<Chunk *> getRecievedChunks();
std::vector<Packet_PlaceBlock> getRecievedBlocks();
ConnectionData getConnectionData();
bool createConnection();
void clientMessageLoop();
void closeConnection();