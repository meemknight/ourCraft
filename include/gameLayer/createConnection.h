#pragma once
#include <enet/enet.h>

struct ConnectionData
{
	ENetHost *client = 0;
	ENetPeer *server = 0;
};


ConnectionData getConnectionData();
bool createConnection();
void clientMessageLoop();