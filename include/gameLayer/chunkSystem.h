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

struct LightSystem;

struct ChunkSystem
{

	bool shouldUpdateLights = 0;

	std::vector<Chunk*> loadedChunks;
	std::vector<int> requestedChunks;
	int squareSize = 3;

	Chunk *getChunkSafeFromChunkSystemCoordonates(int x, int z);
	Chunk *getChunkSafeFromBlockPos(int x, int z);
	void setChunkAndNeighboursFlagDirtyFromBlockPos(int x, int z);

	void createChunks(int viewDistance);

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
		, std::optional<glm::ivec3> &prevBlockForPlace);

	//a client places a block and sends a task to the server for it to be placed
	void placeBlockByClient(glm::ivec3 pos, BlockType type, UndoQueue &undoQueue, glm::dvec3 playerPos, LightSystem &lightSystem);

	//just place the block
	void placeBlockNoClient(glm::ivec3 pos, BlockType type, LightSystem &lightSystem);

	//internal use
	void changeBlockLightStuff(glm::ivec3 pos, int currentSkyLightLevel, int currentNormalLightLevel,
		BlockType oldType,
		BlockType newType, LightSystem &lightSystem);


	std::unordered_map<glm::ivec2, float> recentlyRequestedChunks;
};

int modBlockToChunk(int x);
int divideChunk(int x);

int divideMetaChunk(int chunkPos);
