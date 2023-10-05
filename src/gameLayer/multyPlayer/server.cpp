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
			std::vector<StructureToGenerate> *newStructuresToAdd);

	Block *tryGetBlockIfChunkExistsNoChecks(glm::ivec3 pos);

	//uses chunk coorodonates
	ChunkData *getChunkOrGetNullNotUpdateTable(int posX, int posZ);

	bool generateStructure(StructureToGenerate s, StructuresManager &structureManager,
		std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
		std::vector<glm::ivec3> *controlBlocks);
	
	bool generateStructure(StructureToGenerate s, StructureData *structure, int rotation,
		std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
		std::vector<glm::ivec3> *controlBlocks
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

		std::vector<ChunkPriorityCache::SendBlocksBack> sendNewBlocksToPlayers;

		int count = sd.waitingTasks.size();
		for (int taskIndex = 0; taskIndex < std::min(count, 3); taskIndex++)
		{
			auto &i = sd.waitingTasks.front();

			if (i.t.type == Task::generateChunk)
			{
				auto rez = sd.chunkCache.getOrCreateChunk(i.t.pos.x, i.t.pos.z, wg, structuresManager, biomesManager,
					sendNewBlocksToPlayers, true, nullptr);
				
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
					,biomesManager, sendNewBlocksToPlayers, true, nullptr);
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
						packet.header = headerPlaceBlocks;

						Packet_PlaceBlocks packetData;
						packetData.blockPos = i.t.pos;
						packetData.blockType = i.t.blockType;

						broadCast(packet, &packetData, sizeof(Packet_PlaceBlocks), client.peer, true, 0);
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
				//broadCast(packet, &packetData, sizeof(Packet_PlaceBlock), nullptr, true, 0);
					
				newBlocks[i].blockPos = b.pos;
				newBlocks[i].blockType = b.block;

				i++;
			}

			broadCast(packet, newBlocks, 
				sizeof(Packet_PlaceBlocks) *sendNewBlocksToPlayers.size(), nullptr, true, 0);


			delete[] newBlocks;
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
	StructuresManager &structureManager, BiomesManager &biomesManager,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers, bool generateGhostAndStructures,
	std::vector<StructureToGenerate> *newStructuresToAdd)
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
		//create new chunk!

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
					s.randomNumber1 = 0.5; //todo
					s.randomNumber2 = 0.5; //todo
					s.randomNumber3 = 0.5; //todo
					s.randomNumber4 = 0.5; //todo
					s.replaceBlocks = true;
					s.type = Structure_TreeHouse;


					if (generateStructure(s,
						structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, controllBlocks))
					{
						std::cout << "Generated a jungle tree! : " << s.pos.x << " " << s.pos.y << "\n";
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
					auto tryGenerateTreeHouseChild = [&](glm::ivec2 rootChunk) -> bool
					{
						glm::ivec2 inChunkPos = {8,8};

						auto newC = getOrCreateChunk(rootChunk.x, rootChunk.y, wg,
							structureManager, biomesManager, sendNewBlocksToPlayers, false, &newStructures);
						newCreatedChunks.push_back(rootChunk);

						if (newC->unsafeGetCachedBiome(inChunkPos.x, inChunkPos.y) == 5)
						{
							std::vector<glm::ivec3> rootControllBlocks;

							if (generateTreeHouse(rootChunk, inChunkPos, &rootControllBlocks))
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

					if (newC->unsafeGetCachedBiome(inChunkPos.x, inChunkPos.y) == 5)
					{
						std::vector<glm::ivec3> rootControllBlocks;

						if (generateTreeHouse(rootChunk, inChunkPos, &rootControllBlocks))
						{
							if (rootControllBlocks.size())
							{
								glm::ivec3 front = rootControllBlocks[0];
								glm::ivec3 back = rootControllBlocks[0];
								glm::ivec3 left = rootControllBlocks[0];
								glm::ivec3 right = rootControllBlocks[0];

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

								if (tryGenerateTreeHouseChild({rootChunk.x - 2, rootChunk.y}))
								{
									
								}
								else
								{
									fillControllBlocksWithLeaves(back);
								}

								if (tryGenerateTreeHouseChild({rootChunk.x + 2, rootChunk.y}))
								{

								}
								else
								{
									fillControllBlocksWithLeaves(front);
								}

								if (tryGenerateTreeHouseChild({rootChunk.x, rootChunk.y - 2}))
								{

								}
								else
								{
									fillControllBlocksWithLeaves(left);
								}

								if (tryGenerateTreeHouseChild({rootChunk.x, rootChunk.y + 2}))
								{

								}
								else
								{
									fillControllBlocksWithLeaves(right);
								}
							}

							

							std::cout << "Generated village\n";
						}


					}

				}
				
			}

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

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedChunks, sendNewBlocksToPlayers, controlBlocks);
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

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedChunks, sendNewBlocksToPlayers, controlBlocks);
	}if (s.type == Structure_TreeHouse)
	{

		auto tree = structureManager.treeHouses
			[chooseRandomElement(s.randomNumber1, structureManager.treeHouses.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedChunks, sendNewBlocksToPlayers, controlBlocks);
	}

	return 0;
}

bool ChunkPriorityCache::generateStructure(StructureToGenerate s, StructureData *structure, int rotation, 
	std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	std::vector<glm::ivec3> *controlBlocks)
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
