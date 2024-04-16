#pragma once
#include <cstdint>
#include <enet/enet.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <chunk.h>
#include "server.h"
#include "gamePlayLogic.h"
#include <gameplay/physics.h>
#include <gameplay/droppedItem.h>
#include <gameplay/player.h>
#include <gameplay/zombie.h>

using EventCounter = unsigned int;
using RevisionNumber = unsigned int;

struct EventId
{
	EventId() {};
	EventId(EventCounter counter, RevisionNumber revision):counter(counter), revision(revision) {};

	EventCounter counter = 0;
	RevisionNumber revision = 0;
};

struct Packet
{
	uint32_t header = 0;
	CID cid = 0;
	char *getData()
	{
		return (char *)((&cid) + 1);
	}

	bool isCompressed() { return header & 0x8000'0000; }

	void setCompressed(){header |= 0x8000'0000; }

	void setNotCompressed() { header &= 0x7FFF'FFF; }

};

enum
{
	headerNone = 0,
	headerReceiveCIDAndData,
	headerRequestChunk,
	headerPlaceBlock,
	headerPlaceBlocks,
	headerClientDroppedItem,
	headerRecieveChunk,
	headerValidateEvent,
	headerValidateEventAndChangeID,
	headerInValidateEvent,
	headerSendPlayerData,
	headerClientRecieveOtherPlayerPosition,
	headerClientRecieveDroppedItemUpdate,
	headerClientUpdateTimer,
	headerDisconnectOtherPlayer, //we use cid there to specify the connection to be removed
	headerConnectOtherPlayer,
	headerUpdateZombie,
};

enum 
{
	channelChunksAndBlocks,	  //this also handles items, maybe rename player actions or something
	channelPlayerPositions,
	channelEntityPositions,
	channelHandleConnections, //this will also send Entity cids and timers
	//channelRequestChunks, todo maybe try this in the future
	SERVER_CHANNELS

};

struct Packet_ClientUpdateTimer
{
	std::uint64_t timer = 0;
};

struct Packet_RecieveDroppedItemUpdate
{
	DroppedItem entity = {};
	std::uint64_t eid = 0;
	std::uint64_t timer = 0;
};

struct Packet_UpdateZombie
{
	Zombie entity = {};
	std::uint64_t eid = 0;
	std::uint64_t timer = 0;
};


struct Packet_ReceiveCIDAndData
{
	glm::dvec3 playersPosition = {};
	std::uint64_t yourPlayerEntityId = 0;

};

struct Packet_RequestChunk
{
	glm::ivec2 playersPositionAtRequest = {};
	glm::ivec2 chunkPosition = {};
};

//modifying this will trigger an ssert!
struct Packet_RecieveChunk
{
	ChunkData chunk = {};
};

struct Packet_ValidateEvent
{
	EventId eventId = {};
};

struct Packet_ValidateEventAndChangeId
{
	EventId eventId = {};
	std::uint64_t oldId = 0;
	std::uint64_t newId = 0;
};

struct Packet_InValidateEvent
{
	EventId eventId = {};
};

struct Packer_SendPlayerData
{
	PlayerData playerData = {};
	std::uint64_t timer = 0;

};

struct Packet_ClientRecieveOtherPlayerPosition
{
	glm::dvec3 position = {};
	std::uint64_t entityId = 0;
};

struct Packet_HeaderConnectOtherPlayer
{
	glm::dvec3 position = {};
	std::uint64_t entityId = 0;
	CID cid = 0;
};

//used by the client to talk to the server
struct Packet_PlaceBlock 
{
	glm::ivec3 blockPos = {};
	EventId eventId = {}; //event id is used by the player
	BlockType blockType = {};

};

struct Packet_ClientDroppedItem
{
	glm::dvec3 position = {};
	EventId eventId = {}; //event id is used by the player
	std::uint64_t entityID = 0;
	std::uint64_t timer = 0;
	BlockType blockType = {};
	unsigned char count = 0;
	MotionState motionState = {};
};

//the server doesn't have a backend for this !!, sent from the server to the player only
struct Packet_PlaceBlocks
{
	glm::ivec3 blockPos = {};
	BlockType blockType = {};
};



void *unCompressData(const char *data, size_t compressedSize, size_t &originalSize);

void sendPacketAndCompress(ENetPeer *to, Packet p, const char *data, size_t size, bool reliable, int channel);

void sendPacket(ENetPeer *to, Packet p, const char *data, size_t size, bool reliable, int channel);
char *parsePacket(ENetEvent &event, Packet &p, size_t &dataSize);

float computeRestantTimer(std::uint64_t older, std::uint64_t newer);