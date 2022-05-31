#include "chunkSystem.h"
#include <glm/vec2.hpp>
#include "threadStuff.h"
#include <algorithm>
#include <glm/glm.hpp>
#include "createConnection.h"

//todo rename !!!!!!!!!
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

	loadedChunks.resize(squareSize * squareSize, nullptr);

	//for (int x = 0; x < squareSize; x++)
	//	for (int z = 0; z < squareSize; z++)
	//	{
	//		Chunk *chunk = new Chunk;
	//		chunk->create(x, z);
	//		loadedChunks.push_back(nullptr);
	//	}

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

	cornerPos = minPos;

#pragma region set chunk borders

	if (!created || lastX != x || lastZ != z)
	{

		std::vector<Chunk*> newChunkVector;
		newChunkVector.resize(squareSize * squareSize);

		std::vector<Chunk*> dirtyChunls;
		dirtyChunls.reserve(squareSize * squareSize);

		for (int i = 0; i < squareSize * squareSize; i++)
		{
			if (loadedChunks[i] == nullptr) { continue; }

			glm::ivec2 chunkPos;
			chunkPos.x = loadedChunks[i]->data.x;
			chunkPos.y = loadedChunks[i]->data.z;

			if (
				chunkPos.x >= minPos.x &&
				chunkPos.y >= minPos.y &&
				chunkPos.x < maxPos.x &&
				chunkPos.y < maxPos.y
				)
			{
				glm::ivec2 chunkPosRelToSystem = chunkPos - minPos;
				//loadedChunks[i]->dirty = false;

				newChunkVector[chunkPosRelToSystem.x * squareSize + chunkPosRelToSystem.y] = loadedChunks[i];

				loadedChunks[i] = nullptr;
			}
			else
			{
				delete loadedChunks[i];
				//loadedChunks[i]->dirty = true;
				//dirtyChunls.push_back(loadedChunks[i]);
				loadedChunks[i] = nullptr;
			}
		}

		std::vector<Task> chunkTasks;
		for (int x = 0; x < squareSize; x++)
			for (int z = 0; z < squareSize; z++)
			{

				if (newChunkVector[x * squareSize + z] == nullptr)
				{
					//Chunk* chunk = dirtyChunls.back();
					//dirtyChunls.pop_back();
					//
					//chunk->create(x + minPos.x, z + minPos.y);
					//newChunkVector[x * squareSize + z] = chunk;
					Task t;

					t.type = Task::generateChunk;
					t.pos = glm::ivec3(x + minPos.x, 0, z + minPos.y);
					chunkTasks.push_back(t);
				}
			}
		
		if (!chunkTasks.empty())
		{
			std::sort(chunkTasks.begin(), chunkTasks.end(),
				[x, z](Task &a, Task &b)
			{

				int ax = a.pos.x - x;
				int az = a.pos.z - z;

				int bx = b.pos.x - x;
				int bz = b.pos.z - z;

				unsigned long reza = ax * ax + az * az;
				unsigned long rezb = bx * bx + bz * bz;

				return reza < rezb;
			}
			);

			submitTaskClient(chunkTasks);
		}


		//assert(dirtyChunls.empty());

		loadedChunks = std::move(newChunkVector);

	}

#pragma endregion


	created = 1;
	lastX = x;
	lastZ = z;

	auto recievedChunk = getRecievedChunks();

#pragma region recieve chunks

	for (auto &i : recievedChunk)
	{

		int x = i->data.x - minPos.x;
		int z = i->data.z - minPos.y;

		if (x < 0 || z < 0 || x >= squareSize || z >= squareSize)
		{
			delete i; // ignore chunk, not of use anymore
			continue;
		}
		else
		{
			if (loadedChunks[x * squareSize + z] != nullptr)
			{
				delete i; //double request, ignore
			}
			else
			{
				//assert(loadedChunks[x * squareSize + z] == nullptr);
				loadedChunks[x * squareSize + z] = i;
			}

		}
	}
	
#pragma endregion


