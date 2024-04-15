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

std::mutex serverSettingsMutex;


glm::ivec2 determineChunkThatIsEntityIn(glm::dvec3 position)
{
	return {divideChunk(position.x), divideChunk(position.z)};
}

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

}sd;

void clearSD(WorldSaver &worldSaver)
{
	sd.chunkCache.saveAllChunks(worldSaver);
	sd.chunkCache.cleanup();
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
		signalWaitingFromServer();

		//then wait for the server to close
		//serverThread.join();

		enet_host_destroy(sd.server);

		//todo clear othher stuff
		sd = {};
	}

	//serverSettingsMutex.unlock();
}

void doGameTick(float deltaTime,
	std::uint64_t currentTimer,
	WorldGenerator &wg, StructuresManager &structuresManager,
	BiomesManager &biomeManager,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	WorldSaver &worldSaver
);

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



void initServerWorker()
{

}


void serverWorkerUpdate(
	WorldGenerator &wg,
	StructuresManager &structuresManager,
	BiomesManager &biomesManager,
	WorldSaver &worldSaver,
	float deltaTime
	)
{

	auto settings = getServerSettingsCopy();

#pragma region timers stuff
	auto currentTimer = getTimer();
	sd.tickTimer += deltaTime;
	sd.seccondsTimer += deltaTime;
	sd.tickDeltaTime += deltaTime;
#pragma endregion

	//tasks stuff
	{
		std::vector<ServerTask> tasks;

		//I also commented the notify one thing in submittaskforserver!
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

		tasks = tryForTasksServer();

		for (auto i : tasks)
		{
			sd.waitingTasks.push_back(i);
		}
	}

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
					sendNewBlocksToPlayers, true, nullptr, worldSaver, &wasGenerated);
				profiler.end();

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
					lockConnectionsMutex();
					auto client = getClientNotLocked(i.cid);
					if (client)
					{
						sendPacketAndCompress(client->peer, packet, (char *)rez,
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
					, biomesManager, sendNewBlocksToPlayers, true, nullptr, worldSaver, &wasGenerated);
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

				unlockConnectionsMutex();

			}
			else if (i.t.type == Task::droppedItemEntity)
			{

				lockConnectionsMutex();
				auto client = getClientNotLocked(i.cid);

				//todo some logic
				//todo add items here into the server's storage
				if (client)
				{

					auto serverAllows = getClientSettingCopy(i.cid).validateStuff;
					
					if (sd.chunkCache.entityAlreadyExists(i.t.entityId)) { serverAllows = false; }


					if (computeRevisionStuff(*client, true && serverAllows, i.t.eventId))
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
							chunk->entityData.droppedItems.insert({i.t.entityId, newEntity});
						}

						//std::cout << "restant: " << newEntity.restantTime << "\n";


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



	//todo check if there are too many loaded chunks and unload them before processing
	//generate chunk

#pragma region gameplay tick

	if (sd.tickTimer > 1.f / settings.targetTicksPerSeccond)
	{
		sd.tickTimer -= (1.f / settings.targetTicksPerSeccond);
		sd.ticksPerSeccond++;


		{

			static bool did = 0;

			if (settings.perClientSettings.begin()->second.spawnZombie && !did)
			{
				did = true;

				auto c = getAllClients();


				Zombie z;
				glm::dvec3 position = c.begin()->second.playerData.position;
				z.position = position;
				z.lastPosition = position;

				spawnZombie(sd.chunkCache, z, getEntityIdSafeAndIncrement());
			}


		}


		updateLoadedChunks(wg, structuresManager, biomesManager, sendNewBlocksToPlayers,
			worldSaver);
	
		sd.chunkCache.unloadChunksThatNeedUnloading(worldSaver, 100);

		//todo error and warning logs for server.


		//tick ...
		doGameTick(sd.tickDeltaTime, currentTimer, wg,
			structuresManager, biomesManager, sendNewBlocksToPlayers,
			worldSaver);
		//std::cout << "Loaded chunks: " << sd.chunkCache.loadedChunks.size() << "\n";

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



template <class T, int Packet_Type, class E>
void genericBroadcastEntityUpdateFromServerToPlayer(E &e, bool reliable, 
	std::uint64_t currentTimer)
{

	Packet packet;
	packet.header = Packet_Type;

	T packetData;
	packetData.eid = e.first;
	packetData.entity = e.second.entity;
	packetData.timer = currentTimer;

	broadCast(packet, &packetData, sizeof(packetData),
		nullptr, reliable, channelEntityPositions);

}


template<class T>
void genericCallUpdateForEntity(T &e,
	float deltaTime, ChunkData*(chunkGetter)(glm::ivec2))
{
	float time = deltaTime + e.second.restantTime;
	if (time > 0)
	{
		e.second.update(time, chunkGetter, sd.chunkCache);
	}
	e.second.restantTime = 0;
};


//todo remove this comments
//void moveDroppedItem(glm::ivec2 initialChunk, glm::ivec2 newChunk, 
//	std::uint64_t id, ServerEntityManager &entityManager, bool &shouldDelete)
//{
//	shouldDelete = 0;
//
//	if (initialChunk != newChunk)
//	{
//		auto c = sd.chunkCache.getChunkOrGetNull(initialChunk.x, initialChunk.y);
//
//		if (c)
//		{
//
//			auto pos = c->otherData.droppedItems.find(id);
//			if (pos != c->otherData.droppedItems.end())
//			{
//				c->otherData.droppedItems.erase(pos);
//
//				auto c2 = sd.chunkCache.getChunkOrGetNull(newChunk.x,
//					newChunk.y);
//
//				if (c2)
//				{
//					c2->otherData.droppedItems.insert(id);
//				}
//				else
//				{
//					//unload that entity and save it in the new chunk
//					shouldDelete = 1;
//
//				}
//
//			}
//			else
//			{
//				//todo search for this item otherwhere
//			}
//
//		}
//		else
//		{
//			//todo search for this item otherwhere
//		}
//	}
//};
//
//void moveZombie(glm::ivec2 initialChunk, glm::ivec2 newChunk,
//	std::uint64_t id, ServerEntityManager &entityManager, bool &shouldDelete)
//{
//	shouldDelete = 0;
//
//	if (initialChunk != newChunk)
//	{
//		auto c = sd.chunkCache.getChunkOrGetNull(initialChunk.x, initialChunk.y);
//
//		if (c)
//		{
//
//			auto pos = c->otherData.zombies.find(id);
//			if (pos != c->otherData.zombies.end())
//			{
//				c->otherData.zombies.erase(pos);
//
//				auto c2 = sd.chunkCache.getChunkOrGetNull(newChunk.x,
//					newChunk.y);
//
//				if (c2)
//				{
//					c2->otherData.zombies.insert(id);
//				}
//				else
//				{
//					//unload that entity and save it in the new chunk
//					shouldDelete = 1;
//
//				}
//
//			}
//			else
//			{
//				//todo search for this item otherwhere
//			}
//
//		}
//		else
//		{
//			//todo search for this item otherwhere
//		}
//	}
//};

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
		
		glm::ivec2 pos(divideChunk(c.second.playerData.position.x), 
			divideChunk(c.second.playerData.position.z));
		
		
		//todo
		for (int i = -simulationDistance; i <= simulationDistance; i++)
			for (int j = -simulationDistance; j <= simulationDistance; j++)
			{
				glm::dvec2 vect(i, j);

				float dist = std::sqrt(glm::dot(vect, vect));

				if (dist <= simulationDistance)
				{
					auto finalPos = pos + glm::ivec2(i, j);
					auto c = sd.chunkCache.getOrCreateChunk(finalPos.x, finalPos.y,
						wg, structureManager, biomesManager, sendNewBlocksToPlayers, true,
						nullptr, worldSaver, false
						);

					if (c)
					{
						c->otherData.shouldUnload = false;
					}
				}
			}
	}
	


}


