#pragma once
#include <chrono>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <biome.h>
#include <multyPlayer/chunkSaver.h>
#include <worldGenerator.h>
#include <enet/enet.h>
#include <tickTimer.h>

constexpr std::uint64_t RESERVED_CLIENTS_ID = 100'000;

struct Client;
struct EventId;
struct ServerChunkStorer;

bool isServerRunning();
bool startServer(const std::string &path);
ServerChunkStorer &getServerChunkStorer();
int getServerTicksPerSeccond();
void clearSD(WorldSaver &worldSaver);
int getChunkCapacity();
void closeServer();
bool computeRevisionStuff(Client &client, bool allowed,
	const EventId &eventId, std::uint64_t *oldid = 0, std::uint64_t *newid = 0);


struct PerClientServerSettings
{

	bool validateStuff = 1;
	bool resendInventory = false;
	bool damage = false;
	bool heal = false;
	bool killApig = false;

	glm::dvec3 outPlayerPos;
};




struct ServerSettings
{
	std::unordered_map<std::uint64_t, PerClientServerSettings> perClientSettings;

	bool busyWait = 1;
	unsigned int randomTickSpeed = 3;


};

struct ServerTask;
struct Profiler;

void serverWorkerUpdate(WorldGenerator &wg, StructuresManager &structuresManager, 
	BiomesManager &biomesManager, WorldSaver &worldSaver, 
	std::vector<ServerTask> &serverTask,
	float deltaTime, Profiler &profiler);


//returns the timer since the start of the program
std::uint64_t getTimer();



void addCidToServerSettings(std::uint64_t cid);

void removeCidFromServerSettings(std::uint64_t cid);

void changePlayerGameMode(std::uint64_t cid, unsigned char gameMode);

ServerSettings getServerSettingsCopy();
ServerSettings &getServerSettingsReff();
Profiler getServerProfilerCopy();
Profiler getServerTickProfilerCopy();


unsigned int getRandomTickSpeed();

void setServerSettings(ServerSettings settings);

void genericBroadcastEntityDeleteFromServerToPlayer(std::uint64_t eid, bool reliable,
	std::unordered_map < std::uint64_t, Client *> &allClients,
	glm::ivec2 lastChunkClientsGotUpdates);


void genericBroadcastEntityKillFromServerToPlayer(std::uint64_t eid, bool reliable, ENetPeer* peerToIgnore = 0);