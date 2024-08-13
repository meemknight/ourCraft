#pragma once
#include <glm/glm.hpp>
#include <vector>

constexpr int CHUNK_SIZE = 16;
constexpr int META_CHUNK_SIZE = 32;
constexpr int CHUNK_HEIGHT = 256;

void pushFlagsLightAndPosition(std::vector<int> &vect, 
	glm::ivec3 position, bool isWater, bool isInWater, unsigned char sunLight, unsigned char torchLight, unsigned char aoShape);

int modBlockToChunk(int x);
glm::ivec2 modBlockToChunk(glm::ivec2);
int divideChunk(int x);
glm::ivec2 fromBlockPosToChunkPos(glm::ivec3 blockPos);
glm::ivec2 fromBlockPosToChunkPos(int x, int z);

//this is the index inside the chunk [0-15], y remains the same
glm::ivec3 fromBlockPosToBlockPosInChunk(glm::ivec3 blockPos);

int divideMetaChunk(int chunkPos);

glm::ivec2 determineChunkThatIsEntityIn(glm::dvec3 position);