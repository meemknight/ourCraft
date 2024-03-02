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
#include <unordered_set>
#include <profilerLib.h>
#include <gameplay/entityManagerServer.h>


bool operator==(const BlockInChunkPos &a, const BlockInChunkPos &b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

struct ListNode;

struct SavedChunk
{
	//std::mutex mu;
	ChunkData chunk;
	ListNode *node = nullptr;
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
	struct GhostBlock
	{
		BlockType type;
		bool replaceAnything = 0;

		bool operator==(const GhostBlock &other)
		{
			return type == other.type && replaceAnything == other.replaceAnything;
		}
	};


	std::unordered_map<glm::ivec2, std::unordered_map<BlockInChunkPos, GhostBlock, BlockInChunkHash>, Ivec2Hash>
		ghostBlocks;

	std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> savedChunks = {};
	ListNode *first = nullptr;
	ListNode *last = nullptr;

	struct SendBlocksBack
	{
		glm::ivec3 pos;
		BlockType block;
	};

	//uses chunk coorodonates
	ChunkData *getOrCreateChunk(int posX, int posZ, WorldGenerator &wg, 
		StructuresManager &structureManager, BiomesManager &biomesManager
		,std::vector<SendBlocksBack> &sendNewBlocksToPlayers, bool generateGhostAndStructures, 
			std::vector<StructureToGenerate> *newStructuresToAdd, bool *wasGenerated = 0);

	Block *tryGetBlockIfChunkExistsNoChecks(glm::ivec3 pos);

	//uses chunk coorodonates
	ChunkData *getChunkOrGetNullNotUpdateTable(int posX, int posZ);

	bool generateStructure(StructureToGenerate s, StructuresManager &structureManager,
		std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
		std::vector<glm::ivec3> *controlBlocks);
	
	bool generateStructure(StructureToGenerate s, StructureData *structure, int rotation,
		std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
		std::vector<glm::ivec3> *controlBlocks, bool replace = 0, BlockType from = 0, BlockType to = 0
		);

	void placeGhostBlocksForChunk(int posX, int posZ, ChunkData &c);

};

std::mutex serverSettingsMutex;

struct ServerData
{
	ChunkPriorityCache chunkCache = {};
	std::vector<ServerTask> waitingTasks = {};
	ENetHost *server = nullptr;
	ServerSettings settings = {};

	float updateEntititiesTimer = 0;

}sd;

int getChunkCapacity()
{
	return sd.chunkCache.savedChunks.size();
}

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

	//serverSettingsMutex.unlock();
}

void doGameTick(float deltaTime, ServerEntityManager &entityManager, std::uint64_t currentTimer);

