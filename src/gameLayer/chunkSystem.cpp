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
#include <cmath>
#include <gameplay/gameplayRules.h>

Chunk *ChunkSystem::getChunksInMatrixSpaceUnsafe(int x, int z)
{
	return loadedChunks[x * squareSize + z];
}

Chunk* ChunkSystem::getChunkSafeFromMatrixSpace(int x, int z)
{
	if (x < 0 || z < 0 || x >= squareSize || z >= squareSize)
	{
		return nullptr;
	}
	else
	{
		return getChunksInMatrixSpaceUnsafe(x, z);
	}
}

void ChunkSystem::init(int viewDistance)
{
	squareSize = viewDistance;
	loadedChunks.resize(squareSize * squareSize, nullptr);
	gpuBuffer.create(squareSize * squareSize);
}

void ChunkSystem::cleanup()
{
	dropAllChunks(nullptr);
	gpuBuffer.cleanup();
}

//x and z are the block positions of the player
void ChunkSystem::update(glm::ivec3 playerBlockPosition, float deltaTime, UndoQueue &undoQueue
	, LightSystem &lightSystem)
{
	//multy player stuff

	//timeout recenrly requested chunks
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

	auto checkChunkInRadius = [&](glm::ivec2 pos)
	{
		glm::vec2 center{squareSize / 2,squareSize / 2};
		center -= glm::vec2(pos);

		return std::sqrt(glm::dot(center, center)) <= (squareSize/2);
	};

	cornerPos = minPos;

#pragma region recieve chunks by the server
	auto recievedChunk = getRecievedChunks();

	//chunksToAddLight is in chunk system coordonates space
	std::vector<glm::ivec2> chunksToAddLight;

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

		if (x < 0 || z < 0 || x >= squareSize || z >= squareSize
			|| !checkChunkInRadius({x,z})
			)
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
				shouldUpdateLights = true;

				i->createGpuData();
				//permaAssert(loadedChunks[x * squareSize + z] == nullptr); //no need for assert we check the if above 
				loadedChunks[x * squareSize + z] = i;

				int xBegin = i->data.x * CHUNK_SIZE;
				int zBegin = i->data.z * CHUNK_SIZE;

				if(!dontUpdateLightSystem)
				{
					i->setDontDrawYet(true);

					//c is in chunk system coordonates space, not chunk space!
					chunksToAddLight.push_back({x, z});

					for (int x = 0; x < CHUNK_SIZE; x++)
						for (int z = 0; z < CHUNK_SIZE; z++)
							for (int y = 0; y < CHUNK_HEIGHT; y++)
							{

								auto &b = i->unsafeGet(x, y, z);

								if (isLightEmitter(b.type))
								{
									lightSystem.addLight(*this,
										{i->data.x * CHUNK_SIZE + x, y, i->data.z * CHUNK_SIZE + z},
										15);
								}

							}

				}

			}

		}
	}

#pragma endregion


#pragma region set chunk in matrix

	if (!created || lastX != x || lastZ != z)
	{
		shouldUpdateLights = true;

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
				chunkPos.y < maxPos.y &&
				checkChunkInRadius(chunkPos-minPos)
				)
			{
				glm::ivec2 chunkPosRelToSystem = chunkPos - minPos;

				newChunkVector[chunkPosRelToSystem.x * squareSize + chunkPosRelToSystem.y] = loadedChunks[i];

				loadedChunks[i] = nullptr;
			}
			else
			{	
				//chunk no longer needed delete it
				dropChunkAtIndexUnsafe(i, &gpuBuffer);
			}
		}

		loadedChunks = std::move(newChunkVector);
	}

#pragma endregion

	//don't call chunk position related things untill this line!

