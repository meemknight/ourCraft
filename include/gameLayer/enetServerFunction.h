#pragma once
#include <enet/enet.h>
#include "threadStuff.h"

bool startEnetListener(ENetHost *_server);

struct ServerTask
{
	Task t;
	int32_t cid;
};

struct Client
{
	ENetPeer *peer = {};
	//phisics::Entity entityData = {};
	//bool changed = 1;
	//char clientName[56] = {};
};

std::vector<ServerTask> waitForTasksServer();
std::vector<ServerTask> tryForTasksServer();
Client getClient(int32_t cid);