//Note: it is a problem that the block validation and the item validation are on sepparate threads.
bool computeRevisionStuff(Client &client, bool allowed, const EventId &eventId)
{
	bool noNeedToNotifyUndo = false;

	if (client.revisionNumber > eventId.revision)
	{
		//if the revision number is increased it means that we already undoed all those moves
		allowed = false;
		noNeedToNotifyUndo = true;
		//std::cout << "Server revision number ignore: " << client->revisionNumber << " "
		//	<< i.t.eventId.revision << "\n";
	}


	//validate event
	if(allowed)
	{
		Packet packet;
		packet.header = headerValidateEvent;

		Packet_ValidateEvent packetData;
		packetData.eventId = eventId;

		sendPacket(client.peer, packet,
			(char *)&packetData, sizeof(Packet_ValidateEvent),
			true, channelChunksAndBlocks);
	}
	else if (!noNeedToNotifyUndo)
	{
		Packet packet;
		//packet.cid = i.cid;
		packet.header = headerInValidateEvent;

		Packet_InValidateEvent packetData;
		packetData.eventId = eventId;

		client.revisionNumber++;

		sendPacket(client.peer, packet, (char *)&packetData,
			sizeof(Packet_ValidateEvent), true, channelChunksAndBlocks);
	}

	return allowed;
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

	ServerEntityManager entityManager;

	auto timerStart = std::chrono::high_resolution_clock::now();

	float tickTimer = 0;
	float tickDeltaTime = 0;
	int ticksPerSeccond = 0;
	int runsPerSeccond = 0;
	float seccondsTimer = 0;
	constexpr float RESOLUTION_TIMER = 16.f;

	while (serverRunning)
	{
		auto settings = getServerSettingsCopy();
		//auto clientsCopy = getAllClients();
		//todo this could fail? if the client disconected?
	
		{
			std::vector<ServerTask> tasks;

			//if (!settings.busyWait)
			//{
			//	if (sd.waitingTasks.empty())
			//	{
			//
			//		if (tickTimer > (1.f / settings.targetTicksPerSeccond))
			//		{
			//			tasks = tryForTasksServer(); //we will soon need to tick so we don't block
			//		}
			//		else
			//		{
			//			tasks = waitForTasksServer(); //nothing to do we can wait.
			//		}
			//
			//	}
			//	else
			//	{
			//		tasks = tryForTasksServer(); //already things to do, we just grab more if ready and wating.
			//	}
			//}
			//else
			{

				while (sd.waitingTasks.empty())
				{
					auto timerStop = std::chrono::high_resolution_clock::now();
					float deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(timerStop - timerStart)).count() / 1000000.0f;

					if ((tickTimer + deltaTime) > 1.f / settings.targetTicksPerSeccond)
					{
						break;
					}
				}

				tasks = tryForTasksServer();
			}
			

			for (auto i : tasks)
			{
				sd.waitingTasks.push_back(i);
			}
		}

		auto timerStop = std::chrono::high_resolution_clock::now();
		float deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(timerStop - timerStart)).count() / 1000000.0f;
		timerStart = std::chrono::high_resolution_clock::now();

		auto currentTimer = getTimer();
		tickTimer += deltaTime;
		seccondsTimer += deltaTime;
		tickDeltaTime += deltaTime;

		//todo rather than a sort use buckets, so the clients can't DDOS the server with
		//place blocks tasks, making generating chunks impossible. 
		// 
		// actually be very carefull how you cange the order, you can easily break stuff
		// so I'll leave this commented for now
		// 
		//std::stable_sort(sd.waitingTasks.begin(), sd.waitingTasks.end(),
		//	[](const ServerTask &a, const ServerTask &b) 
		//	{ 
		//		if((a.t.type == Task::placeBlock && b.t.type == Task::droppedItemEntity) ||
		//			(a.t.type == Task::droppedItemEntity && b.t.type == Task::placeBlock))
		//		{
		//			return false;
		//		}
		//		return a.t.type < b.t.type; 
		//	}
		//);

		std::vector<ChunkPriorityCache::SendBlocksBack> sendNewBlocksToPlayers;

		int count = sd.waitingTasks.size();
		for (int taskIndex = 0; taskIndex < std::min(count, 15); taskIndex++)
		{
			auto &i = sd.waitingTasks.front();

			if (i.t.type == Task::generateChunk)
			{
				auto client = getClient(i.cid); //todo this could fail when players leave so return pointer and check
				
				if (checkIfPlayerShouldGetChunk(client.positionForChunkGeneration, 
					{i.t.pos.x, i.t.pos.z}, client.playerData.chunkDistance))
				{
					PL::Profiler profiler;

					profiler.start();
					bool wasGenerated = 0;
					auto rez = sd.chunkCache.getOrCreateChunk(i.t.pos.x, i.t.pos.z, wg, structuresManager, biomesManager,
						sendNewBlocksToPlayers, true, nullptr, &wasGenerated);
					profiler.end();

					//if (wasGenerated)
					//{
					//	std::cout << "Generated ChunK: " << profiler.rezult.timeSeconds / 1000.f << "ms\n";
					//}

					Packet packet;
					packet.cid = i.cid; //todo is this cid here needed? should I just put 0?
					packet.header = headerRecieveChunk;


					//if you have modified Packet_RecieveChunk make sure you didn't break this!
					static_assert(sizeof(Packet_RecieveChunk) == sizeof(ChunkData));


					{
						lockConnectionsMutex();
						auto client = getClientNotLocked(i.cid);
						if (client)
						{
						sendPacket(client->peer, packet, (char *)rez,
							sizeof(Packet_RecieveChunk), true, channelChunksAndBlocks);
						}
						unlockConnectionsMutex();
					}

				}
				else
				{
					std::cout << "Chunk rejected because player too far: " <<
						i.t.pos.x << " " << i.t.pos.z << " dist: " << client.playerData.chunkDistance << "\n";
				}

				
			}
			else
			if (i.t.type == Task::placeBlock)
			{
				bool wasGenerated = 0;
				//std::cout << "server recieved place block\n";
				//auto chunk = sd.chunkCache.getOrCreateChunk(i.t.pos.x / 16, i.t.pos.z / 16);
				auto chunk = sd.chunkCache.getOrCreateChunk(divideChunk(i.t.pos.x), divideChunk(i.t.pos.z), wg, structuresManager
					,biomesManager, sendNewBlocksToPlayers, true, nullptr, &wasGenerated);
				int convertedX = modBlockToChunk(i.t.pos.x);
				int convertedZ = modBlockToChunk(i.t.pos.z);
				
				//todo check if place is legal
				bool noNeedToNotifyUndo = 0;
				lockConnectionsMutex();

				auto client = getClientNotLocked(i.cid); 
				
				if (client)
				{
					//todo hold this with a full mutex in this
					//  thread everywhere because the other thread can change connections
					//todo check in other places in this thread

					bool legal = 1;
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

					if (!b)
					{
						legal = false;
					}
					
					legal = computeRevisionStuff(*client, legal, i.t.eventId);

					if (legal)
					{
						b->type = i.t.blockType;
						//todo mark this chunk dirty if needed for saving

						{
							Packet packet;
							packet.cid = i.cid;
							packet.header = headerPlaceBlocks;

							Packet_PlaceBlocks packetData;
							packetData.blockPos = i.t.pos;
							packetData.blockType = i.t.blockType;

							broadCastNotLocked(packet, &packetData, sizeof(Packet_PlaceBlocks),
								client->peer, true, channelChunksAndBlocks);
						}

					}
					
				}

				unlockConnectionsMutex();

			}
			else if (i.t.type == Task::droppedItemEntity)
			{

				lockConnectionsMutex();
				auto client = getClientNotLocked(i.cid);
				
				//todo some logic
				//todo add items here into the server's storage
				if(client)
				{
				
					auto serverAllows = getClientSettingCopy(i.cid).validateStuff;
					if (entityManager.entityExists(i.t.entityId)) { serverAllows = false; }


					if (computeRevisionStuff(*client, true && serverAllows, i.t.eventId))
					{
						
						DroppedItemNetworked newEntity = {};
						newEntity.item.count = i.t.blockCount;
						newEntity.item.position = i.t.doublePos;
						newEntity.item.lastPosition = i.t.doublePos;
						newEntity.item.type = i.t.blockType;
						newEntity.item.forces = i.t.motionState;
						newEntity.restantTime = computeRestantTimer(i.t.timer, currentTimer);

						entityManager.droppedItems.insert({i.t.entityId, newEntity});
					}
					
				}
				else
				{
				}
				
				unlockConnectionsMutex();

			}

			sd.waitingTasks.erase(sd.waitingTasks.begin());

			//we generate only one chunk per loop
			if (i.t.type == Task::generateChunk)
			{
				break;
			}
		}

		//this are blocks created by new chunks so everyone needs them
		if (!sendNewBlocksToPlayers.empty())
		{
			Packet_PlaceBlocks *newBlocks = new Packet_PlaceBlocks[sendNewBlocksToPlayers.size()];
			
			Packet packet;
			packet.cid = 0;
			packet.header = headerPlaceBlocks;

			int i = 0;
			for (auto &b : sendNewBlocksToPlayers)
			{
				//todo an option to send multiple blocks per place block
				//std::cout << "Sending block...";

				//Packet packet;
				//packet.cid = 0;
				//packet.header = headerPlaceBlock;
				//
				//Packet_PlaceBlock packetData;
				//packetData.blockPos = b.pos;
				//packetData.blockType = b.block;
				//
				//broadCast(packet, &packetData, sizeof(Packet_PlaceBlock), nullptr, true, channelChunksAndBlocks);
					
				newBlocks[i].blockPos = b.pos;
				newBlocks[i].blockType = b.block;

				i++;
			}

			broadCast(packet, newBlocks, 
				sizeof(Packet_PlaceBlocks) *sendNewBlocksToPlayers.size(), 
				nullptr, true, channelChunksAndBlocks);


			delete[] newBlocks;
		}

		if (tickTimer > 1.f / settings.targetTicksPerSeccond)
		{	
			tickTimer -= (1.f / settings.targetTicksPerSeccond);
			ticksPerSeccond++;

			//tick ...
			doGameTick(tickDeltaTime, entityManager, currentTimer);


			tickDeltaTime = 0;
		}

		//std::cout << deltaTime << " <- dt / 1/dt-> " << (1.f / (deltaTime)) << "\n";

		//std::cout << seccondsTimer << '\n';

		runsPerSeccond++;

		if (seccondsTimer >= 1)
		{
			seccondsTimer -= 1;
			//std::cout << "Server ticks per seccond: " << ticksPerSeccond << "\n";
			//std::cout << "Server runs per seccond: " << runsPerSeccond << "\n";
			ticksPerSeccond = 0;
			runsPerSeccond = 0;
		}
	}

	wg.clear();
	structuresManager.clear();
}