#pragma region add lights

	if (!dontUpdateLightSystem)
	{

		//c is in chunk system coordonates space
		for (auto &c : chunksToAddLight)
		{
			auto chunk = getChunkSafeFromMatrixSpace(c.x, c.y);
			assert(chunk);

			
			int xStart = chunk->data.x * CHUNK_SIZE;
			int ZStart = chunk->data.z * CHUNK_SIZE;

			lightSystem.setSunlightForAnEntireChunk(*chunk, *this);

			auto leftNeighbour = getChunkSafeFromMatrixSpace(c.x - 1, c.y);
			auto rightNeighbour = getChunkSafeFromMatrixSpace(c.x + 1, c.y);
			auto frontNeighbour = getChunkSafeFromMatrixSpace(c.x, c.y + 1);
			auto backNeighbour = getChunkSafeFromMatrixSpace(c.x, c.y - 1);

			auto propagateLight = [&](Chunk *neighbour, glm::ivec3 darkBlock,
				glm::ivec3 lightBlock,
				glm::ivec3 absolutPositionLight)
			{
				auto &b = neighbour->unsafeGet(lightBlock.x, lightBlock.y, lightBlock.z);
				auto &b2 = chunk->unsafeGet(darkBlock.x, darkBlock.y, darkBlock.z);

				if (b.getSkyLight() > 1)
				{
					if (!b2.isOpaque())
					{
						lightSystem.addSunLight(*this, 
							{absolutPositionLight.x, absolutPositionLight.y, absolutPositionLight.z},
							b.getSkyLight());
					}
				}

				if (b.getLight() > 1)
				{
					if (!b2.isOpaque())
					{
						lightSystem.addLight(*this,
							{absolutPositionLight.x, absolutPositionLight.y, absolutPositionLight.z},
							b.getLight());
					}
				}
			};


			if (leftNeighbour)
			{
				for (int z = 0; z < CHUNK_SIZE; z++)
				{
					for (int y = 0; y < CHUNK_HEIGHT - 2; y++)
					{
						propagateLight(leftNeighbour, {0, y, z}, {CHUNK_SIZE - 1, y, z},
							{xStart - 1, y, ZStart + z});
					}
				}
			}

			if (rightNeighbour)
			{
				for (int z = 0; z < CHUNK_SIZE; z++)
				{
					for (int y = 0; y < CHUNK_HEIGHT - 2; y++)
					{
						propagateLight(rightNeighbour, {CHUNK_SIZE - 1, y, z}, {0, y, z},
							{xStart + CHUNK_SIZE, y, ZStart + z}
							);
					}
				}
			}
			
			if (frontNeighbour)
			{
				for (int x = 0; x < CHUNK_SIZE; x++)
				{
					for (int y = 0; y < CHUNK_HEIGHT - 2; y++)
					{
						propagateLight(frontNeighbour, {x, y, CHUNK_SIZE - 1}, {x, y, 0},
							{xStart + x, y, CHUNK_SIZE + ZStart});
					}
				}
			}

			if (backNeighbour)
			{
				for (int x = 0; x < CHUNK_SIZE; x++)
				{
					for (int y = 0; y < CHUNK_HEIGHT - 2; y++)
					{
						propagateLight(backNeighbour, {x, y, 0}, {x, y, CHUNK_SIZE - 1},
							{xStart + x, y, ZStart - 1});
					}
				}
			}

		}


	}

