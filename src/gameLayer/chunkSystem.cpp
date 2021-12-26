#include "chunkSystem.h"
#include <glm/vec2.hpp>

Chunk* ChunkSystem::getChunkSafe(int x, int z)
{
	if (x < 0 || z < 0 || x >= squareSize || z >= squareSize)
	{
		return nullptr;
	}
	else
	{
		return loadedChunks[x * squareSize + z];
	}
}

void ChunkSystem::createChunks(int viewDistance, std::vector<int> &data)
{
	squareSize = viewDistance;

	for (int x = 0; x < squareSize; x++)
		for (int z = 0; z < squareSize; z++)
		{
			Chunk *chunk = new Chunk;
			chunk->create(x, z);
			loadedChunks.push_back(chunk);
		}

	//for (int x = 0; x < squareSize; x++)
	//	for (int z = 0; z < squareSize; z++)
	//	{
	//		auto chunk = getChunkSafe(x, z);
	//		chunk->bake();
	//	}

}

void ChunkSystem::update(int x, int z, std::vector<int>& data)
{
	x /= CHUNK_SIZE;
	z /= CHUNK_SIZE;

	data.clear();

	glm::ivec2 minPos = glm::ivec2(x, z) - glm::ivec2(squareSize / 2, squareSize / 2);
	glm::ivec2 maxPos = glm::ivec2(x, z) + glm::ivec2(squareSize / 2 + squareSize % 2, squareSize / 2 + squareSize % 2);
	//exclusive max


	std::vector<Chunk*> newChunkVector;
	newChunkVector.resize(squareSize * squareSize);

	std::vector<Chunk*> dirtyChunls;
	dirtyChunls.reserve(squareSize * squareSize);

	for (int i = 0; i < squareSize * squareSize; i++)
	{
		glm::ivec2 chunkPos;

		chunkPos.x = loadedChunks[i]->x;
		chunkPos.y = loadedChunks[i]->z;

		if (
			chunkPos.x >= minPos.x &&
			chunkPos.y >= minPos.y &&
			chunkPos.x < maxPos.x &&
			chunkPos.y < maxPos.y
			)
		{
			glm::ivec2 chunkPosRelToSystem = chunkPos - minPos;
			loadedChunks[i]->dirty = false;


			newChunkVector[chunkPosRelToSystem.x * squareSize + chunkPosRelToSystem.y] = loadedChunks[i];
			
			loadedChunks[i] = nullptr;
		}
		else
		{
			loadedChunks[i]->dirty = true;
			dirtyChunls.push_back(loadedChunks[i]);
			loadedChunks[i] = nullptr;
		}
	}

	for (int x = 0; x < squareSize; x++)
		for (int z = 0; z < squareSize; z++)
		{
			if (newChunkVector[x * squareSize + z] == nullptr)
			{
				Chunk* chunk = dirtyChunls.back();
				dirtyChunls.pop_back();

				chunk->create(x + minPos.x, z + minPos.y);
				newChunkVector[x * squareSize + z] = chunk;
			}
		}

	assert(dirtyChunls.empty());

	loadedChunks = std::move(newChunkVector);

	for (int x = 0; x < squareSize; x++)
		for (int z = 0; z < squareSize; z++)
		{
			auto chunk = getChunkSafe(x, z);
			auto left = getChunkSafe(x-1, z);
			auto right = getChunkSafe(x+1, z);
			auto front = getChunkSafe(x, z+1);
			auto back = getChunkSafe(x, z-1);

			chunk->bake(left, right, front, back);

			for (auto i : chunk->opaqueGeometry)
			{
				data.push_back(i);
			}

		}


}


