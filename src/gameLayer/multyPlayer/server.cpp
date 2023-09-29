#include "multyPlayer/server.h"
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
#include <enet/enet.h>
#include "multyPlayer/packet.h"
#include "multyPlayer/enetServerFunction.h"
#include <platformTools.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <structure.h>
#include <biome.h>

struct ListNode;

struct SavedChunk
{
	//std::mutex mu;
	ChunkData chunk;
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
std::thread serverThread;

bool serverStartupStuff();

bool isServerRunning()
{
	return serverRunning;
}

bool startServer()
{
	bool expected = 0;
	if (serverRunning.compare_exchange_strong(expected, 1))
	{
		if (!serverStartupStuff())
		{
			serverRunning = false;
			return 0;
		}

		serverThread = std::move(std::thread(serverWorkerFunction));
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

	//uses chunk coorodonates
	ChunkData *getOrCreateChunk(int posX, int posZ, WorldGenerator &wg, 
		StructuresManager &structureManager, BiomesManager &biomesManager);

	//uses chunk coorodonates
	ChunkData *getChunkOrGetNullNotUpdateTable(int posX, int posZ);

	void generateStructure(StructureToGenerate s, StructuresManager &structureManager);
	
	void generateStructure(StructureToGenerate s, StructureData *structure, int rotation);

};

std::mutex serverSettingsMutex;

struct ServerData
{
	ChunkPriorityCache chunkCache = {};
	std::vector<ServerTask> waitingTasks = {};
	ENetHost *server = nullptr;
	ServerSettings settings = {};

}sd;

void closeServer()
{
	//toto cleanup stuff
	if (serverRunning)
	{
		closeEnetListener();

		//close loop
		serverRunning = false;

		//then signal the barier from the task waiting
		signalWaitingFromServer();

		//then wait for the server to close
		serverThread.join();

		enet_host_destroy(sd.server);
		//todo clear othher stuff
		sd = {};
	}

	serverSettingsMutex.unlock();
}

bool serverStartupStuff()
{
	//reset data
	sd = ServerData{};


	//start enet server
	ENetAddress adress;
	adress.host = ENET_HOST_ANY;
	adress.port = 7771;
	ENetEvent event;

	//first param adress, players limit, channels, bandwith limit
	sd.server = enet_host_create(&adress, 32, SERVER_CHANNELS, 0, 0);

	if (!sd.server)
	{
		//todo some king of error reporting to the player
		return 0;
	}

	if (!startEnetListener(sd.server))
	{
		enet_host_destroy(sd.server);
		sd.server = 0;
		return 0;
	}

	return true;
}

void serverWorkerFunction()
{
	StructuresManager structuresManager;
	BiomesManager biomesManager;

	if (!structuresManager.loadAllStructures())
	{
		exit(0); //todo error out
	}

	if (!biomesManager.loadAllBiomes())
	{
		exit(0); //todo error out
	}

	WorldGenerator wg;
	wg.init();

	std::ifstream f(RESOURCES_PATH "gameData/worldGenerator/default.mgenerator");
	if (f.is_open())
	{
		std::stringstream buffer;
		buffer << f.rdbuf();
		WorldGeneratorSettings s;
		if (s.loadSettings(buffer.str().c_str()))
		{
			wg.applySettings(s);
		}
		else
		{
			exit(0); //todo error out
		}
		f.close();
	}

	while (serverRunning)
	{
		auto settings = getServerSettingsCopy();

		std::vector<ServerTask> tasks;
		if (sd.waitingTasks.empty())
		{
			tasks = waitForTasksServer(); //nothing to do we can wait.
		}
		else
		{
			tasks = tryForTasksServer(); //already things to do, we just grab more if ready and wating.
		}

		for (auto i : tasks)
		{
			sd.waitingTasks.push_back(i);
		}

		std::sort(sd.waitingTasks.begin(), sd.waitingTasks.end(),
			[](const ServerTask &a, const ServerTask &b) { return a.t.type < b.t.type; });

		int count = sd.waitingTasks.size();
		for (int taskIndex = 0; taskIndex < std::min(count, 3); taskIndex++)
		{
			auto &i = sd.waitingTasks.front();

			if (i.t.type == Task::generateChunk)
			{
				auto rez = sd.chunkCache.getOrCreateChunk(i.t.pos.x, i.t.pos.z, wg, structuresManager, biomesManager);
				
				Packet packet;
				packet.cid = i.cid;
				packet.header = headerRecieveChunk;

				Packet_RecieveChunk *packetData = new Packet_RecieveChunk(); //todo ring buffer here

				packetData->chunk = *rez;

				auto client = getClient(i.cid); //todo this could fail when players leave so return pointer and check

				sendPacket(client.peer, packet, (char *)packetData, sizeof(Packet_RecieveChunk), true, 0);

				delete packetData;
			}
			else
			if (i.t.type == Task::placeBlock)
			{
				//std::cout << "server recieved place block\n";
				//auto chunk = sd.chunkCache.getOrCreateChunk(i.t.pos.x / 16, i.t.pos.z / 16);
				auto chunk = sd.chunkCache.getOrCreateChunk(divideChunk(i.t.pos.x), divideChunk(i.t.pos.z), wg, structuresManager
					,biomesManager);
				int convertedX = modBlockToChunk(i.t.pos.x);
				int convertedZ = modBlockToChunk(i.t.pos.z);
				
				//todo check if place is legal
				bool legal = 1;
				bool noNeedToNotifyUndo = 0;
				auto client = getClient(i.cid);

				if (client.revisionNumber > i.t.eventId.revision)
				{
					//if the revision number is increased it means that we already undoed all those moes
					legal = false;
					noNeedToNotifyUndo = true;
					std::cout << "Server revision number ignore: " << client.revisionNumber << " "
						<< i.t.eventId.revision << "\n";
				}

				{
					auto f = settings.perClientSettings.find(i.cid);
					if (f != settings.perClientSettings.end())
					{
						if (!f->second.validateStuff)
						{
							legal = false;
						}

					}
				}

				auto b = chunk->safeGet(convertedX, i.t.pos.y, convertedZ);
				if (b && legal)
				{
					b->type = i.t.blockType;
					//todo mark this chunk dirty if needed for saving

					//todo enum for chanels
					{
						Packet packet;
						packet.cid = i.cid;
						packet.header = headerPlaceBlock;

						Packet_PlaceBlock packetData;
						packetData.blockPos = i.t.pos;
						packetData.blockType = i.t.blockType;


						broadCast(packet, &packetData, sizeof(Packet_PlaceBlock), client.peer, true, 0);
						//sendPacket(client.peer, packet, (char *)&packetData, sizeof(Packet_PlaceBlock), true, 0);
					}

					{
						Packet packet;
						packet.cid = i.cid;
						packet.header = headerValidateEvent;


						Packet_ValidateEvent packetData;
						packetData.eventId = i.t.eventId;

						sendPacket(client.peer, packet, (char *)&packetData, sizeof(Packet_ValidateEvent), true, 0);
					}


				}
				else if(!noNeedToNotifyUndo)
				{
					Packet packet;
					packet.cid = i.cid;
					packet.header = headerInValidateEvent;

					Packet_InValidateEvent packetData;
					packetData.eventId = i.t.eventId;

					client.revisionNumber++;

					sendPacket(client.peer, packet, (char *)&packetData, sizeof(Packet_ValidateEvent), true, 0);
				}

			}

			sd.waitingTasks.erase(sd.waitingTasks.begin());

			if (i.t.type == Task::generateChunk)
			{
				break;
			}
		}


	}

	wg.clear();
	structuresManager.clear();
}

ServerSettings getServerSettingsCopy()
{
	serverSettingsMutex.lock();
	auto copy = sd.settings;
	serverSettingsMutex.unlock();
	return copy;
}

void setServerSettings(ServerSettings settings)
{
	serverSettingsMutex.lock();
	for (auto &s : sd.settings.perClientSettings)
	{
		auto it = settings.perClientSettings.find(s.first);
		if (it != settings.perClientSettings.end())
		{
			s.second = it->second;
		}
	}
	serverSettingsMutex.unlock();
}

void addCidToServerSettings(CID cid)
{
	serverSettingsMutex.lock();
	sd.settings.perClientSettings.insert({cid, {}});
	serverSettingsMutex.unlock();
}

void removeCidFromServerSettings(CID cid)
{
	serverSettingsMutex.lock();
	sd.settings.perClientSettings.erase(cid);
	serverSettingsMutex.unlock();
}


ChunkData *ChunkPriorityCache::getOrCreateChunk(int posX, int posZ, WorldGenerator &wg, 
	StructuresManager &structureManager, BiomesManager &biomesManager)
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
		std::vector<StructureToGenerate> newStructures;
		newStructures.reserve(10); //todo cache up

		ChunkData *rez = 0;

		//auto startTimer = std::chrono::high_resolution_clock::now();

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

			generateChunk(chunkToRecycle->chunk, wg, structureManager, biomesManager, newStructures);
			rez = &chunkToRecycle->chunk;
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
		}
		else
		{

			SavedChunk *c = new SavedChunk;
			c->chunk.x = posX;
			c->chunk.z = posZ;

			generateChunk(c->chunk, wg, structureManager, biomesManager, newStructures);
			rez = &c->chunk;

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
		}

