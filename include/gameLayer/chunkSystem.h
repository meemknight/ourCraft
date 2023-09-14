#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "blocks.h"
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <optional>
#include "multyPlayer/undoQueue.h"

struct LightSystem;

struct ChunkSystem
{

	std::vector<Chunk*> loadedChunks;
	std::vector<int> requestedChunks;
	int squareSize = 3;

	Chunk *getChunkSafe(int x, int z);
	Chunk *getChunkSafeFromBlockPos(int x, int z);
	void setChunkAndNeighboursFlagDirtyFromBlockPos(int x, int z);

	void createChunks(int viewDistance);

	void update(int x, int z, float deltaTime, UndoQueue &undoQueue, LightSystem &lightSystem);
	int lastX = 0, lastZ = 0, created = 0;
	glm::ivec2 cornerPos = {};

	Block *getBlockSafe(int x, int y, int z);
	Block *getBlockSafe(glm::dvec3 pos);
	Block* getBlockSafeAndChunk(int x, int y, int z, Chunk* &chunk);

	Block *rayCast(glm::dvec3 from, glm::vec3 dir, glm::ivec3 &outPos, float maxDist
		, std::optional<glm::ivec3> &prevBlockForPlace);

	//a client places a block and sends a task to the server for it to be placed
	void placeBlockByClient(glm::ivec3 pos, int type, UndoQueue &undoQueue, glm::dvec3 playerPos, LightSystem &lightSystem);

	//just place the block
	void placeBlockNoClient(glm::ivec3 pos, int type, LightSystem &lightSystem);

	//internal use
	void changeBlockLightStuff(glm::ivec3 pos, int currentLightLevel, uint16_t oldType,
		uint16_t newType, LightSystem &lightSystem);


	std::unordered_map<glm::ivec2, float> recentlyRequestedChunks;
};

int modBlockToChunk(int x);
int divideChunk(int x);