std::uint64_t getTimer()
{
	auto duration = std::chrono::steady_clock::now().time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return millis;
}

ServerSettings getServerSettingsCopy()
{
	serverSettingsMutex.lock();
	auto copy = sd.settings;
	serverSettingsMutex.unlock();
	return copy;
}

PerClientServerSettings getClientSettingCopy(CID client)
{
	PerClientServerSettings rez = {};

	serverSettingsMutex.lock();
	
	auto it = sd.settings.perClientSettings.find(client);

	if (it != sd.settings.perClientSettings.end())
	{
		rez = it->second;
	}

	serverSettingsMutex.unlock();

	return rez;
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
	StructuresManager &structureManager, BiomesManager &biomesManager,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers, bool generateGhostAndStructures,
	std::vector<StructureToGenerate> *newStructuresToAdd, bool *wasGenerated)
{
	if(wasGenerated){ *wasGenerated = false; }

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
		//create new chunk!
		if (wasGenerated) { *wasGenerated = true; }

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


		//generate big structures
		std::vector<glm::ivec2> newCreatedChunks;
		newCreatedChunks.push_back({posX, posZ});

		if(generateGhostAndStructures)
		{
			int metaChunkX = divideMetaChunk(posX);
			int metaChunkZ = divideMetaChunk(posZ);
				
			auto randValues = wg.whiteNoise->GetNoiseSet(metaChunkX, 0, metaChunkZ, 8, 1, 8);
			for (int i = 0; i < 8 * 8; i++)
			{
				randValues[i] += 1;
				randValues[i] /= 2;
			}
			int randomIndex = 0;

			auto generateTreeHouse = [&](glm::ivec2 rootChunk, glm::ivec2 inChunkPos,
				std::vector<glm::ivec3> *controllBlocks) -> bool
			{
				getOrCreateChunk(rootChunk.x + 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
				getOrCreateChunk(rootChunk.x - 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
				getOrCreateChunk(rootChunk.x + 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
				getOrCreateChunk(rootChunk.x - 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
				getOrCreateChunk(rootChunk.x + 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
				getOrCreateChunk(rootChunk.x - 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
				getOrCreateChunk(rootChunk.x, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
				getOrCreateChunk(rootChunk.x, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);

				newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y + 1});
				newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y - 1});
				newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y - 1});
				newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y + 1});
				newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y});
				newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y});
				newCreatedChunks.push_back({rootChunk.x, rootChunk.y + 1});
				newCreatedChunks.push_back({rootChunk.x, rootChunk.y - 1});



				//std::cout << "Generated root chunk: " << rootChunk.x << " " << rootChunk.y << "\n";

				auto newC = getOrCreateChunk(rootChunk.x, rootChunk.y, wg,
					structureManager, biomesManager, sendNewBlocksToPlayers, false, &newStructures);

				int y;
				for (y = CHUNK_HEIGHT - 20; y > 40; y--)
				{
					auto b = newC->unsafeGet(inChunkPos.x, y, inChunkPos.y);

					if (b.type == BlockTypes::grassBlock)
					{
						y++;
						break;
					}
					else if (b.type == BlockTypes::air || b.type == BlockTypes::grass)
					{

					}
					else
					{
						y = 0;
						break;
					}
				}

				if (y > 40)
				{
					//todo fill this
					std::unordered_set<glm::ivec2, Ivec2Hash> newCreatedChunksSet;

					StructureToGenerate s;
					s.pos.x = rootChunk.x * CHUNK_SIZE + inChunkPos.x;
					s.pos.z = rootChunk.y * CHUNK_SIZE + inChunkPos.y;
					s.pos.y = y;
					s.randomNumber1 = randValues[randomIndex++]; //todo
					s.randomNumber2 = randValues[randomIndex++]; //todo
					s.randomNumber3 = randValues[randomIndex++]; //todo
					s.randomNumber4 = randValues[randomIndex++]; //todo
					s.replaceBlocks = true;
					s.type = Structure_TreeHouse;


					if (generateStructure(s,
						structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, controllBlocks))
					{
						std::cout << "Generated a jungle tree! : " << s.pos.x << " " << s.pos.z << "\n";
						return true;
					}

				}

				return 0;
			};


			//todo random chance
			if (true)
			{
				
				glm::ivec2 offset{2};
		
				glm::ivec2 rootChunk = glm::ivec2(metaChunkX, metaChunkZ) * META_CHUNK_SIZE;
				rootChunk += offset;
		
				//todo or load it
				auto *c = getChunkOrGetNullNotUpdateTable(rootChunk.x, rootChunk.y);
				

				if (c)
				{
					//no need to generate structures, they were already generated.
				}
				else
				{
					auto tryGenerateTreeHouseChild = [&](glm::ivec2 rootChunk, std::vector<glm::ivec3> *controllBlocks) -> bool
					{
						glm::ivec2 inChunkPos = {8,8};

						auto newC = getOrCreateChunk(rootChunk.x, rootChunk.y, wg,
							structureManager, biomesManager, sendNewBlocksToPlayers, false, &newStructures);
						newCreatedChunks.push_back(rootChunk);

						if (newC->unsafeGetCachedBiome(inChunkPos.x, inChunkPos.y) == 5)
						{

							if (generateTreeHouse(rootChunk, inChunkPos, controllBlocks))
							{

								

								return true;
							}
						}

						return 0;
					};

					glm::ivec2 inChunkPos = {8,8};

					auto newC = getOrCreateChunk(rootChunk.x, rootChunk.y, wg,
						structureManager, biomesManager, sendNewBlocksToPlayers, false, &newStructures);
					newCreatedChunks.push_back(rootChunk);

					auto fillControllBlocksWithLeaves = [&](glm::ivec3 pos)
					{
						auto b = tryGetBlockIfChunkExistsNoChecks(pos);
						assert(b);

						if (isControlBlock(b->type))
						{
							b->type = BlockTypes::jungle_leaves;

							auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, 0));

							bUp->type = BlockTypes::jungle_leaves;

							auto b2 = tryGetBlockIfChunkExistsNoChecks({pos.x,pos.y,pos.z - 1});
							auto b3 = tryGetBlockIfChunkExistsNoChecks({pos.x,pos.y,pos.z + 1});
							auto b4 = tryGetBlockIfChunkExistsNoChecks({pos.x - 1,pos.y,pos.z});
							auto b5 = tryGetBlockIfChunkExistsNoChecks({pos.x + 1,pos.y,pos.z});

							if (b2 && isControlBlock(b2->type))
							{
								b2->type = BlockTypes::jungle_leaves;
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, -1));
								bUp->type = BlockTypes::jungle_leaves;

							}
							else
							if (b3 && isControlBlock(b3->type))
							{
								b3->type = BlockTypes::jungle_leaves;
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, +1));
								bUp->type = BlockTypes::jungle_leaves;
							}
							else
							if (b4 && isControlBlock(b4->type))
							{
								b4->type = BlockTypes::jungle_leaves;
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(-1, 1, 0));
								bUp->type = BlockTypes::jungle_leaves;
							}
							else
							if (b5 && isControlBlock(b5->type))
							{
								b5->type = BlockTypes::jungle_leaves;
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(+1, 1, 0));
								bUp->type = BlockTypes::jungle_leaves;
							}
						}
					};

					auto getSides = [&](std::vector<glm::ivec3> &rootControllBlocks,
						glm::ivec3 &front, glm::ivec3 &back, glm::ivec3 &left, glm::ivec3 &right)
					{
						front = rootControllBlocks[0];
						back = rootControllBlocks[0];
						left = rootControllBlocks[0];
						right = rootControllBlocks[0];

						for (auto c : rootControllBlocks)
						{
							if (c.x > front.x)
							{
								front = c;
							}
							if (c.x < back.x)
							{
								back = c;
							}
							if (c.z > right.z)
							{
								right = c;
							}
							if (c.z < left.z)
							{
								left = c;
							}
						}
					};

					auto drawBridge = [&](glm::ivec3 to, glm::ivec3 from)
					{
						glm::dvec3 pos = to;

						int steps = 100;

						glm::dvec3 delta = glm::dvec3(from - to) * (1.0/steps);

						for (int i = 0; i < steps; i++)
						{
							auto b = tryGetBlockIfChunkExistsNoChecks(pos);
							if (b)
							{
								b->type = BlockTypes::jungle_planks;
							}
							pos += delta;
						}

						auto b = tryGetBlockIfChunkExistsNoChecks(from);
						if (b)
						{
							b->type = BlockTypes::jungle_planks;
						}
					};

					auto drawDoubleBridge = [&](glm::ivec3 to, glm::ivec3 from)
					{

						glm::ivec3 neighbour1 = to;
						glm::ivec3 neighbour2 = from;

						auto tryNeighbour = [this](glm::ivec3 pos)
						{
							auto b = tryGetBlockIfChunkExistsNoChecks(pos);
							if (b && isControlBlock(b->type))
							{
								return true;
							}

							return false;
						};

						if (tryNeighbour(to + glm::ivec3(0, 0, 1))) { neighbour1 = to + glm::ivec3(0, 0, 1); }
						if (tryNeighbour(to + glm::ivec3(0, 0, -1))) { neighbour1 = to + glm::ivec3(0, 0, -1); }
						if (tryNeighbour(to + glm::ivec3(1, 0, 0))) { neighbour1 = to + glm::ivec3(1, 0, 0); }
						if (tryNeighbour(to + glm::ivec3(-1, 0, 0))) { neighbour1 = to + glm::ivec3(-1, 0, 0); }
						
						if (tryNeighbour(from + glm::ivec3(0, 0, 1))) { neighbour2 = from + glm::ivec3(0, 0, 1); }
						if (tryNeighbour(from + glm::ivec3(0, 0, -1))) { neighbour2 = from + glm::ivec3(0, 0, -1); }
						if (tryNeighbour(from + glm::ivec3(1, 0, 0))) { neighbour2 = from + glm::ivec3(1, 0, 0); }
						if (tryNeighbour(from + glm::ivec3(-1, 0, 0))) { neighbour2 = from + glm::ivec3(-1, 0, 0); }

						drawBridge(to, from);
						drawBridge(neighbour1, neighbour2);
					};

					auto biome = newC->unsafeGetCachedBiome(inChunkPos.x, inChunkPos.y);
					if (biome == 5)
					{
						std::vector<glm::ivec3> rootControllBlocks;

						if (generateTreeHouse(rootChunk, inChunkPos, &rootControllBlocks))
						{
							if (rootControllBlocks.size())
							{
								glm::ivec3 front = {};
								glm::ivec3 back = {};
								glm::ivec3 left = {};
								glm::ivec3 right = {};

								getSides(rootControllBlocks, front, back, left, right);

								{
									std::vector<glm::ivec3> childControlBlocks;
									if (tryGenerateTreeHouseChild({rootChunk.x - 2, rootChunk.y}, &childControlBlocks))
									{
										glm::ivec3 cfront = {};
										glm::ivec3 cback = {};
										glm::ivec3 cleft = {};
										glm::ivec3 cright = {};

										getSides(childControlBlocks, cfront, cback, cleft, cright);

										//front
										fillControllBlocksWithLeaves(cback);
										fillControllBlocksWithLeaves(cleft);
										fillControllBlocksWithLeaves(cright);

										drawDoubleBridge(cfront, back);
									}
									else
									{
										fillControllBlocksWithLeaves(back);
									}
								}

								{
									std::vector<glm::ivec3> childControlBlocks;
									if (tryGenerateTreeHouseChild({rootChunk.x + 2, rootChunk.y}, &childControlBlocks))
									{
										glm::ivec3 cfront = {};
										glm::ivec3 cback = {};
										glm::ivec3 cleft = {};
										glm::ivec3 cright = {};

										getSides(childControlBlocks, cfront, cback, cleft, cright);

										//back
										fillControllBlocksWithLeaves(cfront);
										fillControllBlocksWithLeaves(cleft);
										fillControllBlocksWithLeaves(cright);

										drawDoubleBridge(front, cback);
									}
									else
									{
										fillControllBlocksWithLeaves(front);
									}
								}

								{
									std::vector<glm::ivec3> childControlBlocks;
									if (tryGenerateTreeHouseChild({rootChunk.x, rootChunk.y - 2}, &childControlBlocks))
									{
										glm::ivec3 cfront = {};
										glm::ivec3 cback = {};
										glm::ivec3 cleft = {};
										glm::ivec3 cright = {};

										getSides(childControlBlocks, cfront, cback, cleft, cright);

										//right
										fillControllBlocksWithLeaves(cfront);
										fillControllBlocksWithLeaves(cleft);
										fillControllBlocksWithLeaves(cback);

										drawDoubleBridge(left, cright);
									}
									else
									{
										fillControllBlocksWithLeaves(left);
									}
								}

								{
									std::vector<glm::ivec3> childControlBlocks;
									if (tryGenerateTreeHouseChild({rootChunk.x, rootChunk.y + 2}, &childControlBlocks))
									{
										glm::ivec3 cfront = {};
										glm::ivec3 cback = {};
										glm::ivec3 cleft = {};
										glm::ivec3 cright = {};

										getSides(childControlBlocks, cfront, cback, cleft, cright);

										//left
										fillControllBlocksWithLeaves(cfront);
										fillControllBlocksWithLeaves(cright);
										fillControllBlocksWithLeaves(cback);

										drawDoubleBridge(cleft, right);
									}
									else
									{
										fillControllBlocksWithLeaves(right);
									}
								}
							}

							

							std::cout << "Generated village" << rootChunk.x * CHUNK_SIZE << " " << rootChunk.y * CHUNK_SIZE << "\n";
						}


					}
					else if(biome == 2 || biome == 1)
					{
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);

						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y - 1});
						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y - 1});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y});
						newCreatedChunks.push_back({rootChunk.x, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x, rootChunk.y - 1});

						int y;
						for (y = CHUNK_HEIGHT - 20; y > 40; y--)
						{
							auto b = newC->unsafeGet(inChunkPos.x, y, inChunkPos.y);

							if (b.type == BlockTypes::sand || b.type == BlockTypes::sand_stone)
							{
								y-= 6 * randValues[randomIndex++];
								break;
							}
							else if (b.type == BlockTypes::air || b.type == BlockTypes::grass)
							{

							}
							else
							{
								y = 0;
								break;
							}
						}

						if (y > 40)
						{
							//todo fill this
							std::unordered_set<glm::ivec2, Ivec2Hash> newCreatedChunksSet;

							StructureToGenerate s;
							s.pos.x = rootChunk.x * CHUNK_SIZE + inChunkPos.x;
							s.pos.z = rootChunk.y * CHUNK_SIZE + inChunkPos.y;
							s.pos.y = y;
							s.randomNumber1 = randValues[randomIndex++];
							s.randomNumber2 = randValues[randomIndex++];
							s.randomNumber3 = randValues[randomIndex++];
							s.randomNumber4 = randValues[randomIndex++];
							s.replaceBlocks = true;
							s.type = Structure_Pyramid;

							if (generateStructure(s,
								structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, 0))
							{
								std::cout << "Generated a pyrmid: " << s.pos.x << " " << s.pos.z << "\n";
							}


						}
						
					}
					else if (biome == 8 || biome == 9)
					{
						
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);
						getOrCreateChunk(rootChunk.x, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures);

						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y - 1});
						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y - 1});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y});
						newCreatedChunks.push_back({rootChunk.x, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x, rootChunk.y - 1});

						int y;
						for (y = CHUNK_HEIGHT - 20; y > 40; y--)
						{
							auto b = newC->unsafeGet(inChunkPos.x, y, inChunkPos.y);

							if (b.type == BlockTypes::snow_block)
							{
								break;
							}
							else if (b.type == BlockTypes::air || b.type == BlockTypes::grass)
							{

							}
							else
							{
								y = 0;
								break;
							}
						}

						if (y > 40)
						{
							//todo fill this
							std::unordered_set<glm::ivec2, Ivec2Hash> newCreatedChunksSet;

							StructureToGenerate s;
							s.pos.x = rootChunk.x * CHUNK_SIZE + inChunkPos.x;
							s.pos.z = rootChunk.y * CHUNK_SIZE + inChunkPos.y;
							s.pos.y = y;
							s.randomNumber1 = randValues[randomIndex++];
							s.randomNumber2 = randValues[randomIndex++];
							s.randomNumber3 = randValues[randomIndex++];
							s.randomNumber4 = randValues[randomIndex++];
							s.replaceBlocks = true;
							s.type = Structure_Igloo;

							if (generateStructure(s,
								structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, 0))
							{
								std::cout << "Generated an igloo: " << s.pos.x << " " << s.pos.z << "\n";
							}


						}
						
					}
				}
				
			}

			FastNoiseSIMD::FreeNoiseSet(randValues);

		}

	
		if (generateGhostAndStructures)
		{
			std::unordered_set<glm::ivec2, Ivec2Hash> newCreatedChunksSet;

			//ghost blocks
			for (auto &cp : newCreatedChunks)
			{
				auto c = getChunkOrGetNullNotUpdateTable(cp.x, cp.y);

				assert(c);

				if (c)
				{
					placeGhostBlocksForChunk(c->x, c->z, *c);
					newCreatedChunksSet.insert({c->x, c->z});
				}
				
			}

			//trees
			for (auto &s : newStructures)
			{
				generateStructure(s, structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, 0);
			}

		}
		else
		{
			if (newStructuresToAdd)
			{
				for (auto &s : newStructures)
				{
					newStructuresToAdd->push_back(s);
				}
			}
		}
		

		//auto elapsed = std::chrono::high_resolution_clock::now() - startTimer;

		//long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
		//	elapsed).count();

		//std::cout << "Chunk timer ms: " << milliseconds << "\n";

		return rez;
	}
}

