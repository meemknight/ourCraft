#pragma once
#include <chrono>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <biome.h>
#include <multyPlayer/chunkSaver.h>
#include <worldGenerator.h>

constexpr std::uint64_t RESERVED_CLIENTS_ID = 100000;

struct Client;
struct EventId;


bool isServerRunning();
bool startServer();
int getServerTicksPerSeccond();
void clearSD(WorldSaver &worldSaver);
int getChunkCapacity();
void closeServer();
bool computeRevisionStuff(Client &client, bool allowed,
	const EventId &eventId, std::uint64_t *oldid = 0, std::uint64_t *newid = 0);


struct PerClientServerSettings
{

	bool validateStuff = true;
	bool spawnZombie = false;
	bool spawnPig = false;

	glm::dvec3 outPlayerPos;
};



constexpr static int targetTicksPerSeccond = 20;

struct ServerSettings
{
	std::unordered_map<std::uint64_t, PerClientServerSettings> perClientSettings;

	bool busyWait = 1;

};

struct ServerTask;

void serverWorkerUpdate(WorldGenerator &wg, StructuresManager &structuresManager, 
	BiomesManager &biomesManager, WorldSaver &worldSaver, 
	std::vector<ServerTask> &serverTask,
	float deltaTime);


//returns the timer since the start of the program
std::uint64_t getTimer();



void addCidToServerSettings(std::uint64_t cid);

void removeCidFromServerSettings(std::uint64_t cid);

ServerSettings getServerSettingsCopy();

void setServerSettings(ServerSettings settings);