#pragma endregion

	glm::ivec2 playerChunkPos = fromBlockPosToChunkPos(playerBlockPosition);

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
				c->setDirtyTransparency(true);
			}

		}
	}

	//request new chunks from the server
	std::vector<Task> chunkTasks;
	for (int x = 0; x < squareSize; x++)
		for (int z = 0; z < squareSize; z++)
		{
			if (loadedChunks[x * squareSize + z] == nullptr
				&& checkChunkInRadius({x, z})
				)
			{
				Task t;

				t.type = Task::generateChunk;
				t.pos = glm::ivec3(x + minPos.x, 0, z + minPos.y);
				t.playerPosForChunkGeneration.x = playerBlockPosition.x;
				t.playerPosForChunkGeneration.y = playerBlockPosition.z;

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



		std::vector<Task> finalTask;
		finalTask.reserve(chunkSystemSettings.maxWaitingSubmisions);

		for (int i = 0; i < chunkSystemSettings.maxWaitingSubmisions; i++)
		{
			if (chunkTasks.size() <= i) { break; /*we managed to request all needed chunls*/ }

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

			placeBlockNoClient(message.blockPos, message.blockType, lightSystem);

			//Chunk *c = 0;
			//auto rez = getBlockSafeAndChunk(message.blockPos.x, message.blockPos.y, message.blockPos.z, c);
			//
			//if (rez)
			//{
			//	setChunkAndNeighboursFlagDirtyFromBlockPos(pos.x, pos.z);
			//	rez->type = message.blockType;
			//}
	
		}
		else
		{
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
	int currentBakedTransparency = 0;
	const int maxToBake = 2; //this frame //max to bake
	const int maxToBakeTransparency = 4; //this frame //max to bake

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
		if (chunk->isDontDrawYet() == true) { chunk->setDontDrawYet(false); continue; }
	
		int x = chunk->data.x - minPos.x;
		int z = chunk->data.z - minPos.y;
	
		if (currentBaked < maxToBake || currentBakedTransparency < maxToBakeTransparency)
		{
			auto left = getChunkSafeFromMatrixSpace(x - 1, z);
			auto right = getChunkSafeFromMatrixSpace(x + 1, z);
			auto front = getChunkSafeFromMatrixSpace(x, z + 1);
			auto back = getChunkSafeFromMatrixSpace(x, z - 1);
	
			if (chunk->shouldBakeOnlyBecauseOfTransparency(left, right, front, back))
			{
				if (currentBakedTransparency < maxToBakeTransparency)
				{
					auto b = chunk->bake(left, right, 
						front, back, playerBlockPosition, gpuBuffer);
					if (b) { currentBakedTransparency++; }
				}
			}
			else
			{
				if (currentBaked < maxToBake)
				{
					auto b = chunk->bake(left, right, front, back, 
						playerBlockPosition, gpuBuffer);
					if (b) { currentBaked++; }
				}
			}
		}
		else
		{
			break;
		}
	
	}

#pragma endregion

}

Chunk *ChunkSystem::getChunkSafeFromBlockPos(int x, int z)
{
	auto p = fromBlockPosToMatrixSpace(x, z);

	auto c = getChunkSafeFromMatrixSpace(p.x, p.y);
	return c;
}

Chunk *ChunkSystem::getChunkSafeFromChunkPos(int x, int z)
{
	auto c = getChunkSafeFromMatrixSpace(x - cornerPos.x, z - cornerPos.y);
	return c;
}

glm::ivec2 ChunkSystem::fromBlockPosToMatrixSpace(int x, int z)
{
	int divideX = divideChunk(x);
	int divideZ = divideChunk(z);

	return{divideX - cornerPos.x, divideZ - cornerPos.y};
}

glm::ivec2 ChunkSystem::fromMatrixSpaceToChunkSpace(int x, int z)
{
	return glm::ivec2(x + cornerPos.x, z + cornerPos.y);
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
			c->setDirty(true);
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

void ChunkSystem::getBlockSafeWithNeigbhours(int x, int y, int z, 
	Block *&center, Block *&front, Block *&back,
	Block *&top, Block *&bottom, Block *&left, Block *&right)
{

	if (y < -1 || y >= CHUNK_HEIGHT+1) 
	{ 
		center = 0;
		front = 0;
		back = 0;
		top = 0;
		bottom = 0;
		left = 0;
		right = 0;
		return; 
	}
	
	auto centerChunkPos = fromBlockPosToMatrixSpace(x, z);
	auto centerChunk = getChunkSafeFromMatrixSpace(centerChunkPos.x,
		centerChunkPos.y);

	//front = getBlockSafe(x + 1, y, z);
	//back = getBlockSafe(x - 1, y, z);
	//left = getBlockSafe(x, y, z - 1);
	//right = getBlockSafe(x - 1, y, z + 1);
	//top = getBlockSafe(x, y + 1, z);
	//bottom = getBlockSafe(x, y - 1, z);
	//center = getBlockSafe(x, y, z);
	//return;

	if (centerChunk) //[[likely]]
	{
		int centerInChunkPosX = modBlockToChunk(x);
		int centerInChunkPosZ = modBlockToChunk(z);

		if (y > 0)
		{
			bottom = &centerChunk->unsafeGet(centerInChunkPosX, y-1, centerInChunkPosZ);
		}
		else
		{
			bottom = nullptr;
		}

		if (y < CHUNK_SIZE - 1)
		{
			top = &centerChunk->unsafeGet(centerInChunkPosX, y + 1, centerInChunkPosZ);
		}
		else
		{
			top = nullptr;
		}

		if (y >= 0 && y < CHUNK_SIZE)
		{
			center = &centerChunk->unsafeGet(centerInChunkPosX, y, centerInChunkPosZ);

			if (centerInChunkPosX < CHUNK_SIZE - 1)
			{
				front = &centerChunk->unsafeGet(centerInChunkPosX + 1, y, centerInChunkPosZ);
			}
			else
			{
				front = getBlockSafe(x + 1, y, z);
			}

			if (centerInChunkPosX > 0)
			{
				back = &centerChunk->unsafeGet(centerInChunkPosX - 1, y, centerInChunkPosZ);
			}
			else
			{
				back = getBlockSafe(x - 1, y, z);
			}

			if (centerInChunkPosZ > 0)
			{
				left = &centerChunk->unsafeGet(centerInChunkPosX, y, centerInChunkPosZ - 1);
			}else
			{
				left = getBlockSafe(x, y, z - 1);
			}

			if (centerInChunkPosZ < CHUNK_SIZE - 1)
			{
				right = &centerChunk->unsafeGet(centerInChunkPosX, y, centerInChunkPosZ + 1);
			}
			else
			{
				right = getBlockSafe(x, y, z + 1);
			}
		}
		else
		{
			front = 0;
			back = 0;
			left = 0;
			right = 0;
			center = 0;
		}
		
	}
	else
	{
		center = getBlockSafe(x, y, z);
		top = getBlockSafe(x, y + 1, z);
		bottom = getBlockSafe(x, y - 1, z);
		front = getBlockSafe(x + 1, y, z);
		back = getBlockSafe(x - 1, y, z);
		left = getBlockSafe(x, y, z - 1);
		right = getBlockSafe(x - 1, y, z + 1);
	}
}

void ChunkSystem::getBlockSafeWithNeigbhoursStopIfCenterFails(int x, int y, int z, Block *&center,
	Block *&front, Block *&back, Block *&top, Block *&bottom, Block *&left, Block *&right)
{
	if (y < 0 || y >= CHUNK_HEIGHT)
	{
		center = 0;
		front = 0;
		back = 0;
		top = 0;
		bottom = 0;
		left = 0;
		right = 0;
		return;
	}

	int centerChunkX = divideChunk(x);
	int centerChunkZ = divideChunk(z);
	auto centerChunk = getChunkSafeFromMatrixSpace(centerChunkX - cornerPos.x,
		centerChunkZ - cornerPos.y);

	if (centerChunk) //[[likely]]
	{
		int centerInChunkPosX = modBlockToChunk(x);
		int centerInChunkPosZ = modBlockToChunk(z);

		if (y > 0)
		{
			bottom = &centerChunk->unsafeGet(centerInChunkPosX, y - 1, centerInChunkPosZ);
		}
		else
		{
			bottom = nullptr;
		}

		if (y < CHUNK_SIZE - 1)
		{
			top = &centerChunk->unsafeGet(centerInChunkPosX, y + 1, centerInChunkPosZ);
		}
		else
		{
			top = nullptr;
		}

		
		center = &centerChunk->unsafeGet(centerInChunkPosX, y, centerInChunkPosZ);

		if (centerInChunkPosX < CHUNK_SIZE - 1)
		{
			front = &centerChunk->unsafeGet(centerInChunkPosX + 1, y, centerInChunkPosZ);
		}
		else
		{
			auto frontChunk = getChunkSafeFromMatrixSpace(centerChunkX - cornerPos.x + 1,
				centerChunkZ - cornerPos.y);
			if (frontChunk)
			{
				front = &frontChunk->unsafeGet(0, y, centerInChunkPosZ);
			}
			else
			{
				front = 0;
			}
		}

		if (centerInChunkPosX > 0)
		{
			back = &centerChunk->unsafeGet(centerInChunkPosX - 1, y, centerInChunkPosZ);
		}
		else
		{
			auto backChunk = getChunkSafeFromMatrixSpace(centerChunkX - cornerPos.x - 1,
				centerChunkZ - cornerPos.y);
			if (backChunk)
			{
				back = &backChunk->unsafeGet(CHUNK_SIZE - 1, y, centerInChunkPosZ);
			}
			else
			{
				back = 0;
			}
		}

		if (centerInChunkPosZ > 0)
		{
			left = &centerChunk->unsafeGet(centerInChunkPosX, y, centerInChunkPosZ - 1);
		}
		else
		{
			auto leftChunk = getChunkSafeFromMatrixSpace(centerChunkX - cornerPos.x,
				centerChunkZ - cornerPos.y - 1);
			if (leftChunk)
			{
				left = &leftChunk->unsafeGet(centerInChunkPosX, y, CHUNK_SIZE - 1);
			}
			else
			{
				left = 0;
			}

		}

		if (centerInChunkPosZ < CHUNK_SIZE - 1)
		{
			right = &centerChunk->unsafeGet(centerInChunkPosX, y, centerInChunkPosZ + 1);
		}
		else
		{
			auto rightChunk = getChunkSafeFromMatrixSpace(centerChunkX - cornerPos.x,
				centerChunkZ - cornerPos.y + 1);
			if (rightChunk)
			{
				right = &rightChunk->unsafeGet(centerInChunkPosX, y, 0);
			}
			else
			{
				right = 0;
			}
		}
		

	}
	else
	{
		center = 0;
		front = 0;
		back = 0;
		top = 0;
		bottom = 0;
		left = 0;
		right = 0;
		return;
	}
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

void ChunkSystem::changeBlockLightStuff(glm::ivec3 pos, int currentSkyLightLevel, 
	int currentNormalLightLevel,
	BlockType oldType, BlockType newType, LightSystem &lightSystem)
{

	if (!isOpaque(oldType) && isOpaque(newType))
	{
		//remove light
		lightSystem.removeSunLight(*this, pos, currentSkyLightLevel);

		lightSystem.removeLight(*this, pos, currentNormalLightLevel);
	}
	else if (isOpaque(oldType) && !isOpaque(newType))
	{
		//add light
		//sky light
		{
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

			checkNeighbour({pos.x + 1,pos.y,pos.z}, 1);
			if (light < 15)
				checkNeighbour({pos.x - 1,pos.y,pos.z}, 1);
			if (light < 15)
				checkNeighbour({pos.x,pos.y + 1,pos.z}, 0);
			if (light < 15)
				checkNeighbour({pos.x,pos.y - 1,pos.z}, 1);
			if (light < 15)
				checkNeighbour({pos.x,pos.y,pos.z + 1}, 1);
			if (light < 15)
				checkNeighbour({pos.x,pos.y,pos.z - 1}, 1);

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

		//normal light
		{
			int light = 0;
			auto checkNeighbour = [&](glm::ivec3 pos2)
			{
				auto b = getBlockSafe(pos2.x, pos2.y, pos2.z);
				if (b && !b->isOpaque())
				{
					light = std::max(light, b->getLight() - 1);
				}
			};

			checkNeighbour({pos.x + 1,pos.y,pos.z});
			if (light < 14)
				checkNeighbour({pos.x - 1,pos.y,pos.z});
			if (light < 14)
				checkNeighbour({pos.x,pos.y + 1,pos.z});
			if (light < 14)
				checkNeighbour({pos.x,pos.y - 1,pos.z});
			if (light < 14)
				checkNeighbour({pos.x,pos.y,pos.z + 1});
			if (light < 14)
				checkNeighbour({pos.x,pos.y,pos.z - 1});

			light = std::max(light, 0);

			if (light > 1)
			{
				lightSystem.addLight(*this, pos, light);
			}
			else
			{
				auto b = getBlockSafe(pos.x, pos.y, pos.z);
				if (b)
				{
					b->setLightLevel(0);
				}
			}
		}

	}

	if (isLightEmitter(oldType) && !isLightEmitter(newType))
	{
		//remove light
		lightSystem.removeLight(*this, pos, currentNormalLightLevel);
	}
	else if (!isLightEmitter(oldType) && isLightEmitter(newType))
	{
		lightSystem.addLight(*this, pos, 15);
	}

}

void ChunkSystem::dropAllChunks(BigGpuBuffer *gpuBuffer)
{
	for (int i = 0; i < loadedChunks.size(); i++)
	{
		dropChunkAtIndexSafe(i, gpuBuffer);
	}
}

void ChunkSystem::dropChunkAtIndexUnsafe(int index, BigGpuBuffer *gpuBuffer)
{
	loadedChunks[index]->clearGpuData(gpuBuffer);
	delete loadedChunks[index];
	loadedChunks[index] = nullptr;

}

void ChunkSystem::dropChunkAtIndexSafe(int index, BigGpuBuffer *gpuBuffer)
{
	if (loadedChunks[index] != nullptr)
	{
		dropChunkAtIndexUnsafe(index, gpuBuffer);
	}
}


bool ChunkSystem::placeBlockByClient(glm::ivec3 pos, BlockType type, 
	UndoQueue &undoQueue, glm::dvec3 playerPos
	, LightSystem &lightSystem)
{
	Chunk *chunk = 0;
	auto b = getBlockSafeAndChunk(pos.x, pos.y, pos.z, chunk);
	
	if (b != nullptr)
	{

		//todo check mob colisions


		if (canBlockBePlaced(type, b->type))
		{
			Task task;
			task.type = Task::placeBlock;
			task.pos = pos;
			task.blockType = type;
			task.eventId = undoQueue.currentEventId;
			submitTaskClient(task);

			undoQueue.addPlaceBlockEvent(pos, b->type, type, playerPos);

			changeBlockLightStuff(pos, b->getSkyLight(), b->getLight(), b->type, type, lightSystem);

			b->type = type;
			if (b->isOpaque()) { b->lightLevel = 0; }

			setChunkAndNeighboursFlagDirtyFromBlockPos(pos.x, pos.z);

			return true;
		}
		else
		{
			return false;
		}
	}
	
	return false;
}


void ChunkSystem::placeBlockNoClient(glm::ivec3 pos, BlockType type, LightSystem &lightSystem)
{

	//this is forcely placed by server
	Chunk *chunk = 0;
	auto b = getBlockSafeAndChunk(pos.x, pos.y, pos.z, chunk);

	if (b != nullptr)
	{
		changeBlockLightStuff(pos, b->getSkyLight(), b->getLight(), b->type, type, lightSystem);

		b->type = type;
		if (b->isOpaque()) { b->lightLevel = 0; }

		setChunkAndNeighboursFlagDirtyFromBlockPos(pos.x, pos.z);
	}
}
