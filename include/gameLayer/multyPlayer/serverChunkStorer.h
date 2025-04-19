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
#include <optional>
#include <gameplay/lootTables.h>

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

//TODO REFACTOR TO ALSO USE FULL BLOCK INFO!!
struct GhostBlock
{
	Block block;
	unsigned char replaceAnything = 0;

	bool operator==(const GhostBlock &other)
	{
		return block == other.block && replaceAnything == other.replaceAnything;
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
		bool dirty = 0; //todo investigate if should remove?
		bool dirtyEntity = 0;
		bool shouldUnload = 0; 
		bool dirtyBlockData = 0;
		bool withinSimulationDistance = 0;
		//TODO, when checking what chunks should be kept, if we recommision an shouldUnloadChunk, make sure we
		//readd all the entities in the entities chunk position cache!


	}otherData;

	EntityData entityData;

	BlocksWithDataHolder blockData;

	//removes the data for a block with data
	void removeBlockWithData(glm::ivec3 pos, std::uint16_t blockType);

	bool normalize();
};

struct SendBlocksBack
{
	glm::ivec3 pos = {};
	Block blockInfo = {};
};


struct ServerChunkStorer
{

	std::vector<ColidableEntry> getCollisionsListThatCanPush(glm::dvec3 position, 
		glm::vec3 colider, std::uint64_t eidToIgnore);

	std::unordered_map<glm::ivec2, std::unordered_map<BlockInChunkPos, GhostBlock, BlockInChunkHash>,
		Ivec2Hash>
		ghostBlocks;

	std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> savedChunks = {};
	std::unordered_map<std::uint64_t, glm::ivec2> entityChunkPositions;

	void removeEntityChunkPositionsForChunk(SavedChunk &chunk);
		
	//uses chunk coorodonates
	SavedChunk *getChunkOrGetNull(int posX, int posZ);

	//OLD VERSION todo remove once refactoring is complete!
	SavedChunk *getOrCreateChunk(int posX, int posZ, WorldGenerator &wg,
		StructuresManager &structureManager, BiomesManager &biomesManager,
		std::vector<SendBlocksBack> &sendNewBlocksToPlayers, bool generateGhostAndStructures,
		std::vector<StructureToGenerate> *newStructuresToAdd, WorldSaver &worldSaver,
		bool *wasGenerated = 0, bool *wasLoaded = 0);

	SavedChunk *getOrCreateChunk(int posX, int posZ, WorldGenerator &wg,
		StructuresManager &structureManager, BiomesManager &biomesManager,
		std::vector<SendBlocksBack> &sendNewBlocksToPlayers, WorldSaver &worldSaver,
		bool *wasGenerated = 0, bool *wasLoaded = 0);


	bool generateStructure(StructureToGenerate s, StructureDataAndFlags &structure, int rotation,
		std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> &newCreatedOrLoadedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
		std::vector<glm::ivec3> *controlBlocks, bool replace = 0, BlockType from = 0, BlockType to = 0
	);

	bool generateStructure(StructureToGenerate s, StructuresManager &structureManager,
		std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> &newCreatedOrLoadedChunks,
		std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
		std::vector<glm::ivec3> *controlBlocks);

	Block *getBlockSafe(glm::ivec3 pos);

	ChestBlock *getChestBlock(glm::ivec3 pos, SavedChunk *&c);


	Block *getBlockSafeAndChunk(glm::ivec3 pos, SavedChunk *&c);

	Block *tryGetBlockIfChunkExistsNoChecks(glm::ivec3 pos);

	//returns true if placed
	bool placeGhostBlocksForChunk(int posX, int posZ, ChunkData &c);

	void cleanup();

	bool saveNextChunk(WorldSaver &worldSaver, int count = 1, int entitySaver = 1);

	void saveChunk(WorldSaver &worldSaver, SavedChunk *savedChunks);

	void saveChunkBlockData(WorldSaver &worldSaver, SavedChunk *savedChunks);

	void saveAllChunks(WorldSaver &worldSaver);

	int unloadChunksThatNeedUnloading(WorldSaver &worldSaver, int count = 2);

	bool entityAlreadyExists(std::uint64_t eid);

	std::uint64_t anyEntityIntersectsWithBlock(glm::ivec3 position);

	//if you call this for a player it will just return false!
	bool removeEntity(WorldSaver &worldSaver, std::uint64_t eid);

	//returns true if succeed
	bool hitEntityByPlayer(std::uint64_t eid, glm::dvec3 playerPosition,
		Item &weapon, std::uint64_t &wasKilled, glm::vec3 dir, std::minstd_rand &rng
		, float hitCorectness, float critChanceBonus, LootTable *&lottTable);

	std::optional<glm::dvec3> getEntityPosition(std::uint64_t entity);

};



std::array<glm::ivec2, 9> *getChunkNeighboursOffsets();