Block *ChunkPriorityCache::tryGetBlockIfChunkExistsNoChecks(glm::ivec3 pos)
{
	auto c = getChunkOrGetNullNotUpdateTable(divideChunk(pos.x), divideChunk(pos.z));

	if (c)
	{
		return &c->unsafeGet(modBlockToChunk(pos.x), pos.y, modBlockToChunk(pos.z));
	}

	return nullptr;
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

bool ChunkPriorityCache::generateStructure(StructureToGenerate s, StructuresManager &structureManager,
	std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	std::vector<glm::ivec3> *controlBlocks)
{
	auto chooseRandomElement = [](float randVal, int elementCount)
	{
		return int(floor(randVal * elementCount));
	};


	if (s.type == Structure_Tree)
	{

		auto tree = structureManager.trees
			[chooseRandomElement(s.randomNumber1, structureManager.trees.size())];

		return generateStructure(s, tree, 
			chooseRandomElement(s.randomNumber2, 4), newCreatedChunks, sendNewBlocksToPlayers, 
			controlBlocks);
	}
	else 
	if (s.type == Structure_JungleTree)
	{

		auto tree = structureManager.jungleTrees
			[chooseRandomElement(s.randomNumber1, structureManager.jungleTrees.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedChunks, sendNewBlocksToPlayers, controlBlocks);
	}if (s.type == Structure_PalmTree)
	{

		auto tree = structureManager.palmTrees
			[chooseRandomElement(s.randomNumber1, structureManager.palmTrees.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedChunks,
			sendNewBlocksToPlayers, controlBlocks);

	}if (s.type == Structure_TreeHouse)
	{

		auto tree = structureManager.treeHouses
			[chooseRandomElement(s.randomNumber1, structureManager.treeHouses.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedChunks,
			sendNewBlocksToPlayers, controlBlocks);

	}if (s.type == Structure_Pyramid)
	{

		auto tree = structureManager.smallPyramids
			[chooseRandomElement(s.randomNumber1, structureManager.smallPyramids.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedChunks,
			sendNewBlocksToPlayers, controlBlocks);

	}if (s.type == Structure_BirchTree)
	{
		auto tree = structureManager.birchTrees
			[chooseRandomElement(s.randomNumber1, structureManager.birchTrees.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), 
			newCreatedChunks, sendNewBlocksToPlayers, controlBlocks);

	}if (s.type == Structure_Igloo)
	{
		auto tree = structureManager.igloos
			[chooseRandomElement(s.randomNumber1, structureManager.igloos.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
			newCreatedChunks, sendNewBlocksToPlayers, controlBlocks);

	}
	if (s.type == Structure_Spruce)
	{
		auto tree = structureManager.spruceTrees
			[chooseRandomElement(s.randomNumber1, structureManager.spruceTrees.size())];

		if (s.randomNumber3 > 0.5)
		{
			return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
				newCreatedChunks, sendNewBlocksToPlayers, controlBlocks, true,
				BlockTypes::spruce_leaves, BlockTypes::spruce_leaves_red);
		}
		else
		{
			return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
				newCreatedChunks, sendNewBlocksToPlayers, controlBlocks);
		}
		
	}
	

	return 0;
}

bool ChunkPriorityCache::generateStructure(StructureToGenerate s, StructureData *structure, int rotation,
	std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	std::vector<glm::ivec3> *controlBlocks, bool replace, BlockType from, BlockType to)
{
	auto size = structure->size;

	auto replaceB = [&](BlockType &b)
	{
		if (replace)
		{
			if (b == from)
			{
				b = to;
			}
		}
	};

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

				//todo this will be replaced with also a check if the
				//chunk was created in the past
				auto c = getChunkOrGetNullNotUpdateTable(chunkX, chunkZ);

				int inChunkX = modBlockToChunk(x);
				int inChunkZ = modBlockToChunk(z);
				
				constexpr bool replaceAnything = 0;

				bool sendDataToPlayers = 0;
				//auto it = newCreatedChunks.find({chunkX, chunkZ});
				//if (it != newCreatedChunks.end())
				//{
				//	//todo server should know what chunks can the player see to corectly send him the data
				//	//even if the chunk is not loaded in the server side
				//	//std::cout << "Updating chunk info to player\n";
				//	sendDataToPlayers = true;
				//
				//}
				sendDataToPlayers = true;

			
				if (c)
				{
			
					for (int y = startPos.y; y < endPos.y; y++)
					{

						auto &b = c->unsafeGet(inChunkX, y, inChunkZ);

						if (b.type == BlockTypes::air || replaceAnything)
						{
							auto newB = structure->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z,
								rotation);

							replaceB(newB);

							if (newB != BlockTypes::air)
							{
								b.type = newB;

								if (sendDataToPlayers)
								{
									SendBlocksBack sendB;
									sendB.pos = {x,y,z};
									sendB.block = newB;
									sendNewBlocksToPlayers.push_back(sendB);
								}

								if (controlBlocks)
								{
									if (isControlBlock(newB))
									{
										controlBlocks->push_back({x,y,z});
									}
								}
							}

						}
					}
				}
				else
				{

					auto it = ghostBlocks.find({chunkX, chunkZ});


					if(it == ghostBlocks.end())
					{

						std::unordered_map<BlockInChunkPos, GhostBlock, BlockInChunkHash> rez;

						for (int y = startPos.y; y < endPos.y; y++)
						{
							auto b = structure->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z,
								rotation);
							
							replaceB(b);

							GhostBlock ghostBlock;
							ghostBlock.type = b;
							ghostBlock.replaceAnything = replaceAnything;

							rez[{inChunkX, y, inChunkZ}] = ghostBlock; //todo either ghost either send

							if (sendDataToPlayers)
							{
								SendBlocksBack sendB;
								sendB.pos = {x,y,z};
								sendB.block = b;
								sendNewBlocksToPlayers.push_back(sendB);
							}

							if (controlBlocks)
							{
								if (isControlBlock(b))
								{
									controlBlocks->push_back({x,y,z});
								}
							}
						}

						ghostBlocks[{chunkX, chunkZ}] = rez;

					}
					else
					{
						for (int y = startPos.y; y < endPos.y; y++)
						{
							auto b = structure->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z,
								rotation);

							replaceB(b);

							GhostBlock ghostBlock;
							ghostBlock.type = b;
							ghostBlock.replaceAnything = replaceAnything;

							auto blockIt = it->second.find({inChunkX, y, inChunkZ});

							if (blockIt == it->second.end())
							{
								it->second[{inChunkX, y, inChunkZ}] = ghostBlock;

								if (sendDataToPlayers)
								{
									SendBlocksBack sendB;
									sendB.pos = {x,y,z};
									sendB.block = b;
									sendNewBlocksToPlayers.push_back(sendB);
								}

								if (controlBlocks)
								{
									if (isControlBlock(b))
									{
										controlBlocks->push_back({x,y,z});
									}
								}
							}
							else
							{
								if (replaceAnything)
								{
									blockIt->second = ghostBlock;

									if (sendDataToPlayers)
									{
										SendBlocksBack sendB;
										sendB.pos = {x,y,z};
										sendB.block = b;
										sendNewBlocksToPlayers.push_back(sendB);
									}

									if (controlBlocks)
									{
										if (isControlBlock(b))
										{
											controlBlocks->push_back({x,y,z});
										}
									}
								}
							}


						}
					}
				

				}

			}

		return true;
	}

	return 0;
}

