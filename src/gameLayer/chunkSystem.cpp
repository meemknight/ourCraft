#include "chunkSystem.h"
#include <glm/vec2.hpp>
#include "threadStuff.h"
#include <algorithm>
#include <glm/glm.hpp>
#include "createConnection.h"
#include <iostream>

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

void ChunkSystem::update(int x, int z, std::vector<int>& data, float deltaTime)
{
	{
		std::vector < std::unordered_map<glm::ivec2, float>::iterator > toRemove;
		for (auto i = recentlyRequestedChunks.begin(); i != recentlyRequestedChunks.end(); i++)
		{
			i->second -= deltaTime;

			if (i->second <= 0)
			{
				toRemove.push_back(i);
			}
		}

		for (auto i : toRemove)
		{
			recentlyRequestedChunks.erase(i);
		}
	}

	{
		std::vector < std::unordered_map<glm::ivec3, GhostBlock>::iterator > toRemove;
		for (auto i = ghostBlocks.begin(); i != ghostBlocks.end(); i++)
		{
			i->second.timer -= deltaTime;

			if (i->second.timer <= 0)
			{
				toRemove.push_back(i);

				Chunk *chunk = 0;
				auto b = getBlockSafeAndChunk(i->first.x, i->first.y, i->first.z, chunk);

				if (b)
				{
					b->type = i->second.prevBlock;
					setChunkAndNeighboursFlagDirtyFromBlockPos(i->first.x, i->first.z);

				}
			}
		}

		for (auto i : toRemove)
		{
			ghostBlocks.erase(i);
		}
	}


	//x /= CHUNK_SIZE;
	//z /= CHUNK_SIZE;

	x = divideChunk(x);
	z = divideChunk(z);

	data.clear();

	glm::ivec2 minPos = glm::ivec2(x, z) - glm::ivec2(squareSize / 2, squareSize / 2);
	glm::ivec2 maxPos = glm::ivec2(x, z) + glm::ivec2(squareSize / 2 + squareSize % 2, squareSize / 2 + squareSize % 2);
	//exclusive max

	cornerPos = minPos;

#pragma region recieve chunks
	auto recievedChunk = getRecievedChunks();

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


#pragma region set chunk borders

	if (!created || lastX != x || lastZ != z)
	{

		std::vector<Chunk*> newChunkVector;
		newChunkVector.resize(squareSize * squareSize);

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

				newChunkVector[chunkPosRelToSystem.x * squareSize + chunkPosRelToSystem.y] = loadedChunks[i];

				loadedChunks[i] = nullptr;
			}
			else
			{
				delete loadedChunks[i];
				loadedChunks[i] = nullptr;
			}
		}

		loadedChunks = std::move(newChunkVector);
	}

	std::vector<Task> chunkTasks;
	for (int x = 0; x < squareSize; x++)
		for (int z = 0; z < squareSize; z++)
		{
			if (loadedChunks[x * squareSize + z] == nullptr)
			{
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


		constexpr int maxWaitingSubmisions = 5;
		std::vector<Task> finalTask;
		finalTask.reserve(maxWaitingSubmisions);

		for (int i = 0; i < maxWaitingSubmisions; i++)
		{
			if (chunkTasks.size() <= i) { break; }

			auto &t = chunkTasks[i];

			if (recentlyRequestedChunks.find({t.pos.x, t.pos.z}) == recentlyRequestedChunks.end())
			{
				recentlyRequestedChunks[{t.pos.x, t.pos.z}] = 3.f; //time not request again, it can be big since packets are reliable
				finalTask.push_back(t);
			}
		}

		submitTaskClient(finalTask);
	}



#pragma endregion


	created = 1;
	lastX = x;
	lastZ = z;



#pragma region place block
	auto recievedBLocks = getRecievedBlocks();
	for (auto &message : recievedBLocks)
	{
		auto pos = message.blockPos;
		int xPos = divideChunk(pos.x);
		int zPos = divideChunk(pos.z);
	
		if (xPos >= minPos.x && zPos >= minPos.y
			&& xPos < maxPos.x && zPos < maxPos.y
			)
		{
			//process block placement

			Chunk *c = 0;
			auto rez = getBlockSafeAndChunk(message.blockPos.x, message.blockPos.y, message.blockPos.z, c);

			auto it = ghostBlocks.find(message.blockPos);

			if (rez)
			{
				if (it != ghostBlocks.end())
				{
					//no need to check if c
					if (it->second.newBlock != message.blockType)
					{
						setChunkAndNeighboursFlagDirtyFromBlockPos(pos.x, pos.z);
					}
				}
				else
				{
					setChunkAndNeighboursFlagDirtyFromBlockPos(pos.x, pos.z);
				}

				rez->type = message.blockType;

			}

			if (it != ghostBlocks.end())
			{
				ghostBlocks.erase(it);
			}
	
		}
		else
		{
			//todo possible problems here
			//ignore message
		}
	}
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

Chunk *ChunkSystem::getChunkSafeFromBlockPos(int x, int z)
{
	auto c = getChunkSafe(divideChunk(x) - cornerPos.x, divideChunk(z) - cornerPos.y);
	return c;
}

void ChunkSystem::setChunkAndNeighboursFlagDirtyFromBlockPos(int x, int z)
{
	const int o = 5;
	glm::ivec2 offsets[o] = {{0,0}, {-1,0}, {1,0}, {0, -1}, {0, 1}};

	for (int i = 0; i < o; i++)
	{
		auto c = getChunkSafeFromBlockPos(x + offsets[i].x, z + offsets[i].y);
		if (c)
		{
			c->dirty = true;
		}
	}

}


Block* ChunkSystem::getBlockSafe(int x, int y, int z)
{
	Chunk *c = 0;
	return getBlockSafeAndChunk(x, y, z, c);
}



Block *ChunkSystem::getBlockSafeAndChunk(int x, int y, int z, Chunk *&chunk)
{
	chunk = nullptr;
	if (y < 0 || y >= CHUNK_HEIGHT) { return nullptr; }

	int cornerX = cornerPos.x;
	int cornerZ = cornerPos.y;

	auto c = getChunkSafe(divideChunk(x) - cornerX, divideChunk(z) - cornerZ);

	if (!c) { return nullptr; }

	chunk = c;

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
		glm::ivec3 intPos = pos;
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
				prevBlockForPlace = intPos;
			}
		}

		pos += delta;
	}

	prevBlockForPlace = std::nullopt;
	return nullptr;
}

//todo short for type
void ChunkSystem::placeBlock(glm::ivec3 pos, int type)
{
	//todo check legality locally
	Chunk *chunk = 0;
	auto b = getBlockSafeAndChunk(pos.x, pos.y, pos.z, chunk);
	
	if (b != nullptr)
	{
		Task task;
		task.type = Task::placeBlock;
		task.pos = pos;
		task.blockType = type;
		submitTaskClient(task);
	
		GhostBlock ghostBlock;
		ghostBlock.newBlock = type;
		ghostBlock.prevBlock = b->type;
		ghostBlock.timer = 2; //secconds

		ghostBlocks[pos] = ghostBlock;

		b->type = type;
		
		setChunkAndNeighboursFlagDirtyFromBlockPos(pos.x, pos.z);
	}
	
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
		return ((x+1) / CHUNK_SIZE) - 1;
	}
	else
	{
		return x / CHUNK_SIZE;
	}
};
