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
	std::uint64_t cid;
};

struct Client
{
	ENetPeer *peer = {};
	//phisics::Entity entityData = {};
	//bool changed = 1;
	//char clientName[56] = {};
	RevisionNumber revisionNumber = 1;

	PlayerServer playerData;
	glm::ivec2 positionForChunkGeneration = {};
};

Client getClient(std::uint64_t cid);
Client *getClientNotLocked(std::uint64_t cid);
std::unordered_map<std::uint64_t, Client> getAllClients();

void sendPlayerInventory(Client &client, int channel = channelChunksAndBlocks);


void broadCast(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel);
void broadCastNotLocked(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel);

bool checkIfPlayerShouldGetChunk(glm::ivec2 playerPos2D,
	glm::ivec2 chunkPos, int playerSquareDistance);


std::uint64_t getEntityIdAndIncrement(WorldSaver &worldSaver);

std::uint64_t getCurrentEntityId();

