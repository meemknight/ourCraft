#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <enet/enet.h>
#include "threadStuff.h"
#include "packet.h"
#include "createConnection.h"
#include "gamePlayLogic.h"
#include <unordered_set>
#include "serverChunkStorer.h"
#include <multyPlayer/client.h>

bool startEnetListener(ENetHost *_server, const std::string &path);
void closeEnetListener();

struct ServerTask
{
	Task t;
	std::uint64_t cid;
};



struct EntityIdHolder
{
	std::uint64_t entityIds[EntitiesTypesCount] = {};

	//todo an init method and stuff
	void create()
	{
		for (int i = 0; i < EntitiesTypesCount; i++)
		{
			entityIds[i] = RESERVED_CLIENTS_ID + 1;
		}
	}
};

Client getClient(std::uint64_t cid);
Client *getClientSafe(std::uint64_t cid);
Client *getClientNotLocked(std::uint64_t cid);
std::unordered_map<std::uint64_t, Client> getAllClients();

std::unordered_map<std::uint64_t, Client> &getAllClientsReff();

void sendPlayerInventoryAndIncrementRevision(Client &client, 
	int channel = channelChunksAndBlocks);

void sendPlayerInventoryNotIncrementRevision(Client &client,
	int channel = channelChunksAndBlocks);

void sendPlayerExitInteraction(Client &client, unsigned char revisionNumber);

//this updates the player's effects
void updatePlayerEffects(Client &client);


void broadCast(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel);
void broadCastNotLocked(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel);

bool checkIfPlayerShouldGetChunk(glm::ivec2 playerPos2D,
	glm::ivec2 chunkPos, int playerSquareDistance);


std::uint64_t getEntityIdAndIncrement(WorldSaver &worldSaver, int entityType);

std::uint64_t getCurrentEntityId(int entityType);


