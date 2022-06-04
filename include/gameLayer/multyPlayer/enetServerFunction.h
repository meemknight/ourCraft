#pragma once
#include <enet/enet.h>
#include "threadStuff.h"
#include "packet.h"
#include "createConnection.h"

bool startEnetListener(ENetHost *_server);
void closeEnetListener();

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

void signalWaitingFromServer();

std::vector<ServerTask> waitForTasksServer();
std::vector<ServerTask> tryForTasksServer();
Client getClient(int32_t cid);
void broadCast(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel);