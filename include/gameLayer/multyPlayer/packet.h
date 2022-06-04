#pragma once
#include <cstdint>
#include <enet/enet.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <chunkSystem.h>

struct Packet
{
	int32_t header = 0;
	int32_t cid = 0;
	char *getData()
	{
		return (char *)((&cid) + 1);
	}
};

enum
{
	headerNone = 0,
	headerReceiveCIDAndData,
	headerRequestChunk,
	headerPlaceBlock,
	headerRecieveChunk,
};

struct Packet_ReceiveCIDAndData
{
};

struct Packet_RequestChunk
{
	glm::ivec2 chunkPosition = {};
};

struct Packet_RecieveChunk
{
	ChunkData chunk = {};
};

struct Packet_PlaceBlock
{
	uint16_t blockType = {};
	glm::ivec3 blockPos = {};
};

//first channel connection and chunks
//second channel blocks
constexpr static int SERVER_CHANNELS = 2;

void sendPacket(ENetPeer *to, Packet p, const char *data, size_t size, bool reliable, int channel);
char *parsePacket(ENetEvent &event, Packet &p, size_t &dataSize);