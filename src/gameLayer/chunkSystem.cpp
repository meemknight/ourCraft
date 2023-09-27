#include "chunkSystem.h"
#include <glm/vec2.hpp>
#include "threadStuff.h"
#include <algorithm>
#include <glm/glm.hpp>
#include "multyPlayer/createConnection.h"
#include <iostream>
#include <rendering/camera.h>
#include <lightSystem.h>
#include <platformTools.h>

Chunk* ChunkSystem::getChunkSafeFromChunkSystemCoordonates(int x, int z)
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

void ChunkSystem::createChunks(int viewDistance)
{
	squareSize = viewDistance;
	loadedChunks.resize(squareSize * squareSize, nullptr);
}

//x and z are the block positions of the player
void ChunkSystem::update(glm::ivec3 playerBlockPosition, float deltaTime, UndoQueue &undoQueue
	, LightSystem &lightSystem)
{
	//multy player stuff
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

	//index of the chunk
	int x = divideChunk(playerBlockPosition.x);
	int z = divideChunk(playerBlockPosition.z);

	//bottom left most chunk and top right most chunk of my array
	glm::ivec2 minPos = glm::ivec2(x, z) - glm::ivec2(squareSize / 2, squareSize / 2);
	glm::ivec2 maxPos = glm::ivec2(x, z) + glm::ivec2(squareSize / 2 + squareSize % 2, squareSize / 2 + squareSize % 2);
	//exclusive max

	cornerPos = minPos;

#pragma region recieve chunks by the server
	auto recievedChunk = getRecievedChunks();

	for (auto &i : recievedChunk)
	{

		//remove from recently requested chunks so we can request again
		//if needed for some reason
		{
			auto f = recentlyRequestedChunks.find({i->data.x, i->data.z});
			if (f != recentlyRequestedChunks.end())
			{
				recentlyRequestedChunks.erase(f); 
					
			}
		}

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
				i->createGpuData();
				//permaAssert(loadedChunks[x * squareSize + z] == nullptr); //no need for assert we check the if above 
				loadedChunks[x * squareSize + z] = i;

				int xBegin = i->data.x * CHUNK_SIZE;
				int zBegin = i->data.z * CHUNK_SIZE;

				{
					for (int xPos = xBegin; xPos < xBegin + CHUNK_SIZE; xPos++)
						for (int zPos = zBegin; zPos < zBegin + CHUNK_SIZE; zPos++)
						{
							if (!i->unsafeGet(xPos - xBegin, CHUNK_HEIGHT - 1, zPos - zBegin).isOpaque())
							{
								lightSystem.addSunLight(*this, {xPos, CHUNK_HEIGHT - 1, zPos}, 15);
							}
						}

					i->dontDrawYet = true;
				}

			}

		}
	}

#pragma endregion


#pragma region set chunk in matrix

	if (!created || lastX != x || lastZ != z)
	{

		std::vector<Chunk*> newChunkVector;
		newChunkVector.resize(squareSize * squareSize);

		//copy old chunks from the old chunk vector to the new one if they are kept
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
				//chunk no longer needed delete it
				loadedChunks[i]->clearGpuData();
				delete loadedChunks[i];
				loadedChunks[i] = nullptr;
			}
		}

		loadedChunks = std::move(newChunkVector);
	}

#pragma endregion

	if (lastPlayerBlockPosition != playerBlockPosition)
	{
		//player moved sooooo update transparency
		Chunk *c[9] = {};
		glm::ivec2 positions[9] = {{0,0},{CHUNK_SIZE,0},{-CHUNK_SIZE,0},{0,CHUNK_SIZE},{0,-CHUNK_SIZE},
			{-CHUNK_SIZE,CHUNK_SIZE},{CHUNK_SIZE,-CHUNK_SIZE},{CHUNK_SIZE,CHUNK_SIZE},{-CHUNK_SIZE,-CHUNK_SIZE}
		};

		for (int i = 0; i < 9; i++)
		{
			auto c = getChunkSafeFromBlockPos(playerBlockPosition.x + positions[i].x,
				playerBlockPosition.z + positions[i].y);

			if (c)
			{
				c->dirtyTransparency = true;
			}

		}
	}

	//request new chunks from the server
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
				recentlyRequestedChunks[{t.pos.x, t.pos.z}] = 10.f; //time not request again, it can be big since packets are reliable
				finalTask.push_back(t);
			}
		}

		submitTaskClient(finalTask);
	}



	created = 1;
	lastX = x;
	lastZ = z;
	lastPlayerBlockPosition = playerBlockPosition;


#pragma region place block by server
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

			if (rez)
			{
				setChunkAndNeighboursFlagDirtyFromBlockPos(pos.x, pos.z);
				rez->type = message.blockType;
			}
	
		}
		else
		{
			//todo possible problems here
			//ignore message
		}


		//remove undo queue

		for (auto &e :undoQueue.events)
		{

			if (e.type == Event::iPlacedBlock && e.blockPos == pos)
			{
				e.type = Event::doNothing;
			}

		}


	}
#pragma endregion


