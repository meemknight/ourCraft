#pragma once
#include <glm/glm.hpp>

constexpr int CHUNK_SIZE = 16;
constexpr int META_CHUNK_SIZE = 32;
constexpr int CHUNK_HEIGHT = 256;

int modBlockToChunk(int x);
int divideChunk(int x);
glm::ivec2 fromBlockPosToChunkPos(glm::ivec3 blockPos);
glm::ivec2 fromBlockPosToChunkPos(int x, int z);

//this is the index inside the chunk [0-15], y remains the same
glm::ivec3 fromBlockPosToBlockPosInChunk(glm::ivec3 blockPos);

int divideMetaChunk(int chunkPos);

glm::ivec2 determineChunkThatIsEntityIn(glm::dvec3 position);