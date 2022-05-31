#include "server.h"
#include <glm/vec3.hpp>
#include "chunkSystem.h"
#include "threadStuff.h"
#include <thread>
#include <mutex>
#include <queue>
#include "worldGenerator.h"
#include <thread>
#include <unordered_map>
#include <iostream>
#include <atomic>

struct ListNode;

struct SavedChunk
{
	//std::mutex mu;
	Chunk chunk;
	ListNode *node = nullptr;
};

//https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-user-defined-class-in-cpp/
struct ChunkHash
{
	size_t operator()(const glm::ivec2 &in) const
	{
		int x = in.x;
		int z = in.y;

		size_t ret = 0;
		ret += x;
		ret += (z < 32);

		return ret;
	}
};


struct ListNode
{
	ListNode *prev = nullptr;
	glm::ivec2 chunkPos = {};
	ListNode *next = nullptr;
};


static std::atomic<bool> serverRunning = false;

bool startServer()
{
	bool expected = 0;
	if (serverRunning.compare_exchange_strong(expected, 0))
	{
		std::thread serverThread(serverFunction);
		serverThread.detach();
		return 1;
	}
	else
	{
		return 0;
	}
}

const int maxSavedChunks = 2000;

struct ChunkPriorityCache
{
	std::unordered_map<glm::ivec2, SavedChunk *, ChunkHash> savedChunks = {};
	ListNode *first = nullptr;
	ListNode *last = nullptr;

	Chunk *getOrCreateChunk(int posX, int posZ);;

};



struct ServerData
{
	ChunkPriorityCache chunkCache = {};
	std::queue<Task> waitingTasks = {};

}sd;

void serverStartupStuff()
{
	//reset data
	sd = ServerData{};


}


void serverFunction()
{

	serverStartupStuff();


	while (true)
	{
		std::vector<Task> tasks;
		if (sd.waitingTasks.empty())
		{
			tasks = waitForTasks();
		}
		else
		{
			tasks = tryForTasks();
		}

		for (auto i : tasks)
		{
			sd.waitingTasks.push(i);
		}

		int taskIndex = 0;
		int count = sd.waitingTasks.size();
		for (; taskIndex < std::min(count, 10); taskIndex++)
		{
			auto &i = sd.waitingTasks.front();

			if (i.type == Task::generateChunk)
			{
				auto rez = sd.chunkCache.getOrCreateChunk(i.pos.x, i.pos.z);
				Message mes;
				mes.type = Message::recievedChunk;
				mes.chunk = new Chunk(*rez);
				submitMessage(mes);
			}
			else
				if (i.type == Task::placeBlock)
				{
					auto chunk = sd.chunkCache.getOrCreateChunk(i.pos.x / 16, i.pos.z / 16);
					int convertedX = modBlockToChunk(i.pos.x);
					int convertedZ = modBlockToChunk(i.pos.z);

					auto b = chunk->safeGet(convertedX, i.pos.y, convertedZ);
					if (b)
					{
						b->type = i.type;

						Message mes;
						mes.type = Message::placeBlock;
						mes.pos = i.pos;
						mes.blockType = i.blockType;
						submitMessage(mes);
					}

				}

			sd.waitingTasks.pop();
		}


	}



}


Chunk *ChunkPriorityCache::getOrCreateChunk(int posX, int posZ)
{
	glm::ivec2 pos = {posX, posZ};
	auto foundPos = savedChunks.find(pos);
	if (foundPos != savedChunks.end())
	{
		auto nod = (*foundPos).second->node;

		auto left = nod->prev;
		auto right = nod->next;

		if (left == nullptr)
		{
			//nothing, this is the first element
		}
		else if (right == nullptr)
		{
			left->next = nullptr;
			last = left;

			nod->next = first;
			if (first)
			{
				first->prev = nod;
			}

			nod->prev = nullptr;
			first = nod;
		}
		else
		{
			//link left right:
			left->next = right;
			right->prev = left;

			nod->next = first;
			if (first)
			{
				first->prev = nod;
			}

			nod->prev = nullptr;
			first = nod;
		}

		//submitChunk(new Chunk((*foundPos).second->chunk));
		return &((*foundPos).second->chunk);
	}
	else
	{

		if (savedChunks.size() >= maxSavedChunks)
		{
			//std::cout << "bad";
			glm::ivec2 oldPos = last->chunkPos;
			auto foundPos = savedChunks.find(oldPos);
			assert(foundPos != savedChunks.end());//unlink of the 2 data structures

			SavedChunk *chunkToRecycle = foundPos->second;

			//here we can save the chunk to a file

			savedChunks.erase(foundPos);

			chunkToRecycle->chunk.x = pos.x;
			chunkToRecycle->chunk.z = pos.y;

			generateChunk(1234, chunkToRecycle->chunk);
			auto rez = &chunkToRecycle->chunk;
			//submitChunk(new Chunk(chunkToRecycle->chunk));

			assert(chunkToRecycle->node == last);
			last->chunkPos = pos;

			auto prev = last->prev;
			prev->next = nullptr;

			auto newFirst = last;
			last = prev;

			first->prev = newFirst;
			newFirst->prev = nullptr;
			newFirst->next = first;
			first = newFirst;

			savedChunks[pos] = chunkToRecycle;

			return rez;
		}
		else
		{

			SavedChunk *c = new SavedChunk;
			c->chunk.x = posX;
			c->chunk.z = posZ;

			generateChunk(1234, c->chunk);
			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
			//submitChunk(new Chunk(c->chunk));
			auto rez = &c->chunk;

			ListNode *node = new ListNode;
			if (last == nullptr) { last = node; }

			node->prev = nullptr;
			node->next = first;
			node->chunkPos = {posX, posZ};

			if (first)
			{
				first->prev = node;
			}

			first = node;

			c->node = node;
			savedChunks[{posX, posZ}] = c;

			return rez;
		}
	}
}
