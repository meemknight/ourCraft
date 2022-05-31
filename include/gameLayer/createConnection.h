#pragma once
#include <enet/enet.h>
#include <stdint.h>

struct ConnectionData
{
	ENetHost *client = 0;
	ENetPeer *server = 0;
	int32_t cid = 0;
};


ConnectionData getConnectionData();
bool createConnection();
void clientMessageLoop();