void ChunkPriorityCache::placeGhostBlocksForChunk(int posX, int posZ, ChunkData &c)
{
	auto iter = ghostBlocks.find({posX, posZ});

	if (iter != ghostBlocks.end())
	{

		for (auto &b : iter->second)
		{
			//the pos is in chunk space
			auto pos = b.first;

			auto &block = c.unsafeGet(pos.x, pos.y, pos.z);

			if (b.second.replaceAnything || block.type == BlockTypes::air)
			{
				block.type = b.second.type;
			}
		}

		ghostBlocks.erase(iter);
	}
}

void doGameTick(float deltaTime, ServerEntityManager &entityManager, std::uint64_t currentTimer)
{

	auto chunkGetter = [](glm::ivec2 pos) -> ChunkData *
	{
		auto c = sd.chunkCache.getChunkOrGetNullNotUpdateTable(pos.x, pos.y);
		if (c)
		{
			return c;
		}
		else
		{
			return nullptr;
		}
	};

	//todo when a player joins, send him all of the entities

	//todo server constants
	constexpr float SERVER_UPDATE_ENTITIES_TIMER = 1.f;

	sd.updateEntititiesTimer -= deltaTime;

	if (sd.updateEntititiesTimer < 0)
	{
		sd.updateEntititiesTimer = SERVER_UPDATE_ENTITIES_TIMER;
	}


#pragma region physics update

	for (auto &e : entityManager.droppedItems)
	{

		{
			float time = deltaTime + e.second.restantTime;
			
			if (time > 0)
			{
				updateDroppedItem(e.second.item, time, chunkGetter);
			}
			e.second.restantTime = 0;
		}

		//send item data
		{
			Packet packet;
			//packet.cid = i.cid;
			packet.header = headerClientRecieveDroppedItemUpdate;

			Packet_RecieveDroppedItemUpdate packetData;
			packetData.eid = e.first;
			packetData.entity = e.second.item;
			packetData.timer = currentTimer;

			//todo sepparate entity update method so we can reuse,
			//+ take into accound distance.

			broadCast(packet, &packetData, sizeof(packetData),
				nullptr, false, channelEntityPositions);

		}

	}


#pragma endregion





}