		for (auto &s : newStructures)
		{
			generateStructure(s, structureManager);
		}

		//auto elapsed = std::chrono::high_resolution_clock::now() - startTimer;

		//long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
		//	elapsed).count();

		//std::cout << "Chunk timer ms: " << milliseconds << "\n";

		return rez;
	}
}

ChunkData *ChunkPriorityCache::getChunkOrGetNullNotUpdateTable(int posX, int posZ)
{
	glm::ivec2 pos = {posX, posZ};
	auto foundPos = savedChunks.find(pos);

	if (foundPos != savedChunks.end())
	{
		return &((*foundPos).second->chunk);
	}
	else
	{
		return nullptr;
	}
}

void ChunkPriorityCache::generateStructure(StructureToGenerate s, StructuresManager &structureManager)
{

	auto chooseRandomElement = [](float randVal, int elementCount)
	{
		return int(floor(randVal * elementCount));
	};



	if (s.type == Structure_Tree)
	{

		auto tree = structureManager.trees
			[chooseRandomElement(s.randomNumber1, structureManager.trees.size())];

		generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4) );
	}else 
	if (s.type == Structure_JungleTree)
	{

		auto tree = structureManager.jungleTrees
			[chooseRandomElement(s.randomNumber1, structureManager.jungleTrees.size())];

		generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4));
	}if (s.type == Structure_PalmTree)
	{

		auto tree = structureManager.palmTrees
			[chooseRandomElement(s.randomNumber1, structureManager.palmTrees.size())];

		generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4));
	}

}

