#pragma once
#include <enet/enet.h>
#include "threadStuff.h"
#include "packet.h"
#include "createConnection.h"
#include "gamePlayLogic.h"

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

	std::uint64_t entityId = 0;

	PlayerData playerData; //todo remove maybe?
	glm::ivec2 positionForChunkGeneration = {};
};

void signalWaitingFromServer();

std::vector<ServerTask> waitForTasksServer();
std::vector<ServerTask> tryForTasksServer();
Client getClient(CID cid);
std::unordered_map<CID, Client> getAllClients();

void broadCast(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel);

bool checkIfPlayerShouldGetChunk(glm::ivec2 playerPos2D,
	glm::ivec2 chunkPos, int playerSquareDistance);
