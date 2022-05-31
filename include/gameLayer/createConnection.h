#pragma once
#include <enet/enet.h>
#include <stdint.h>
#include <vector>

struct Chunk;

struct ConnectionData
{
	ENetHost *client = 0;
	ENetPeer *server = 0;
	int32_t cid = 0;
};


std::vector<Chunk *> getRecievedChunks();
ConnectionData getConnectionData();
bool createConnection();
void clientMessageLoop();
