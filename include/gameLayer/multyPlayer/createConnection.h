#pragma once
#include <enet-1.3.18/include/enet/enet.h>
#include <stdint.h>
#include <vector>
#include "packet.h"
#include <gameplay/physics.h>
#include <gameplay/battleUI.h>

struct ClientEntityManager;
struct UndoQueue;
struct ChunkSystem;
struct LightSystem;
struct PlayerConnectionData;
struct ChestBlock;

struct Task
{
	enum Type
	{
		none = 0,
		placeBlock,
		placeBlockForce,
		breakBlock,
		droppedItemEntity,
		clientMovedItem,
		clientOverwriteItem,
		clientCraftedItem,
		clientSwapItems,
		clientUsedItem,
		clientInteractedWithBlock,
		clientExitedInteractionWithBlock,
		clientAttackedEntity,
		clientWantsToRespawn,
		clientRecievedDamageLocally,
		clientRecievedDamageLocallyAndDied,
		clientChangedBlockData,
	};
	
	
	glm::dvec3 doublePos = {};
	glm::ivec3 pos = {};
	glm::vec3 vector = {};
	int taskType = 0;
	unsigned short craftingRecepieIndex = 0;
	BlockType blockType = 0;
	Block block = {};
	EventId eventId = {};
	glm::ivec2 playerPosForChunkGeneration = {};
	unsigned short blockCount = 0;
	std::uint64_t entityId;
	MotionState motionState;
	std::uint64_t timer;
	unsigned short itemType = 0;
	unsigned char from;
	unsigned char to;
	unsigned char revisionNumber = 0;
	unsigned char inventroySlot = 0;
	short damage = 0;
	HitResult hitResult = {};
	std::vector<unsigned char> metaData;
};

void submitTaskClient(Task &t);
void submitTaskClient(std::vector<Task> &t);

Packet formatPacket(int header);
ENetPeer *getServer();


struct Chunk;

struct ConnectionData
{
	ENetHost *client = 0;
	ENetPeer *server = 0;
	std::uint64_t cid = 0;
	bool conected = false;
};


ConnectionData getConnectionData();
bool createConnection(Packet_ReceiveCIDAndData &playerData, const char *c);

//0 for all
bool placeItem(PlayerInventory &inventory, ChestBlock *chestBlock, int from, int to, int counter = 0);

//0 for all

bool swapItems(PlayerInventory &inventory, ChestBlock *chestBlock, int from, int to);
bool grabItem(PlayerInventory &inventory, ChestBlock *chestBlock, int from, int to, int counter = 0);
bool forceOverWriteItem(PlayerInventory &inventory, ChestBlock *chestBlock, int index, Item &item);
void clientMessageLoop(EventCounter &validatedEvent, RevisionNumber &invalidateRevision
	,glm::ivec3 playerPosition, int squareDistance, ClientEntityManager& entityManager,
	UndoQueue &undoQueue, ChunkSystem &chunkSystem,
	LightSystem &lightSystem,
	std::uint64_t &serverTimer, bool &disconnect,
	unsigned char revisionNumberBlockInteraction, bool &shouldExitBlockInteraction,
	bool &killedPlayer, bool &respawn,
	std::deque<std::string> &chat, float &chatTimer,
	InteractionData &playerInteraction,
	std::unordered_map<std::uint64_t, PlayerConnectionData> &playersConnectionData
	);

void attackEntity(std::uint64_t eid, unsigned char inventorySlot, glm::vec3 direction,
	HitResult hitResult);

void sendBlockInteractionMessage(
	std::uint64_t playerID,
	glm::ivec3 pos, BlockType block, unsigned char revisionNumber);

void closeConnection();

bool hostServer(const std::string &path);