void doGameTick(float deltaTime, std::uint64_t currentTimer, WorldGenerator &wg, 
	StructuresManager &structuresManager, BiomesManager &biomesManager,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	WorldSaver &worldSaver)
{

	auto chunkGetter = [](glm::ivec2 pos) -> ChunkData *
	{
		auto c = sd.chunkCache.getChunkOrGetNull(pos.x, pos.y);
		if (c)
		{
			return &c->chunk;
		}
		else
		{
			return nullptr;
		}
	};

	//todo when a player joins, send him all of the entities


#pragma region entity updates

	EntityData orphanEntities;
	
	for (auto &c : sd.chunkCache.savedChunks)
	{
		auto &entityData = c.second->entityData;

		auto initialChunk = c.first;

		for (auto it = entityData.droppedItems.begin(); it != entityData.droppedItems.end(); )
		{
			auto &e = *it;

			genericCallUpdateForEntity(e, deltaTime, chunkGetter);
			auto newChunk = determineChunkThatIsEntityIn(e.second.getPosition());

			//todo this should take into acount if that player should recieve it
			genericBroadcastEntityUpdateFromServerToPlayer
				< Packet_RecieveDroppedItemUpdate,
				headerClientRecieveDroppedItemUpdate>(e, false, currentTimer);

			if (initialChunk != newChunk)
			{
				orphanEntities.droppedItems.insert(
					{e.first, e.second});
			
				it = entityData.droppedItems.erase(it);
			}
			else
			{
				++it;
			}

		}

		for (auto it = entityData.zombies.begin(); it != entityData.zombies.end(); )
		{
			auto &e = *it;

			genericCallUpdateForEntity(e, deltaTime, chunkGetter);
			auto newChunk = determineChunkThatIsEntityIn(e.second.getPosition());

			//todo this should take into acount if that player should recieve it
			genericBroadcastEntityUpdateFromServerToPlayer
				< Packet_UpdateZombie,
				headerUpdateZombie>(e, false, currentTimer);

			if (initialChunk != newChunk)
			{
				orphanEntities.zombies.insert(
					{e.first, e.second});
			
				it = entityData.zombies.erase(it);

				std::cout << "Orphaned!\n";
			}
			else
			{
				++it;
			}
		}


	}
	
	
	//re set entities in their new chunk
	for (auto &e : orphanEntities.droppedItems)
	{
		auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
		auto chunk = sd.chunkCache.getChunkOrGetNull(pos.x, pos.y);

		if (chunk)
		{
			chunk->entityData.droppedItems.insert({e.first, e.second});
		}
		else
		{
			//todo save entity to disk!
		}
	}

	for (auto &e : orphanEntities.zombies)
	{
		auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
		auto chunk = sd.chunkCache.getChunkOrGetNull(pos.x, pos.y);

		if (chunk)
		{
			chunk->entityData.zombies.insert({e.first, e.second});
		}
		else
		{
			//todo save entity to disk!
		}
	}


#pragma endregion





}