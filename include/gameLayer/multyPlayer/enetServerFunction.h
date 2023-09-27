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
	CID cid;
};

struct Client
{
	ENetPeer *peer = {};
	//phisics::Entity entityData = {};
	//bool changed = 1;
	//char clientName[56] = {};
	RevisionNumber revisionNumber = 1;
};

void signalWaitingFromServer();

std::vector<ServerTask> waitForTasksServer();
std::vector<ServerTask> tryForTasksServer();
Client getClient(CID cid);
void broadCast(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel);