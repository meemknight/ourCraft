#pragma once
#include <chrono>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <biome.h>
#include <multyPlayer/chunkSaver.h>
#include <worldGenerator.h>

constexpr std::uint64_t RESERVED_CLIENTS_ID = 100;

struct Client;
struct EventId;

using CID = int32_t;

bool isServerRunning();
bool startServer();
void clearSD(WorldSaver &worldSaver);
int getChunkCapacity();
void closeServer();
bool computeRevisionStuff(Client &client, bool allowed,
	const EventId &eventId, std::uint64_t *oldid = 0, std::uint64_t *newid = 0);

void serverWorkerUpdate(WorldGenerator &wg, StructuresManager &structuresManager, 
	BiomesManager &biomesManager, WorldSaver &worldSaver, 
	float deltaTime);


std::uint64_t getTimer();


struct PerClientServerSettings
{

	bool validateStuff = true;
	bool spawnZombie = false;

	glm::dvec3 outPlayerPos; //just for printing (todo remove)
};

struct ServerSettings
{
	std::unordered_map<CID, PerClientServerSettings> perClientSettings;

	int targetTicksPerSeccond = 20;
	bool busyWait = 1;

};

ServerSettings getServerSettingsCopy();

PerClientServerSettings getClientSettingCopy(CID client);

void setServerSettings(ServerSettings settings);

void addCidToServerSettings(CID cid);

void removeCidFromServerSettings(CID cid);



