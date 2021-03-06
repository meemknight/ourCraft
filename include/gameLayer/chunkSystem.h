#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "blocks.h"
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <optional>

struct GhostBlock
{
	float timer = 0;
	uint16_t prevBlock=0;
	uint16_t newBlock=0;
};

struct ChunkSystem
{

	std::vector<Chunk*> loadedChunks;
	std::vector<int> requestedChunks;
	int squareSize = 3;

	Chunk *getChunkSafe(int x, int z);
	Chunk *getChunkSafeFromBlockPos(int x, int z);
	void setChunkAndNeighboursFlagDirtyFromBlockPos(int x, int z);

	void createChunks(int viewDistance);

	void update(int x, int z, std::vector<int>& data, float deltaTime);
	int lastX = 0, lastZ = 0, created = 0;
	glm::ivec2 cornerPos = {};

	Block *getBlockSafe(int x, int y, int z);
	Block* getBlockSafeAndChunk(int x, int y, int z, Chunk* &chunk);

	Block *rayCast(glm::dvec3 from, glm::vec3 dir, glm::ivec3 &outPos, float maxDist
		, std::optional<glm::ivec3> &prevBlockForPlace);

	void placeBlock(glm::ivec3 pos, int type);

	std::unordered_map<glm::ivec2, float> recentlyRequestedChunks;
	std::unordered_map<glm::ivec3, GhostBlock> ghostBlocks;

};

int modBlockToChunk(int x);
int divideChunk(int x);
