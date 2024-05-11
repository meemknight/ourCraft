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
#include "multyPlayer/chunkSaver.h"
#include "multyPlayer/serverChunkStorer.h"
#include <multyPlayer/tick.h>
#include <multyPlayer/splitUpdatesLogic.h>



static std::atomic<bool> serverRunning = false;

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

		return 1;
	}
	else
	{
		return 0;
	}
}


void updateLoadedChunks(
	WorldGenerator &wg,
	StructuresManager &structureManager,
	BiomesManager &biomesManager,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	WorldSaver &worldSaver);


struct ServerData
{

	//todo probably move this just locally
	ServerChunkStorer chunkCache = {};
	std::vector<ServerTask> waitingTasks = {};
	ENetHost *server = nullptr;
	ServerSettings settings = {};

	float tickTimer = 0;
	float tickDeltaTime = 0;
	int ticksPerSeccond = 0;
	int runsPerSeccond = 0;
	float seccondsTimer = 0;

	float saveEntitiesTimer = 5;


}sd;

int outTicksPerSeccond = 0;

int getServerTicksPerSeccond()
{
	return outTicksPerSeccond;
}

void clearSD(WorldSaver &worldSaver)
{
	worldSaver.saveEntityId(getCurrentEntityId());
	sd.chunkCache.saveAllChunks(worldSaver);
	sd.chunkCache.cleanup();
	closeThreadPool();
}

int getChunkCapacity()
{
	return sd.chunkCache.savedChunks.size();
}

void closeServer()
{
	//todo cleanup stuff
	if (serverRunning)
	{

		closeEnetListener();


		//close loop
		serverRunning = false;

		//then signal the barier from the task waiting to unlock the mutex

		//then wait for the server to close
		//serverThread.join();

		enet_host_destroy(sd.server);

		//todo clear othher stuff
		sd = {};
	}

	//serverSettingsMutex.unlock();
}