#pragma region place block
	//todo re add
	//for (auto &message : recievedMessages)
	//{
	//	if (message.type == Message::placeBlock)
	//	{
	//		auto pos = message.pos;
	//		int xPos = divideChunk(pos.x);
	//		int zPos = divideChunk(pos.z);
	//
	//		if (xPos >= minPos.x && zPos >= minPos.y
	//			&& xPos < maxPos.x && zPos < maxPos.y
	//			)
	//		{
	//			//process block placement
	//
	//			auto rez = getBlockSafe(message.pos.x, message.pos.y, message.pos.z);
	//
	//			if (rez)
	//			{
	//				rez->type = message.blockType;
	//			}
	//
	//			auto c = getChunkSafe(xPos - minPos.x, zPos - minPos.y);
	//
	//			if (c)
	//			{
	//				c->dirty = true;
	//			}
	//
	//		}
	//		else
	//		{
	//			//ignore message
	//		}
	//	}
	//
	//}
#pragma endregion


#pragma region bake

	int currentBaked = 0;
	const int maxToBake = 3; //this frame

	auto chunkVectorCopy = loadedChunks;

	std::sort(chunkVectorCopy.begin(), chunkVectorCopy.end(),
		[x, z](Chunk* a, Chunk* b) 
			{
				if (a == nullptr) { return false; }
				if (b == nullptr) { return true; }
				
				int ax = a->data.x - x;
				int az = a->data.z - z;
	
				int bx = b->data.x - x;
				int bz = b->data.z - z;
	
				unsigned long reza = ax * ax + az * az;
				unsigned long rezb = bx * bx + bz * bz;
	
				return reza < rezb;
			}
		);

	for (int i = 0; i < chunkVectorCopy.size(); i++)
	{
		auto chunk = chunkVectorCopy[i];
		if (chunk == nullptr) { continue; } //todo break? 

		int x = chunk->data.x - minPos.x;
		int z = chunk->data.z - minPos.y;

		if (currentBaked < maxToBake)
		{
			auto left = getChunkSafe(x - 1, z);
			auto right = getChunkSafe(x + 1, z);
			auto front = getChunkSafe(x, z + 1);
			auto back = getChunkSafe(x, z - 1);
		
			auto b = chunk->bake(left, right, front, back);
		
			if (b) { currentBaked++; }
		
			for (auto i : chunk->opaqueGeometry)
			{
				data.push_back(i);
			}
		}
		else
		{
			if (!chunk->dirty)
			{
				for (auto i : chunk->opaqueGeometry)
				{
					data.push_back(i);
				}
			}
		}

	}

#pragma endregion

}

Block* ChunkSystem::getBlockSafe(int x, int y, int z)
{
	if (y < 0 || y >= CHUNK_HEIGHT) { return nullptr; }

	int cornerX = cornerPos.x;
	int cornerZ = cornerPos.y;
	
	auto c = getChunkSafe(divideChunk(x) - cornerX, divideChunk(z) - cornerZ);

	if (!c) { return nullptr; }

	auto b = c->safeGet(modBlockToChunk(x), y, modBlockToChunk(z));

	return b;
}

Block *ChunkSystem::rayCast(glm::dvec3 from, glm::vec3 dir, glm::ivec3 &outPos, float maxDist, std::optional<glm::ivec3> &prevBlockForPlace)
{
	float deltaMagitude = 0.01f;
	glm::vec3 delta = glm::normalize(dir) * deltaMagitude;

	glm::dvec3 pos = from;

	prevBlockForPlace = std::nullopt;

	for (float walkedDist = 0.f; walkedDist < maxDist; walkedDist += deltaMagitude)
	{
		glm::vec3 intPos = pos;
		outPos = intPos;
		auto b = getBlockSafe(intPos.x, intPos.y, intPos.z);
		
		if (b != nullptr)
		{
			if (!b->air())
			{
				outPos = intPos;
				return b;
			}
			else
			{
				prevBlockForPlace = glm::ivec3{intPos.x, intPos.y, intPos.z};
			}
		}

		pos += delta;
	}

	prevBlockForPlace = std::nullopt;
	return nullptr;
}

void ChunkSystem::placeBlock(glm::ivec3 pos, int type)
{
	Task task;
	task.type = Task::placeBlock;
	task.pos = pos;
	task.blockType = type;
	submitTaskClient(task);
}

int modBlockToChunk(int x)
{
	if (x < 0)
	{
		x = -x;
		x--;
		return CHUNK_SIZE - (x % CHUNK_SIZE) - 1;
	}
	else
	{
		return x % CHUNK_SIZE;
	}
}

int divideChunk(int x)
{
	if (x < 0)
	{
		return (x / CHUNK_SIZE) - 1;
	}
	else
	{
		return x / CHUNK_SIZE;
	}
};
