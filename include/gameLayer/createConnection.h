#pragma once
#include <enet/enet.h>
#include <stdint.h>
#include <vector>
#include "packet.h"

struct Chunk;

struct ConnectionData
{
	ENetHost *client = 0;
	ENetPeer *server = 0;
	int32_t cid = 0;
};


std::vector<Chunk *> getRecievedChunks();
std::vector<Packet_PlaceBlock> getRecievedBlocks();
ConnectionData getConnectionData();
bool createConnection();
void clientMessageLoop();
