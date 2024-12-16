#pragma once
#include <chunk.h>
#include <unordered_set>
#include <unordered_map>
#include <glm/glm.hpp>
#include <worldGenerator.h>
#include <multyPlayer/chunkSaver.h>
#include <gameplay/droppedItem.h>
#include <gameplay/allentities.h>
#include <gameplay/physics.h>
#include <gameplay/blocks/blocksWithData.h>


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

struct ListNode
{
	ListNode *prev = nullptr;
	glm::ivec2 chunkPos = {};
	ListNode *next = nullptr;
};

struct GhostBlock
{
	BlockType type;
	bool replaceAnything = 0;

	bool operator==(const GhostBlock &other)
	{
		return type == other.type && replaceAnything == other.replaceAnything;
	}
};




//0.25 MB
struct SavedChunk
{

	ChunkData chunk;

	//todo remove this struct
	struct OtherData
	{
	
		//dirty means we should resave it to the disk
		bool dirty = 0;
		bool dirtyEntity = 0;
		bool shouldUnload = 0;


	}otherData;

	EntityData entityData;

	BlocksWithDataHolder blockData;

	//removes the data for a block with data
	void removeBlockWithData(glm::ivec3 pos, std::uint16_t blockType);
};

struct SendBlocksBack
{
	glm::ivec3 pos;
	BlockType block;
};


struct ServerChunkStorer
{

	std::vector<ColidableEntry> getCollisionsListThatCanPush(glm::dvec3 position, 
		glm::vec3 colider, std::uint64_t eidToIgnore);

	std::unordered_map<glm::ivec2, std::unordered_map<BlockInChunkPos, GhostBlock, BlockInChunkHash>,
		Ivec2Hash>
		ghostBlocks;

	std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> savedChunks = {};

	//uses chunk coorodonates
	SavedChunk *getChunkOrGetNull(int posX, int posZ);

	SavedChunk *getOrCreateChunk(int posX, int posZ, WorldGenerator &wg,
		StructuresManager &structureManager, BiomesManager &biomesManager,
		std::vector<SendBlocksBack> &sendNewBlocksToPlayers, bool generateGhostAndStructures,
		std::vector<StructureToGenerate> *newStructuresToAdd, WorldSaver &worldSaver,
		bool *wasGenerated = 0);


	bool generateStructure(StructureToGenerate s, StructureData *structure, int rotation,
		std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
		std::vector<glm::ivec3> *controlBlocks, bool replace = 0, BlockType from = 0, BlockType to = 0
	);

	bool generateStructure(StructureToGenerate s, StructuresManager &structureManager,
		std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, 
		std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
		std::vector<glm::ivec3> *controlBlocks);

	Block *getBlockSafe(glm::ivec3 pos);

	Block *tryGetBlockIfChunkExistsNoChecks(glm::ivec3 pos);


	void placeGhostBlocksForChunk(int posX, int posZ, ChunkData &c);

	void cleanup();

	bool saveNextChunk(WorldSaver &worldSaver, int count = 1, int entitySaver = 1);

	void saveChunk(WorldSaver &worldSaver, SavedChunk *savedChunks);

	void saveAllChunks(WorldSaver &worldSaver);

	int unloadChunksThatNeedUnloading(WorldSaver &worldSaver, int count = 1);

	bool entityAlreadyExists(std::uint64_t eid);

	std::uint64_t anyEntityIntersectsWithBlock(glm::ivec3 position);

	//if you call this for a player it will just return false!
	bool removeEntity(WorldSaver &worldSaver, std::uint64_t eid);

	//returns true if succeed
	bool hitEntityByPlayer(std::uint64_t eid, glm::dvec3 playerPosition,
		Item &weapon, std::uint64_t &wasKilled, glm::vec3 dir, std::minstd_rand &rng);

};



std::array<glm::ivec2, 9> *getChunkNeighboursOffsets();