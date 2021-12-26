#pragma once
#include "blocks.h"
#include <unordered_map>



struct ChunkSystem
{

	std::vector<Chunk*> loadedChunks;
	int squareSize = 3;

	Chunk* getChunkSafe(int x, int z);

	void createChunks(int viewDistance, std::vector<int>& data);

	void update(int x, int z, std::vector<int>& data);

	Block* getBlockSafe(int x, int y, int z);

};