//Note: it is a problem that the block validation and the item validation are on sepparate threads.
bool computeRevisionStuff(Client &client, bool allowed, 
	const EventId &eventId, std::uint64_t *oldid, std::uint64_t *newid)
{

	permaAssertComment((oldid == 0 && newid == 0) || (oldid != 0 && newid != 0),
		"both ids should be supplied or none");


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
		if (oldid && newid)
		{
			Packet packet;
			packet.header = headerValidateEventAndChangeID;

			Packet_ValidateEventAndChangeId packetData;
			packetData.eventId = eventId;
			packetData.oldId = *oldid;
			packetData.newId = *newid;

			sendPacket(client.peer, packet,
				(char *)&packetData, sizeof(Packet_ValidateEventAndChangeId),
				true, channelChunksAndBlocks);
		}
		else
		{
			Packet packet;
			packet.header = headerValidateEvent;

			Packet_ValidateEvent packetData;
			packetData.eventId = eventId;

			sendPacket(client.peer, packet,
				(char *)&packetData, sizeof(Packet_ValidateEvent),
				true, channelChunksAndBlocks);
		}
		
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


bool spawnZombie(
	ServerChunkStorer &chunkManager,
	Zombie zombie, std::uint64_t newId)
{

	//todo also send packets

	auto chunkPos = determineChunkThatIsEntityIn(zombie.position);

	auto c = chunkManager.getChunkOrGetNull(chunkPos.x, chunkPos.y);

	if (c)
	{

		ZombieServer serverZombie = {};
		serverZombie.entity = zombie;

		c->entityData.zombies.insert({newId, serverZombie});

	}
	else
	{
		return 0;
	}

	return 1;
}


bool spawnPig(
	ServerChunkStorer &chunkManager,
	Pig pig, WorldSaver &worldSaver,
	std::minstd_rand &rng)
{
	//todo also send packets
	//todo generic spawn for any entity

	auto chunkPos = determineChunkThatIsEntityIn(pig.position);
	auto c = chunkManager.getChunkOrGetNull(chunkPos.x, chunkPos.y);

	if (c)
	{
		PigServer e = {};
		e.entity = pig;
		e.configureSpawnSettings(rng);

		c->entityData.pigs.insert({getEntityIdAndIncrement(worldSaver), e});
	}
	else
	{
		return 0;
	}

	return 1;
}



ServerSettings getServerSettingsCopy()
{
	return sd.settings;
}

void setServerSettings(ServerSettings settings)
{
	for (auto &s : sd.settings.perClientSettings)
	{
		auto it = settings.perClientSettings.find(s.first);
		if (it != settings.perClientSettings.end())
		{
			s.second = it->second;
		}
	}
}

void serverWorkerUpdate(
	WorldGenerator &wg,
	StructuresManager &structuresManager,
	BiomesManager &biomesManager,
	WorldSaver &worldSaver,
	std::vector<ServerTask> &serverTask,
	float deltaTime
	)
{

#pragma region timers stuff
	auto currentTimer = getTimer();
	sd.tickTimer += deltaTime;
	sd.seccondsTimer += deltaTime;
	sd.tickDeltaTime += deltaTime;
	sd.saveEntitiesTimer -= deltaTime;
#pragma endregion

	auto &settings = sd.settings;

	for (auto i : serverTask)
	{
		sd.waitingTasks.push_back(i);
	}
	serverTask.clear();

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

	std::vector<SendBlocksBack> sendNewBlocksToPlayers;

	int chunksGenerated = 0;
	int chunksLoaded = 0;

	int count = sd.waitingTasks.size();
	for (int taskIndex = 0; taskIndex < std::min(count, 25); taskIndex++)
	{
		auto &i = sd.waitingTasks.front();

		if (i.t.type == Task::generateChunk)
		{
			auto client = getClient(i.cid); //todo this could fail when players leave so return pointer and check
			bool wasGenerated = 0;

			if (checkIfPlayerShouldGetChunk(client.positionForChunkGeneration,
				{i.t.pos.x, i.t.pos.z}, client.playerData.entity.chunkDistance))
			{
				PL::Profiler profiler;

				profiler.start();
				auto rez = sd.chunkCache.getOrCreateChunk(i.t.pos.x, i.t.pos.z, wg, structuresManager, biomesManager,
					sendNewBlocksToPlayers, true, nullptr, worldSaver, &wasGenerated);
				profiler.end();

				chunksLoaded++;

				//if (wasGenerated)
				//{
				//	std::cout << "Generated ChunK: " << profiler.rezult.timeSeconds * 1000.f << "ms  per 100: " <<
				//		profiler.rezult.timeSeconds * 100'000.f << "\n";
				//}

				Packet packet;
				packet.cid = i.cid; //todo is this cid here needed? should I just put 0?
				packet.header = headerRecieveChunk;


				//if you have modified Packet_RecieveChunk make sure you didn't break this!
				static_assert(sizeof(Packet_RecieveChunk) == sizeof(ChunkData));


				{
					auto client = getClientNotLocked(i.cid);
					if (client)
					{
						sendPacketAndCompress(client->peer, packet, (char *)rez,
							sizeof(Packet_RecieveChunk), true, channelChunksAndBlocks);
					}
				}

			}
			else
			{
				std::cout << "Chunk rejected because player too far: " <<
					i.t.pos.x << " " << i.t.pos.z << " dist: " << client.playerData.entity.chunkDistance << "\n";
			}

			if (wasGenerated) { chunksGenerated++; }
		}
		else
			if (i.t.type == Task::placeBlock)
			{
				bool wasGenerated = 0;
				//std::cout << "server recieved place block\n";
				//auto chunk = sd.chunkCache.getOrCreateChunk(i.t.pos.x / 16, i.t.pos.z / 16);
				auto chunk = sd.chunkCache.getOrCreateChunk(divideChunk(i.t.pos.x), divideChunk(i.t.pos.z), wg, structuresManager
					, biomesManager, sendNewBlocksToPlayers, true, nullptr, worldSaver, &wasGenerated);
				int convertedX = modBlockToChunk(i.t.pos.x);
				int convertedZ = modBlockToChunk(i.t.pos.z);

				//todo check if place is legal
				bool noNeedToNotifyUndo = 0;

				auto client = getClientNotLocked(i.cid);

				if (client)
				{

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


					auto b = chunk->chunk.safeGet(convertedX, i.t.pos.y, convertedZ);

					if (!b)
					{
						legal = false;
					}

					legal = computeRevisionStuff(*client, legal, i.t.eventId);

					if (legal)
					{
						b->type = i.t.blockType;
						chunk->otherData.dirty = true;

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


			}
			else if (i.t.type == Task::droppedItemEntity)
			{

				auto client = getClientNotLocked(i.cid);

				//todo some logic
				//todo add items here into the server's storage
				if (client)
				{

					auto serverAllows = settings.perClientSettings[i.cid].validateStuff;
					
					if (i.t.entityId >= RESERVED_CLIENTS_ID)
					{
						serverAllows = false; 
					}

					auto newId = getEntityIdAndIncrement(worldSaver);

					if (computeRevisionStuff(*client, true && serverAllows, i.t.eventId,
						&i.t.entityId, &newId))
					{

						DroppedItemServer newEntity = {};
						newEntity.entity.count = i.t.blockCount;
						newEntity.entity.position = i.t.doublePos;
						newEntity.entity.lastPosition = i.t.doublePos;
						newEntity.entity.type = i.t.blockType;
						newEntity.entity.forces = i.t.motionState;
						//newEntity.restantTime = computeRestantTimer(currentTimer, i.t.timer);
						newEntity.restantTime = computeRestantTimer(i.t.timer, currentTimer);


						auto chunkPosition = determineChunkThatIsEntityIn(i.t.doublePos);

						SavedChunk *chunk = sd.chunkCache.getOrCreateChunk(chunkPosition.x,
							chunkPosition.y, wg, structuresManager, biomesManager,
							sendNewBlocksToPlayers, true, nullptr, worldSaver
						);

						if (chunk)
						{
							chunk->entityData.droppedItems.insert({newId, newEntity});
						}

						//std::cout << "restant: " << newEntity.restantTime << "\n";


					}

				}
				else
				{

				}


			}
			else if (i.t.type == Task::clientMovedItem)
			{
				
				auto client = getClientNotLocked(i.cid);

				if (client)
				{

					Item *from = client->playerData.inventory.getItemFromIndex(i.t.from);
					Item *to = client->playerData.inventory.getItemFromIndex(i.t.to);

					//todo they should always be sanitized so we should check during task creation if they are
					if (from && to)
					{

						//todo this can be abstracted
						if (from->type != i.t.itemType
							|| (i.t.blockCount > from->counter)
							)
						{
							//this is a desync, resend inventory.
							sendPlayerInventory(*client, channelChunksAndBlocks);
						}
						else
						{

							if (to->type == 0)
							{
								*to = *from;
								to->counter = i.t.blockCount;
								from->counter -= i.t.blockCount;

								if (!from->counter) { *from = {}; }
							}
							else if(to->type == from->type)
							{

								if (to->counter >= 64)
								{
									sendPlayerInventory(*client, channelChunksAndBlocks);
								}

								int total = (int)to->counter + (int)i.t.blockCount;
								if (total <= 64)
								{
									to->counter += i.t.blockCount;
									from->counter -= i.t.blockCount;

									if (!from->counter) { *from = {}; }
								}
								else
								{
									//this is a desync, resend inventory.
									sendPlayerInventory(*client, channelChunksAndBlocks);
								}

							}
							else
							{
								//this is a desync, resend inventory.
								sendPlayerInventory(*client, channelChunksAndBlocks);
							}

						}

					}

				};

			}
			else if (i.t.type == Task::clientOverwriteItem)
			{

				//todo if creative.

				auto client = getClientNotLocked(i.cid);

				if (client)
				{
					Item *to = client->playerData.inventory.getItemFromIndex(i.t.to);

					if (to)
					{
						*to = {};
						to->counter = i.t.blockCount;
						to->type = i.t.itemType;
					}
				}

			}
			else if (i.t.type == Task::clientSwapItems)
			{

				auto client = getClientNotLocked(i.cid);

				if (client)
				{

					Item *from = client->playerData.inventory.getItemFromIndex(i.t.from);
					Item *to = client->playerData.inventory.getItemFromIndex(i.t.to);

					if (from && to)
					{
						Item copy;
						copy = std::move(*from);
						*from = std::move(*to);
						*to = std::move(copy);
					}

				}

			}


		sd.waitingTasks.erase(sd.waitingTasks.begin());

		//we generate only one chunk per loop
		if (chunksGenerated >= 1 || chunksLoaded >= 5)
		{
			break;
		}
	}



	//todo check if there are too many loaded chunks and unload them before processing
	//generate chunk

#pragma region gameplay tick

	static std::minstd_rand rng(std::random_device{}());


	if (sd.tickTimer > 1.f / targetTicksPerSeccond)
	{
		sd.tickTimer -= (1.f / targetTicksPerSeccond);
		sd.ticksPerSeccond++;


		{

			if (settings.perClientSettings.begin()->second.spawnZombie)
			{
				settings.perClientSettings.begin()->second.spawnZombie = false;


				auto c = getAllClients();

				Zombie z;
				glm::dvec3 position = c.begin()->second.playerData.entity.position;
				z.position = position;
				z.lastPosition = position;
				spawnZombie(sd.chunkCache, z, getEntityIdAndIncrement(worldSaver));
			}

			if (settings.perClientSettings.begin()->second.spawnPig)
			{
				settings.perClientSettings.begin()->second.spawnPig = false;
				auto c = getAllClients();

				Pig p;
				glm::dvec3 position = c.begin()->second.playerData.entity.position;
				p.position = position;
				p.lastPosition = position;
				spawnPig(sd.chunkCache, p, worldSaver, rng);
			}

			if (settings.perClientSettings.begin()->second.resendInventory)
			{
				settings.perClientSettings.begin()->second.resendInventory = false;
				auto c = getAllClients();

				sendPlayerInventory(c.begin()->second, channelChunksAndBlocks);
			}


		}


		updateLoadedChunks(wg, structuresManager, biomesManager, sendNewBlocksToPlayers,
			worldSaver);
	
		sd.chunkCache.unloadChunksThatNeedUnloading(worldSaver, 10);

		//todo error and warning logs for server.


		//todo get all clients should probably dissapear.
		auto c = getAllClients();
		splitUpdatesLogic(sd.tickDeltaTime, currentTimer, sd.chunkCache, rng(), c, worldSaver);

		sd.tickDeltaTime = 0;
	}

	//std::cout << deltaTime << " <- dt / 1/dt-> " << (1.f / (deltaTime)) << "\n";

	//std::cout << seccondsTimer << '\n';

	sd.runsPerSeccond++;

	if (sd.seccondsTimer >= 1)
	{
		sd.seccondsTimer -= 1;
		//std::cout << "Server ticks per seccond: " << ticksPerSeccond << "\n";
		//std::cout << "Server runs per seccond: " << runsPerSeccond << "\n";
		outTicksPerSeccond = sd.ticksPerSeccond;
		sd.ticksPerSeccond = 0;
		sd.runsPerSeccond = 0;
	}

#pragma endregion

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
			sizeof(Packet_PlaceBlocks) * sendNewBlocksToPlayers.size(),
			nullptr, true, channelChunksAndBlocks);


		delete[] newBlocks;
	}



	//save one chunk on disk
	sd.chunkCache.saveNextChunk(worldSaver);

	if (sd.saveEntitiesTimer <= 0)
	{
		sd.saveEntitiesTimer = 5;

		for (auto &c : sd.chunkCache.savedChunks)
		{
			c.second->otherData.dirtyEntity = true;
		}

	}

}



std::uint64_t getTimer()
{
	static const auto start_time = std::chrono::steady_clock::now();
	auto now = std::chrono::steady_clock::now();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
	return millis;
}


void addCidToServerSettings(std::uint64_t cid)
{
	sd.settings.perClientSettings.insert({cid, {}});
}

void removeCidFromServerSettings(std::uint64_t cid)
{
	sd.settings.perClientSettings.erase(cid);
}




//adds loaded chunks.
void updateLoadedChunks(
	WorldGenerator &wg,
	StructuresManager &structureManager,
	BiomesManager &biomesManager,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	WorldSaver &worldSaver)
{

	for (auto &c : sd.chunkCache.savedChunks)
	{
		c.second->otherData.shouldUnload = true;
	}

	auto clientsCopy = getAllClients();

	for (auto &c : clientsCopy)
	{
		
		glm::ivec2 pos(divideChunk(c.second.playerData.entity.position.x),
			divideChunk(c.second.playerData.entity.position.z));
		
		int distance = (c.second.playerData.entity.chunkDistance/2) + 1;

		for (int i = -distance; i <= distance; i++)
			for (int j = -distance; j <= distance; j++)
			{
				glm::vec2 vect(i, j);

				float dist = std::sqrt(glm::dot(vect, vect));

				if (dist <= distance)
				{
					auto finalPos = pos + glm::ivec2(i, j);
					auto c = sd.chunkCache.getOrCreateChunk(finalPos.x, finalPos.y,
						wg, structureManager, biomesManager, sendNewBlocksToPlayers, true,
						nullptr, worldSaver, nullptr
						);

					if (c)
					{
						c->otherData.shouldUnload = false;
					}
				}
			}
	}
	


}

