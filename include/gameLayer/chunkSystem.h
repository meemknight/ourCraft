#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "blocks.h"
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <optional>
#include "multyPlayer/undoQueue.h"
#include "chunk.h"
#include <rendering/bigGpuBuffer.h>

struct LightSystem;


struct ChunkSystem
{

	BigGpuBuffer gpuBuffer;

	struct ChunkSystemSettings
	{

		//max number of chunks the client can be waiting for at a time.
		//note that this number is not a hard limit, if the player moves or teleports it will always be able to request
		//more chunks, this is a hard limit only when the player s
		int maxWaitingSubmisions = 15;

	}chunkSystemSettings;

	bool shouldUpdateLights = 0;

	std::vector<Chunk*> loadedChunks;
	int squareSize = 3;

	//[0 -> squareSize)
	Chunk *getChunksInMatrixSpaceUnsafe(int x, int z);

	Chunk *getChunkSafeFromMatrixSpace(int x, int z);
	Chunk *getChunkSafeFromBlockPos(int x, int z);

	//this is the chunk pos, so not in matrix space
	Chunk *getChunkSafeFromChunkPos(int x, int z);

	glm::ivec2 fromBlockPosToMatrixSpace(int x, int z);
	glm::ivec2 fromMatrixSpaceToChunkSpace(int x, int z);

	void setChunkAndNeighboursFlagDirtyFromBlockPos(int x, int z);

	void init(int squareDistance);

	void changeRenderDistance(int squareDistance);

	void cleanup();

	void update(glm::ivec3 playerBlockPosition, float deltaTime, UndoQueue &undoQueue, LightSystem &lightSystem);
	int lastX = 0, lastZ = 0, created = 0;

	glm::ivec3 lastPlayerBlockPosition = {};

	glm::ivec2 cornerPos = {};

	Block *getBlockSafe(int x, int y, int z);
	Block *getBlockSafe(glm::dvec3 pos);
	Block* getBlockSafeAndChunk(int x, int y, int z, Chunk* &chunk);

	void getBlockSafeWithNeigbhours(int x, int y, int z,
		Block *&center, Block *&front, Block *&back, Block *&top, Block *&bottom, 
		Block *&left, Block *&right);

	void getBlockSafeWithNeigbhoursStopIfCenterFails(int x, int y, int z,
		Block *&center, Block *&front, Block *&back, Block *&top, Block *&bottom,
		Block *&left, Block *&right);

	Block *rayCast(glm::dvec3 from, glm::vec3 dir, glm::ivec3 &outPos, float maxDist
		, std::optional<glm::ivec3> &prevBlockForPlace, float &outDist);

	//a client places a block and sends a task to the server for it to be placed
	//returns true if succeeded
	bool placeBlockByClient(glm::ivec3 pos, unsigned char inventorySlot,
		UndoQueue &undoQueue, glm::dvec3 playerPos, 
		LightSystem &lightSystem, PlayerInventory &inventory, bool decreaseCounter
	);

	//a client breaks a block and sends a task to the server for it to be blocked
	//returns true if succeeded
	bool breakBlockByClient(glm::ivec3 pos,
		UndoQueue &undoQueue, glm::dvec3 playerPos,
		LightSystem &lightSystem
	);

	//just place the block, forcely by server
	void placeBlockNoClient(glm::ivec3 pos, BlockType type, LightSystem &lightSystem);

	//internal use
	void changeBlockLightStuff(glm::ivec3 pos, int currentSkyLightLevel, int currentNormalLightLevel,
		BlockType oldType,
		BlockType newType, LightSystem &lightSystem);

	//unloads all loaded chunks
	void dropAllChunks(BigGpuBuffer *gpuBuffer);
	
	//unloads a chunk atn the specified index in the matrix vector,
	//ASSUMES THE CHUNK IS LOADED AND HAS GPU DATA
	void dropChunkAtIndexUnsafe(int index, BigGpuBuffer *gpuBuffer);

	void dropChunkAtIndexSafe(int index, BigGpuBuffer *gpuBuffer);

	std::unordered_map<glm::ivec2, float> recentlyRequestedChunks;
};


