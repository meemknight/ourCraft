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
#include <gameplay/pig.h>
#include <gameplay/cat.h>
#include <gl2d/gl2d.h>
#include <gameplay/life.h>
#include <gameplay/battleUI.h>

using EventCounter = unsigned int;
using RevisionNumber = unsigned int;

struct EventId
{
	EventId() {};
	EventId(EventCounter counter, RevisionNumber revision):counter(counter), revision(revision) {};

	EventCounter counter = 0; //todo just remove the counter or use the same revision for the items and everything
	RevisionNumber revision = 0;
};

struct Packet
{
	std::uint64_t cid = 0;
	uint32_t header = 0;
	char *getData()
	{
		return (char *)((&cid) + 1);
	}

	bool isCompressed() { return header & 0x8000'0000; }

	void setCompressed(){header |= 0x8000'0000; }

	void setNotCompressed() { header &= 0x7FFF'FFF; }

};

enum : std::uint32_t
{
	headerNone = 0,
	headerReceiveCIDAndData,
	headerPlaceBlock,
	headerPlaceBlockForce,
	headerBreakBlock,
	headerPlaceBlocks,
	headerClientDroppedItem,
	headerRecieveChunk,
	headerRecieveEntireBlockDataForChunk,
	headerValidateEvent,
	headerValidateEventAndChangeID,
	headerInValidateEvent,
	headerSendPlayerData,
	headerSendPlayerOtherData,
	headerClientRecieveOtherPlayerPosition,
	headerClientRecieveDroppedItemUpdate,
	headerClientRecieveAllInventory,
	headerClientUpdateTimer,
	headerClientMovedItem,
	headerClientCraftedItem,
	headerClientOverWriteItem,
	headerClientSwapItems,
	headerClientUsedItem,
	headerClientInteractWithBlock,
	headerUpdateOwnOtherPlayerSettings,
	headerRecieveExitBlockInteraction,
	headerDisconnectOtherPlayer, 
	headerUpdateZombie,
	headerUpdatePig,
	headerUpdateCat,
	headerUpdateGenericEntity,
	headerRemoveEntity,
	headerCompoundPacket,
	headerSendPlayerSkin,
	headerRecieveDamage,
	headerRecieveLife,
	headerUpdateLife,
	headerKillEntity,
	headerAttackEntity,
	headerClientWantsToRespawn,
	headerRespawnPlayer,
	headerClientDamageLocally,
	headerClientDamageLocallyAndDied,
	headerUpdateEffects,
	headerClientDroppedChunk,
	headerClientDroppedAllChunks,
	headerSendChat, //just the letters for now
	headerClientChangeBlockData,
	headerChangeBlockData,

};

enum 
{
	channelChunksAndBlocks,		//this also handles items, maybe rename player actions or something
	channelPlayerPositions,		
	channelEntityPositions,
	channelHandleConnections,	//this will also send Entity cids and timers
	channelEffects,				

	SERVER_CHANNELS

};

struct Packet_ClientChangeBlockData
{
	BlockDataHeader blockDataHeader = {};
	EventId eventId = {};
	//+ data
};

struct Packet_ChangeBlockData
{
	BlockDataHeader blockDataHeader = {};
	//+ data
};

struct Packet_DisconectOtherPlayer
{
	std::uint64_t EID = 0;
};

struct Packet_ClientUpdateTimer
{
	std::uint64_t timer = 0;
};

//also used for Packet_ClientDamageLocallyAndDied
struct Packet_ClientDamageLocally
{
	short damage = 0;
};

struct Packet_RespawnPlayer
{
	glm::dvec3 pos = {};
};

struct Packet_ClientUsedItem
{
	glm::ivec3 position = {};
	unsigned short itemType = 0;
	unsigned char from = 0;
	unsigned char revisionNumber = 0;
};

//todo add revision number to client drop item
struct Packet_RecieveDroppedItemUpdate
{
	DroppedItem entity = {};
	std::uint64_t eid = 0;
	std::uint64_t timer = 0;
};

struct Packet_RecieveExitBlockInteraction
{
	unsigned char revisionNumber;
};

struct Packet_ClientMovedItem
{
	unsigned short itemType;
	unsigned char from;
	unsigned char to;
	unsigned char counter;
	unsigned char revisionNumber;
};

struct Packet_ClientCraftedItem
{
	unsigned short recepieIndex;
	unsigned char to;
	unsigned char revisionNumber;
};

struct Packet_ClientOverWriteItem
{
	unsigned short itemType;
	unsigned short metadataSize = 0;
	unsigned char to;
	unsigned char counter;
	unsigned char revisionNumber;
};

struct Packet_ClientSwapItems
{
	unsigned char from;
	unsigned char to;
	unsigned char revisionNumber;
};

struct Packet_ClientInteractWithBlock
{
	glm::ivec3 blockPos;
	unsigned short blockType;
	unsigned char interactionCounter;

};

struct Packet_UpdateZombie
{
	Zombie entity = {};
	std::uint64_t eid = 0;
	std::uint64_t timer = 0;
};

struct Packet_RemoveEntity
{
	std::uint64_t EID = 0;
};

struct Packet_KillEntity
{
	std::uint64_t EID = 0;
};


//used with:
//headerRecieveDamage,
//headerRecieveLife,
//headerUpdateLife,
struct Packet_UpdateLife
{
	Life life;
};

struct Packet_UpdateGenericEntity
{
	std::uint64_t eid = 0;
	std::uint64_t timer = 0;

};

struct Packet_UpdatePig
{
	Pig entity = {};
	std::uint64_t eid = 0;
	std::uint64_t timer = 0;
};

struct Packet_UpdateCat
{
	Cat entity = {};
	std::uint64_t eid = 0;
	std::uint64_t timer = 0;
};

struct Packet_ReceiveCIDAndData
{
	Player entity = {};
	OtherPlayerSettings otherSettings = {};
	std::uint64_t timer = 0;
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
	Player playerData = {};
	std::uint64_t timer = 0;

};

struct Packet_ClientRecieveOtherPlayerPosition
{
	Player entity = {};
	std::uint64_t eid = 0;
	std::uint64_t timer = 0;
};

struct Packet_UpdateOwnOtherPlayerSettings
{
	OtherPlayerSettings otherPlayerSettings = {};
};

//used by the client to talk to the server
struct Packet_ClientPlaceBlock 
{
	glm::ivec3 blockPos = {};
	EventId eventId = {}; //event id is used by the player
	BlockType blockType = {};
	unsigned char inventoryRevision;
	unsigned char inventorySlot = 0;
};

//used by creative players to paste structures
struct Packet_ClientPlaceBlockForce
{
	glm::ivec3 blockPos = {};
	EventId eventId = {}; //event id is used by the player
	BlockType blockType = {};
};

struct Packet_ClientBreakBlock
{
	glm::ivec3 blockPos = {};
	EventId eventId = {}; //event id is used by the player
};

struct Packet_ClientDroppedItem
{
	glm::dvec3 position = {};
	EventId eventId = {}; //event id is used by the player
	std::uint64_t entityID = 0;
	std::uint64_t timer = 0;
	unsigned char inventorySlot = 0;
	unsigned char count = 0;
	unsigned char revisionNumberInventory = 0;
	unsigned short type = 0;
	MotionState motionState = {};
};

//the server doesn't have a backend for this !!, sent from the server to the player only
struct Packet_PlaceBlocks
{
	glm::ivec3 blockPos = {};
	BlockType blockType = {};
};

struct Packet_AttackEntity
{
	HitResult hitResult;
	std::uint64_t entityID = 0;
	glm::vec3 direction = {};
	unsigned char inventorySlot = 0;
};

struct Packet_UpdateEffects
{
	std::uint64_t timer = 0; //tick timer
	Effects effects = {};
};

struct Packet_ClientDroppedChunk
{
	glm::ivec2 chunkPos = {};
};

void *unCompressData(const char *data, size_t compressedSize, size_t &originalSize);

void sendPacketAndCompress(ENetPeer *to, Packet p,
	const char *data, size_t size, bool reliable, int channel,
	ENetPacketFreeCallback freeCallback = nullptr, unsigned int packetId = 0);

void sendPacket(ENetPeer *to, Packet p, const char *data, size_t size, 
	bool reliable, int channel, ENetPacketFreeCallback freeCallback = 0, unsigned int packetId = 0);

//ton't use in client code!!
void sendPacket(ENetPeer *to, uint32_t header, void *data, size_t size, bool reliable, int channel);

void sendPacket(ENetPeer *to, uint32_t header, std::uint64_t cid, void *data, size_t size, bool reliable, int channel);


char *parsePacket(ENetEvent &event, Packet &p, size_t &dataSize);

char *parsePacket(ENetPacket &packet, Packet &p, size_t &dataSize);


float computeRestantTimer(std::uint64_t older, std::uint64_t newer);

void sendPlayerSkinPacket(ENetPeer *to, std::uint64_t cid, gl2d::Texture &t);

void sendPlayerSkinPacket(ENetPeer *to, std::uint64_t cid, std::vector<unsigned char> &data);