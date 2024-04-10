#pragma once
#include <chrono>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Client;
struct EventId;

using CID = int32_t;

bool isServerRunning();

bool startServer();
int getChunkCapacity();
void closeServer();
bool computeRevisionStuff(Client &client, bool allowed, const EventId &eventId);
void serverWorkerFunction();


std::uint64_t getTimer();


struct PerClientServerSettings
{

	bool validateStuff = true;

	glm::dvec3 outPlayerPos; //just for printing (todo remove)
};

struct ServerSettings
{
	std::unordered_map<CID, PerClientServerSettings> perClientSettings;

	int targetTicksPerSeccond = 20;
	bool busyWait = 1;
	int unloadChunksEverySecconds = 10;

};

ServerSettings getServerSettingsCopy();

PerClientServerSettings getClientSettingCopy(CID client);

void setServerSettings(ServerSettings settings);

void addCidToServerSettings(CID cid);

void removeCidFromServerSettings(CID cid);


//https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-user-defined-class-in-cpp/
struct Ivec2Hash
{
	size_t operator()(const glm::ivec2 &in) const
	{
		int x = in.x;
		int z = in.y;

		size_t ret = 0;
		ret += x;
		ret += (z < 32);

		return ret;
	}
};

struct BlockInChunkPos
{
	BlockInChunkPos() {};
	BlockInChunkPos(int x, int y, int z):x(x), y(y), z(z) {};

	unsigned char x = 0;
	unsigned char y = 0;
	unsigned char z = 0;

	bool operator==(const BlockInChunkPos &other)
	{
		return x == other.x && y == other.y && z == other.z;
	}

};

bool operator==(const BlockInChunkPos &a, const BlockInChunkPos &b);

struct BlockInChunkHash
{
	size_t operator()(const BlockInChunkPos &in) const
	{
		int x = in.x;
		int y = in.y;
		int z = in.z;

		size_t ret = 0;
		ret += x;
		ret += (y < 8);
		ret += (z < 16);

		return ret;
	}
};