#pragma region bake

	int currentBaked = 0;
	const int maxToBake = 10; //this frame

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
		if (chunk == nullptr) { continue; }
		if (chunk->dontDrawYet == true) { chunk->dontDrawYet = false; continue; }

		int x = chunk->data.x - minPos.x;
		int z = chunk->data.z - minPos.y;

		if (currentBaked < maxToBake)
		{
			auto left = getChunkSafeFromChunkSystemCoordonates(x - 1, z);
			auto right = getChunkSafeFromChunkSystemCoordonates(x + 1, z);
			auto front = getChunkSafeFromChunkSystemCoordonates(x, z + 1);
			auto back = getChunkSafeFromChunkSystemCoordonates(x, z - 1);
		
			auto b = chunk->bake(left, right, front, back, playerBlockPosition);
		
			if (b) { currentBaked++; }
		}
		else
		{
			
		}

	}

#pragma endregion

}

Chunk *ChunkSystem::getChunkSafeFromBlockPos(int x, int z)
{
	int divideX = divideChunk(x);
	int divideZ = divideChunk(z);

	auto c = getChunkSafeFromChunkSystemCoordonates(divideX - cornerPos.x, divideZ - cornerPos.y);
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

Block *ChunkSystem::getBlockSafe(glm::dvec3 pos)
{
	auto p = from3DPointToBlock(pos);
	return getBlockSafe(p.x, p.y, p.z);
}


Block *ChunkSystem::getBlockSafeAndChunk(int x, int y, int z, Chunk *&chunk)
{
	chunk = nullptr;
	if (y < 0 || y >= CHUNK_HEIGHT) { return nullptr; }

	auto c = getChunkSafeFromBlockPos(x, z);

	if (!c) { return nullptr; }

	chunk = c;

	int modX = modBlockToChunk(x);
	int modZ = modBlockToChunk(z);

	auto b = c->safeGet(modX, y, modZ);

	return b;
}

Block *ChunkSystem::rayCast(glm::dvec3 from, glm::vec3 dir, glm::ivec3 &outPos,
	float maxDist, std::optional<glm::ivec3> &prevBlockForPlace)
{
	float deltaMagitude = 0.01f;
	glm::vec3 delta = glm::normalize(dir) * deltaMagitude;

	glm::dvec3 pos = from;

	prevBlockForPlace = std::nullopt;

	for (float walkedDist = 0.f; walkedDist < maxDist; walkedDist += deltaMagitude)
	{
		glm::ivec3 intPos = from3DPointToBlock(pos);
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

void ChunkSystem::changeBlockLightStuff(glm::ivec3 pos, int currentLightLevel,
	BlockType oldType, BlockType newType, LightSystem &lightSystem)
{
	
	if (!isOpaque(oldType) && isOpaque(newType))
	{
		//remove light
		lightSystem.removeSunLight(*this, pos, currentLightLevel);
	}
	else if (isOpaque(oldType) && !isOpaque(newType))
	{
		//add light
		int light = 0;
		auto checkNeighbour = [&](glm::ivec3 pos2, int decrease)
		{
			auto b = getBlockSafe(pos2.x, pos2.y, pos2.z);
			if (b && !b->isOpaque())
			{
				if (!decrease && b->getSkyLight() == 15)
				{
					light = 15;
				}
				else
				{
					light = std::max(light, b->getSkyLight() - 1);
				}
			}
		};

		checkNeighbour({pos.x+1,pos.y,pos.z}, 1);
		if(light < 15)
			checkNeighbour({pos.x-1,pos.y,pos.z}, 1);
		if (light < 15)
			checkNeighbour({pos.x,pos.y+1,pos.z}, 0);
		if (light < 15)
			checkNeighbour({pos.x,pos.y-1,pos.z}, 1);
		if (light < 15)
			checkNeighbour({pos.x,pos.y,pos.z+1}, 1);
		if (light < 15)
			checkNeighbour({pos.x,pos.y,pos.z-1}, 1);

		light = std::max(light, 0);

		if (light > 1)
		{
			lightSystem.addSunLight(*this, pos, light);
		}
		else
		{
			auto b = getBlockSafe(pos.x, pos.y, pos.z);
			if (b)
			{
				b->setSkyLevel(light);
			}
		}

	}

}

//todo short for type
void ChunkSystem::placeBlockByClient(glm::ivec3 pos, BlockType type, UndoQueue &undoQueue, glm::dvec3 playerPos
	, LightSystem &lightSystem)
{
	//todo were we will check legality locally
	Chunk *chunk = 0;
	auto b = getBlockSafeAndChunk(pos.x, pos.y, pos.z, chunk);
	
	if (b != nullptr)
	{
		Task task;
		task.type = Task::placeBlock;
		task.pos = pos;
		task.blockType = type;
		task.eventId = undoQueue.currentEventId;
		submitTaskClient(task);

		undoQueue.addPlaceBlockEvent(pos, b->type, type, playerPos);

		changeBlockLightStuff(pos, b->getSkyLight(), b->type, type, lightSystem);

		b->type = type;
		if (b->isOpaque()) { b->lightLevel = 0; }

		setChunkAndNeighboursFlagDirtyFromBlockPos(pos.x, pos.z);

	}
	
}

//todo refactor and reuse up
void ChunkSystem::placeBlockNoClient(glm::ivec3 pos, BlockType type, LightSystem &lightSystem)
{
	//todo were we will check legality locally
	Chunk *chunk = 0;
	auto b = getBlockSafeAndChunk(pos.x, pos.y, pos.z, chunk);

	if (b != nullptr)
	{
		changeBlockLightStuff(pos, b->getSkyLight(), b->type, type, lightSystem);

		b->type = type;
		if (b->isOpaque()) { b->lightLevel = 0; }

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
	return (int)floor((float)x / (float)CHUNK_SIZE);
};
