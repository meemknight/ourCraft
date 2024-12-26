#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <worldGenerator.h>
#include <multyPlayer/chunkSaver.h>
#include <multyPlayer/serverChunkStorer.h>
#include <random>


void entityDeleteFromServerToPlayer(std::uint64_t clientToSend, std::uint64_t eid, 
	bool reliable);

void entityDeleteFromServerToPlayer(Client &client, std::uint64_t eid,
	bool reliable);

void doGameTick(float deltaTime,
	int deltaTimeMs,
	std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache,
	EntityData &orphanEntities,
	unsigned int seed,
	std::vector<ServerTask> waitingTasks, WorldSaver &worldSaver
);


void sendDamagePlayerPacket(Client &client);
void sendIncreaseLifePlayerPacket(Client &client);
void sendUpdateLifeLifePlayerPacket(Client &client);