void ChunkPriorityCache::generateStructure(StructureToGenerate s, StructureData *structure, int rotation)
{
	auto size = structure->size;

	if (s.pos.y + size.y <= CHUNK_HEIGHT)
	{

		glm::ivec3 startPos = s.pos;
		startPos.x -= size.x / 2;
		startPos.z -= size.z / 2;
		glm::ivec3 endPos = startPos + size;
		
		for (int x = startPos.x; x < endPos.x; x++)
			for (int z = startPos.z; z < endPos.z; z++)
			{

				int chunkX = divideChunk(x);
				int chunkZ = divideChunk(z);

				auto c = getChunkOrGetNullNotUpdateTable(chunkX, chunkZ);

				if (c)
				{
					int inChunkX = modBlockToChunk(x);
					int inChunkZ = modBlockToChunk(z);

					for (int y = startPos.y; y < endPos.y; y++)
					{

						//todo server side problems for chunks that were already on the client

						auto &b = c->unsafeGet(inChunkX, y, inChunkZ);

						if (b.type == BlockTypes::air)
						{
							b.type = structure->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z, 
								rotation);
							
							if (b.type > BlockTypes::jungle_leaves)
							{
								int a = 0;
							}

						}
					}
				}
				else
				{
					//todo ghost blocks or something
				}

			}

	}
}
