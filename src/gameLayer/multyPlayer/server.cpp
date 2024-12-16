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
#include <gameplay/crafting.h>
#include <gameplay/cat.h>
#include <gameplay/gameplayRules.h>


static std::atomic<bool> serverRunning = false;

bool serverStartupStuff(const std::string &path);

bool isServerRunning()
{
	return serverRunning;
}

bool startServer(const std::string &path)
{
	bool expected = 0;
	if (serverRunning.compare_exchange_strong(expected, 1))
	{
		if (!serverStartupStuff(path))
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
	WorldSaver &worldSaver, bool generateNewChunks);


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

ServerChunkStorer &getServerChunkStorer()
{
	return sd.chunkCache;
}

void clearSD(WorldSaver &worldSaver)
{
	//todo saveEntityId stuff
	//worldSaver.saveEntityId(getCurrentEntityId());
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

bool serverStartupStuff(const std::string &path)
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

	if (!startEnetListener(sd.server, path))
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

		c->entityData.pigs.insert({getEntityIdAndIncrement(worldSaver, EntityType::pigs), e});
	}
	else
	{
		return 0;
	}
	return 1;
}

bool spawnGoblin(
	ServerChunkStorer &chunkManager,
	Goblin goblin, WorldSaver &worldSaver,
	std::minstd_rand &rng)
{
	//todo also send packets
	//todo generic spawn for any entity

	auto chunkPos = determineChunkThatIsEntityIn(goblin.position);
	auto c = chunkManager.getChunkOrGetNull(chunkPos.x, chunkPos.y);
	if (c)
	{
		GoblinServer e = {};
		e.entity = goblin;
		//e.configureSpawnSettings(rng);

		c->entityData.goblins.insert({getEntityIdAndIncrement(worldSaver, EntityType::goblins), e});
	}
	else
	{
		return 0;
	}
	return 1;
}

bool spawnCat(
	ServerChunkStorer &chunkManager,
	Cat cat, WorldSaver &worldSaver,
	std::minstd_rand &rng)
{
	//todo also send packets
	//todo generic spawn for any entity

	auto chunkPos = determineChunkThatIsEntityIn(cat.position);
	auto c = chunkManager.getChunkOrGetNull(chunkPos.x, chunkPos.y);
	if (c)
	{
		CatServer e = {};
		e.entity = cat;
		e.configureSpawnSettings(rng);

		c->entityData.cats.insert({getEntityIdAndIncrement(worldSaver, EntityType::cats), e});
	}
	else
	{
		return 0;
	}
	return 1;
}

bool spawnDroppedItemEntity(
	ServerChunkStorer &chunkManager, WorldSaver &worldSaver,
	unsigned char counter, unsigned short type,
	std::vector<unsigned char> *metaData, glm::dvec3 pos, MotionState motionState = {},
	std::uint64_t newId = 0,
	float restantTimer = 0
	)
{

	if (newId == 0) { newId = getEntityIdAndIncrement(worldSaver, EntityType::droppedItems);  }

	DroppedItemServer newEntity = {};
	newEntity.item.counter = counter;
	newEntity.item.type = type;
	if (metaData)
	{
		newEntity.item.metaData = *metaData;
	}

	newEntity.entity.position = pos;
	newEntity.entity.lastPosition = pos;
	newEntity.entity.forces = motionState;
	newEntity.restantTime = restantTimer;

	auto chunkPosition = determineChunkThatIsEntityIn(pos);
	auto chunk = chunkManager.getChunkOrGetNull(chunkPosition.x, chunkPosition.y);

	if (chunk)
	{
		chunk->entityData.droppedItems.insert({newId, newEntity});
	}
	else
	{
		return 0;
	}

	return 1;

}

void changePlayerGameMode(std::uint64_t cid, unsigned char gameMode)
{

	auto client = getClientNotLocked(cid);

	if (client)
	{
		if (client->playerData.otherPlayerSettings.gameMode != gameMode)
		{
			client->playerData.otherPlayerSettings.gameMode = gameMode;

			Packet_UpdateOwnOtherPlayerSettings packet;
			packet.otherPlayerSettings = client->playerData.otherPlayerSettings;

			sendPacket(client->peer, headerUpdateOwnOtherPlayerSettings,
				&packet, sizeof(packet), true, channelChunksAndBlocks);

		}

	}
}


void sendDamagePlayerPacket(Client &client)
{
	Packet_UpdateLife p;
	p.life = client.playerData.life;
	sendPacket(client.peer, headerRecieveDamage, &p, sizeof(p), true, channelChunksAndBlocks);
}

void sendIncreaseLifePlayerPacket(Client &client)
{
	Packet_UpdateLife p;
	p.life = client.playerData.life;
	sendPacket(client.peer, headerRecieveLife, &p, sizeof(p), true, channelChunksAndBlocks);
}

//sets the life of the player with no animations
void sendUpdateLifeLifePlayerPacket(Client &client)
{
	Packet_UpdateLife p;
	p.life = client.playerData.life;
	sendPacket(client.peer, headerUpdateLife, &p, sizeof(p), true, channelChunksAndBlocks);
}

void applyDamageOrLifeToPlayer(short difference, Client &client)
{
	if (difference == 0) { return; }

	int life = client.playerData.life.life;
	life += difference;
	if (life < 0) { life = 0; }
	if (life > client.playerData.life.maxLife) { life = client.playerData.life.maxLife; }
	client.playerData.life.life = life;

	if (difference < 0)
	{
		sendDamagePlayerPacket(client);
	}
	else
	{
		sendIncreaseLifePlayerPacket(client);
	}
}


void killEntity(WorldSaver &worldSaver, std::uint64_t entity)
{
	auto entityType = getEntityTypeFromEID(entity);

	if (entityType == EntityType::player)
	{
		//genericBroadcastEntityKillFromServerToPlayer(entity, true);
		//sd.chunkCache.e
		auto &clients = getAllClientsReff();
		auto found = clients.find(entity);
		
		//it is enough to set the life of players to 0 to kill them!
		if (found != clients.end())
		{
			found->second.playerData.life.life = 0;
		};
	}
	else
	{
		if (sd.chunkCache.removeEntity(worldSaver, entity))
		{
			genericBroadcastEntityKillFromServerToPlayer(entity, true);
		}
	}


}

ServerSettings getServerSettingsCopy()
{
	return sd.settings;
}

unsigned int getRandomTickSpeed()
{
	return sd.settings.randomTickSpeed;
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

void genericBroadcastEntityDeleteFromServerToPlayer(std::uint64_t eid, bool reliable)
{
	Packet packet;
	packet.header = headerRemoveEntity;

	Packet_RemoveEntity data;
	data.EID = eid;

	broadCast(packet, &data, sizeof(data),
		nullptr, reliable, channelEntityPositions);
}

void genericBroadcastEntityKillFromServerToPlayer(std::uint64_t eid, bool reliable, ENetPeer *peerToIgnore)
{
	Packet packet;
	packet.header = headerKillEntity;

	Packet_KillEntity data;
	data.EID = eid;

	broadCast(packet, &data, sizeof(data),
		peerToIgnore, reliable, channelEntityPositions);
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

	static std::minstd_rand rng(std::random_device{}());
	static bool generateNewChunks = 0; //this is set to true only once per tick

	//std::cout << "Before\n";
	std::vector<SendBlocksBack> sendNewBlocksToPlayers;
	updateLoadedChunks(wg, structuresManager, biomesManager, sendNewBlocksToPlayers,
		worldSaver, true);
	generateNewChunks = 0;

	//std::cout << "After\n";


	for (auto &c : sd.chunkCache.savedChunks)
	{
		c.second->entityData.players.clear();
	}

	//set players in their chunks, set players in chunks
	for (auto &client : getAllClients())
	{

		auto cPos = determineChunkThatIsEntityIn(client.second.playerData.entity.position);

		auto chunk = sd.chunkCache.getChunkOrGetNull(cPos.x, cPos.y);

		permaAssertComment(chunk, "Error, A chunk that a player is in unloaded...");

		if (!client.second.playerData.killed)
		{
			chunk->entityData.players.insert({client.first, &client.second.playerData});
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

#pragma region check players killed


	auto &clients = getAllClientsReff();
	for (auto &c : clients)
	{
		//kill player
		if (c.second.playerData.life.life <= 0 && !c.second.playerData.killed)
		{
			//todo a method to reset multiple things
			c.second.playerData.killed = true;
			c.second.playerData.interactingWithBlock = 0;
			c.second.playerData.currentBlockInteractWithPosition = {0,-1,0};

			genericBroadcastEntityKillFromServerToPlayer(c.first, true);
		}

	}


#pragma endregion


	int chunksGenerated = 0;
	int chunksLoaded = 0;

	int count = sd.waitingTasks.size();
	for (int taskIndex = 0; taskIndex < std::min(count, 25); taskIndex++)
	{
		auto &i = sd.waitingTasks.front();

		if (i.t.taskType == Task::generateChunk)
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
				packet.header = headerRecieveChunk;


				//if you have modified Packet_RecieveChunk make sure you didn't break this!
				static_assert(sizeof(Packet_RecieveChunk) == sizeof(ChunkData));

				{
					auto client = getClientNotLocked(i.cid);
					if (client)
					{
						sendPacketAndCompress(client->peer, packet, (char *)(&rez->chunk),
							sizeof(Packet_RecieveChunk), true, channelChunksAndBlocks);
					}

					std::vector<unsigned char> blockData;
					rez->blockData.formatBlockData(blockData, rez->chunk.x, rez->chunk.z);

					if (blockData.size())
					{
						Packet packet;
						packet.header = headerRecieveBlockData;

						if (blockData.size() > 1000)
						{
							sendPacketAndCompress(client->peer, packet, (char*)blockData.data(),
								blockData.size(), true, channelChunksAndBlocks);
						}
						else
						{
							sendPacket(client->peer, packet, (char *)blockData.data(),
								blockData.size(), true, channelChunksAndBlocks);
						};

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
			if (i.t.taskType == Task::placeBlockForce)
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

					auto b = chunk->chunk.safeGet(convertedX, i.t.pos.y, convertedZ);
					bool good = 0;

					if (b)
					{
						auto block = i.t.blockType;

						if (isBlock(block) || block == 0)
						{
							good = true;
						}

						bool legal = computeRevisionStuff(*client, good, i.t.eventId);

						if (legal)
						{
							auto lastBlock = b->getType();
							chunk->removeBlockWithData({convertedX,
								i.t.pos.y, convertedZ}, lastBlock);
							b->setType(i.t.blockType);
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



			}else
			if (i.t.taskType == Task::placeBlock
				|| i.t.taskType == Task::breakBlock
				)
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

					//if revision number for the inventory is good we can continue,
					//	if else we need to undo that move
					if (
						i.t.taskType == Task::breakBlock ||
						(
						client->playerData.inventory.revisionNumber
						== i.t.revisionNumber)
						)
					{
						if (i.t.taskType == Task::breakBlock)
						{
							i.t.blockType = 0;
						}

						bool legal = 1;
						bool rensendInventory = 0;

						if (client->playerData.killed)
						{
							legal = 0;
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

						auto b = chunk->chunk.safeGet(convertedX, i.t.pos.y, convertedZ);
						Item *item = 0;

						Block actualPlacedBLock;
						actualPlacedBLock.typeAndFlags = i.t.blockType;

						if (!b)
						{
							legal = false;
						}
						else
						{

							if (i.t.taskType == Task::placeBlock)
							{
								item = client->playerData.inventory.getItemFromIndex(i.t.inventroySlot);



								if(item && item->isBlock() && 
									actualPlacedBLock.getType() == item->type
									&& item->counter
									)
								{
									//good
								}
								else
								{
									legal = false;
									rensendInventory = true;
								}
							}

							if (i.t.taskType == Task::placeBlock)
							{
								if (!canBlockBePlaced(actualPlacedBLock.getType(), b->getType()))
								{
									legal = false;
								}
							}
							else
							{
								if (!canBlockBeBreaked(b->getType(), client->playerData.otherPlayerSettings.gameMode
									== OtherPlayerSettings::CREATIVE))
								{
									legal = false;
								}
							}
							

							if (i.t.taskType == Task::placeBlock && isColidable(actualPlacedBLock.getType()))
							{
								//don't place blocks over entities

								if (sd.chunkCache.anyEntityIntersectsWithBlock(i.t.pos))
								{
									legal = false;
								}

							}

						}

						legal = computeRevisionStuff(*client, legal, i.t.eventId);

						if (legal)
						{
							auto lastBlock = b->getType();
							chunk->removeBlockWithData({convertedX,
								i.t.pos.y, convertedZ}, lastBlock);
							*b = actualPlacedBLock;
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

							if (i.t.taskType == Task::placeBlock)
							{
								if (client->playerData.otherPlayerSettings.gameMode ==
									OtherPlayerSettings::SURVIVAL)
								{
									item->counter--;
									item->sanitize();
								};
							}

							if (i.t.taskType == Task::breakBlock)
							{

								if (client->playerData.otherPlayerSettings.gameMode ==
									OtherPlayerSettings::SURVIVAL)
								{
									//todo other checks here like tools

									MotionState ms;
									ms.velocity.y = 2;

									spawnDroppedItemEntity(sd.chunkCache,
										worldSaver, 1, lastBlock, nullptr,
										glm::dvec3(i.t.pos), ms);


								}


							}
						}

						if (rensendInventory)
						{
							sendPlayerInventoryAndIncrementRevision(*client);
						}

						if (legal)
						{

							for (auto &c : getAllClientsReff())
							{

								if (c.second.playerData.interactingWithBlock &&
									c.second.playerData.currentBlockInteractWithPosition ==
									i.t.pos
									)
								{
									//close interaction with block.
									//todo close chests here.
									c.second.playerData.interactingWithBlock = 0;
									c.second.playerData.currentBlockInteractWithPosition = {0,-1,0};
								}
							}


						}

					}
					else
					{
						//undo that move
						computeRevisionStuff(*client, false, i.t.eventId);
					}
					

				}


			}

		
			//todo this and also item usages need to use the current inventory's revision state!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//	for validation
			else if (i.t.taskType == Task::droppedItemEntity)
			{

				auto client = getClientNotLocked(i.cid);


				if (client)
				{

					//if the revision number isn't good
					//we tell the player to kill that item.
					bool killItem = 0;

					if (client->playerData.inventory.revisionNumber
						== i.t.revisionNumber
						)
					{

						auto serverAllows = settings.perClientSettings[i.cid].validateStuff;

						if (client->playerData.killed)
						{
							serverAllows = 0;
						}

					
						if (
							getEntityTypeFromEID(i.t.entityId) != EntityType::droppedItems ||
							getOnlyIdFromEID(i.t.entityId)  >= RESERVED_CLIENTS_ID)
						{
							//todo well this can cause problems
							//so we should better do a hard reset here
							serverAllows = false;
						}

						auto from = client->playerData.inventory.getItemFromIndex(i.t.from);

						if (!from) { serverAllows = false; }
						else
						{
							if (from->type != i.t.blockType)
							{
								serverAllows = false;
							}
							else if (from->counter < i.t.blockCount)
							{
								serverAllows = false;
							}
						}

						auto newId = getEntityIdAndIncrement(worldSaver, EntityType::droppedItems);

						if (computeRevisionStuff(*client, true && serverAllows, i.t.eventId,
							&i.t.entityId, &newId))
						{

							//todo get or create chunk here, so we create a function that cant fail.
							spawnDroppedItemEntity(sd.chunkCache,
								worldSaver, i.t.blockCount, i.t.blockType, &from->metaData,
								i.t.doublePos, i.t.motionState, newId,
								computeRestantTimer(i.t.timer, currentTimer));

							//std::cout << "restant: " << newEntity.restantTime << "\n";

							//substract item from inventory
							from->counter -= i.t.blockCount;
							if (!from->counter) { *from = {}; }

						}

						if (!serverAllows)
						{
							sendPlayerInventoryAndIncrementRevision(*client);
							killItem = true;
						}

					}
					else
					{
						std::cout << "Server revision : "
							<< (int)client->playerData.inventory.revisionNumber << "\n";

						std::cout << "Recieved revision : "
							<< (int)i.t.revisionNumber << "\n";

						killItem = true;
					}

					if (killItem)
					{
						entityDeleteFromServerToPlayer(*client, i.t.entityId, true);
					}

				}
				


			}
			else if (i.t.taskType == Task::clientMovedItem)
			{
				
				auto client = getClientNotLocked(i.cid);

				if (client)
				{

					//if the revision number isn't good we don't do anything
					if (client->playerData.inventory.revisionNumber
						== i.t.revisionNumber
						)
					{

						Item *from = client->playerData.inventory.getItemFromIndex(i.t.from);
						Item *to = client->playerData.inventory.getItemFromIndex(i.t.to);

						if (client->playerData.killed)
						{
							sendPlayerInventoryAndIncrementRevision(*client); //dissalow
						}else
						if (from && to)
						{
							//todo they should always be sanitized so we should check during task creation if they are


							if (from->type != i.t.itemType
								|| (i.t.blockCount > from->counter)
								)
							{
								//this is a desync, resend inventory.
								sendPlayerInventoryAndIncrementRevision(*client);
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
								else if (areItemsTheSame(*to, *from))
								{

									if (to->counter >= to->getStackSize())
									{
										sendPlayerInventoryAndIncrementRevision(*client);
									}

									int total = (int)to->counter + (int)i.t.blockCount;
									if (total <= to->getStackSize())
									{
										to->counter += i.t.blockCount;
										from->counter -= i.t.blockCount;

										if (!from->counter) { *from = {}; }
									}
									else
									{
										//this is a desync, resend inventory.
										sendPlayerInventoryAndIncrementRevision(*client);
									}

								}
								else
								{
									//this is a desync, resend inventory.
									sendPlayerInventoryAndIncrementRevision(*client);
								}

							}

						}
						else
						{
							sendPlayerInventoryAndIncrementRevision(*client);
						}

					};

				};

			}
			else if (i.t.taskType == Task::clientOverwriteItem)
			{


				auto client = getClientNotLocked(i.cid);

				if (client)
				{


					//if the revision number isn't good we don't do anything
					if (client->playerData.inventory.revisionNumber
						== i.t.revisionNumber
						)
					{

						if (client->playerData.killed)
						{
							sendPlayerInventoryAndIncrementRevision(*client); //dissalow
						}
						else
						if (client->playerData.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE)
						{

							Item *to = client->playerData.inventory.getItemFromIndex(i.t.to);

							if (to)
							{
								*to = {};
								to->counter = i.t.blockCount;
								to->type = i.t.itemType;
								to->metaData = std::move(i.t.metaData);
							}
							else
							{

								sendPlayerInventoryAndIncrementRevision(*client);
							}

						}
						else
						{
							sendPlayerInventoryAndIncrementRevision(*client);
							//todo send other player data
						}

					}

				}

			}
			else if (i.t.taskType == Task::clientSwapItems)
			{

				auto client = getClientNotLocked(i.cid);

				if (client)
				{

					//if the revision number isn't good we don't do anything
					if (client->playerData.inventory.revisionNumber
						== i.t.revisionNumber
						)
					{


						if (client->playerData.killed)
						{
							sendPlayerInventoryAndIncrementRevision(*client); //dissalow
						}
						else
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
							else
							{
								sendPlayerInventoryAndIncrementRevision(*client);
							}
						}

						
					}

				}

			}
			else if (i.t.taskType == Task::clientCraftedItem)
			{

				auto client = getClientNotLocked(i.cid);

				if (client)
				{
						


					//if the revision number isn't good we don't do anything
					if (client->playerData.inventory.revisionNumber
						== i.t.revisionNumber
						)
					{
						if (client->playerData.killed)
						{
							sendPlayerInventoryAndIncrementRevision(*client); //dissalow
						}
						else
						{ 
							int craftingIndex = i.t.craftingRecepieIndex;
							auto to = i.t.to;

							if (!recepieExists(craftingIndex))
							{
								sendPlayerInventoryAndIncrementRevision(*client); //dissalow
							}
							else
							{
								
								auto resultCrafting = getRecepieFromIndexUnsafe(craftingIndex);

								auto toslot = client->playerData.inventory.getItemFromIndex(to);

								if (!toslot)
								{
									sendPlayerInventoryAndIncrementRevision(*client); //dissalow
								}
								else
								{

									if (!canItemBeCrafted(resultCrafting, client->playerData.inventory))
									{
										sendPlayerInventoryAndIncrementRevision(*client);

									}
									else
									{
										if (!canItemBeMovedToAndMoveIt(resultCrafting.result, *toslot))
										{
											sendPlayerInventoryAndIncrementRevision(*client);
										}
										else
										{
											craftItemUnsafe(resultCrafting, client->playerData.inventory);
										}
									}

								}


							}

						}

						

					}
				}

			}
			else if (i.t.taskType == Task::clientUsedItem)
			{

				auto client = getClientNotLocked(i.cid);
				
				if (client)
				{

					//if the revision number isn't good we don't do anything
					if (client->playerData.inventory.revisionNumber
						== i.t.revisionNumber
						)
					{

						//serverTask.t.pos = packetData->position;

						Item *from = client->playerData.inventory.getItemFromIndex(i.t.from);

						if (from && !client->playerData.killed)
						{

							if (from->counter <= 0) { from = {}; }

							if (from->type == i.t.itemType)
							{


								if (from->type == ItemTypes::pigSpawnEgg)
								{
									Pig p;
									glm::dvec3 position = glm::dvec3(i.t.pos) + glm::dvec3(0.0, -0.49, 0.0);
									p.position = position;
									p.lastPosition = position;
									spawnPig(sd.chunkCache, p, worldSaver, rng);
								}
								else if (from->type == ItemTypes::zombieSpawnEgg)
								{
									Zombie z;
									glm::dvec3 position = glm::dvec3(i.t.pos) + glm::dvec3(0.0, -0.49, 0.0);
									z.position = position;
									z.lastPosition = position;
									spawnZombie(sd.chunkCache, z, getEntityIdAndIncrement(worldSaver, 
										EntityType::zombies));
								}
								else if (from->type == ItemTypes::catSpawnEgg)
								{
									Cat c;
									glm::dvec3 position = glm::dvec3(i.t.pos) + glm::dvec3(0.0, -0.49, 0.0);
									c.position = position;
									c.lastPosition = position;
									spawnCat(sd.chunkCache, c, worldSaver, rng);
								}
								else if (from->type == ItemTypes::goblinSpawnEgg)
								{
									Goblin g;
									glm::dvec3 position = glm::dvec3(i.t.pos) + glm::dvec3(0.0, -0.49, 0.0);
									g.position = position;
									g.lastPosition = position;
									spawnGoblin(sd.chunkCache, g, worldSaver, rng);
								}


								if (from->isConsumedAfterUse() && client->playerData.otherPlayerSettings.gameMode ==
									OtherPlayerSettings::SURVIVAL)
								{
									from->counter--;
									if (from->counter <= 0)
									{
										*from = {};
									}
								}
							}
							else
							{
								sendPlayerInventoryAndIncrementRevision(*client);
							}

						}
						else
						{
							sendPlayerInventoryAndIncrementRevision(*client);
						}

					};

				}

			}
			else if (i.t.taskType == Task::clientInteractedWithBlock)
			{

				auto pos = i.t.pos;
				auto blockType = i.t.blockType;
				unsigned char revisionNumberInteraction = i.t.revisionNumber;


				auto client = getClientNotLocked(i.cid);
				
				if (client)
				{

					bool allows = 0;

					auto client = getClientNotLocked(i.cid);

					if (client)
					{

						allows = true;
						{
							auto f = settings.perClientSettings.find(i.cid);
							if (f != settings.perClientSettings.end())
							{
								if (!f->second.validateStuff)
								{
									allows = false;
								}
							}
						}

						if (client->playerData.killed)
						{
							allows = false;
						}

						if (allows)
						{
							bool wasGenerated = 0;
							//std::cout << "server recieved place block\n";
							//auto chunk = sd.chunkCache.getOrCreateChunk(i.t.pos.x / 16, i.t.pos.z / 16);
							auto chunk = sd.chunkCache.getOrCreateChunk(divideChunk(i.t.pos.x), divideChunk(i.t.pos.z), wg, structuresManager
								, biomesManager, sendNewBlocksToPlayers, true, nullptr, worldSaver, &wasGenerated);
							int convertedX = modBlockToChunk(i.t.pos.x);
							int convertedZ = modBlockToChunk(i.t.pos.z);

							allows = false;

							if (chunk)
							{
								auto b = chunk->chunk.safeGet(convertedX, i.t.pos.y, convertedZ);

								if (b && b->getType() == blockType
									&& isInteractable(blockType)
									)
								{
									//todo check distance.
									allows = true;
								}
							}
						}
						
						if (allows)
						{

							client->playerData.interactingWithBlock =
								isInteractable(blockType);
							client->playerData.revisionNumberInteraction = revisionNumberInteraction;
							client->playerData.currentBlockInteractWithPosition = pos;


						}
						else
						{
							sendPlayerExitInteraction(*client, revisionNumberInteraction);
						}
					

					}
					

				}

			}
			else if (i.t.taskType == Task::clientExitedInteractionWithBlock)
			{
				unsigned char revisionNumberInteraction = i.t.revisionNumber;

				auto client = getClientNotLocked(i.cid);

				if (client)
				{
					if (client->playerData.revisionNumberInteraction == revisionNumberInteraction)
					{
						client->playerData.interactingWithBlock = 0;
						client->playerData.currentBlockInteractWithPosition = {0,-1,0};
						//std::cout << "Server, exit interaction!\n";
					}

				}


			}
			else if (i.t.taskType == Task::clientUpdatedSkin)
			{

				Packet p;
				p.header = headerSendPlayerSkin;
				p.cid = i.cid;
				
				auto client = getClientNotLocked(i.cid);
				
				if(client)
				{
					if (client->skinDataCompressed)
					{
						p.setCompressed();
					}

					broadCastNotLocked(p, client->skinData.data(),
						client->skinData.size(), client->peer, true, channelHandleConnections);
				}
			}
			else if (i.t.taskType == Task::clientAttackedEntity)
			{
				unsigned char itemInventoryIndex = i.t.inventroySlot;
				std::uint64_t entityId = i.t.entityId;
				glm::vec3 dir = i.t.vector;

				auto client = getClientNotLocked(i.cid);

				if (client)
				{
					if (!client->playerData.killed)
					{
						auto item = client->playerData.inventory.getItemFromIndex(itemInventoryIndex);
						if (item)
						{
							std::uint64_t wasKilled = 0;
							sd.chunkCache.hitEntityByPlayer(entityId, client->playerData.getPosition(),
								*item, wasKilled, dir, rng);

							if (wasKilled)
							{
								killEntity(worldSaver, wasKilled);
							}

						}
					}

				}

			}
			else if (i.t.taskType == Task::clientWantsToRespawn)
			{
				auto client = getClientNotLocked(i.cid);

				if (client)
				{
			
					if (client->playerData.killed)
					{

						client->playerData.life = Life(20); //todo a default player max life
						client->playerData.killed = false;
						sendPlayerInventoryAndIncrementRevision(*client);
						sendUpdateLifeLifePlayerPacket(*client);

						Packet packet;
						packet.cid = i.cid;
						packet.header = headerRespawnPlayer;

						Packet_RespawnPlayer packetData;
						packetData.pos = worldSaver.spawnPosition; //todo propper spawn position

						broadCastNotLocked(packet, &packetData, sizeof(packetData), 
							false, true, channelChunksAndBlocks);	
					}

				}

			}
			else if (i.t.taskType == Task::clientRecievedDamageLocally)
			{
				auto client = getClientNotLocked(i.cid);

				if (client && !client->playerData.killed)
				{
					client->playerData.life.life -= i.t.damage;
					if (client->playerData.life.life < 0)
					{
						client->playerData.life.life = 0;
					}
				}

			}
			else if (i.t.taskType == Task::clientRecievedDamageLocallyAndDied)
			{
				auto client = getClientNotLocked(i.cid);

				if (client && !client->playerData.killed)
				{
					//todo duplicate code above!!!!!
					client->playerData.killed = true;
					client->playerData.life.life = 0;
					client->playerData.interactingWithBlock = 0;
					client->playerData.currentBlockInteractWithPosition = {0,-1,0};

					genericBroadcastEntityKillFromServerToPlayer(i.cid, true, 
						client->peer);
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



	if (sd.tickTimer > 1.f / targetTicksPerSeccond)
	{
		//for (auto &c : sd.chunkCache.savedChunks)
		//{
		//	c.second->entityData.players.clear();
		//}
		//
		//for (auto &client : getAllClients())
		//{
		//
		//	auto cPos = determineChunkThatIsEntityIn(client.second.playerData.entity.position);
		//
		//	auto chunk = sd.chunkCache.getChunkOrGetNull(cPos.x, cPos.y);
		//
		//	permaAssertComment(chunk, "Error, A chunk that a player is in unloaded...");
		//
		//	chunk->entityData.players.insert({client.first, &client.second.playerData});
		//
		//}

		//todo if first time ever or not do it if the chunk isn't loaded!
	#pragma region replace spawn position
		//worldSaver.spawnPosition.y = 170;
		//if(0)
		{
			glm::ivec3 spawnPos = worldSaver.spawnPosition;
			auto spawnChunk = sd.chunkCache.getOrCreateChunk(divideChunk(spawnPos.x),
				divideChunk(spawnPos.z), wg, structuresManager, biomesManager,
				sendNewBlocksToPlayers, true, nullptr, worldSaver);

			if (spawnChunk)
			{
				glm::ivec3 blockPos = spawnPos;
				blockPos.x = modBlockToChunk(blockPos.x);
				blockPos.z = modBlockToChunk(blockPos.z);

				if (blockPos.y >= CHUNK_HEIGHT)
				{
					worldSaver.spawnPosition.y = CHUNK_HEIGHT;
				}
				else
				{
					if (blockPos.y < 1)
					{
						blockPos.y = 1;
					}

					//try down first
					{
						while (true)
						{
							auto b = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y, blockPos.z);

							if (!b)
							{
								break;
							}

							if (!b->isColidable())
							{
								auto bunder = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y - 1, blockPos.z);
								if (bunder && !bunder->isColidable())
								{
									blockPos.y--;
								}
								else
								{
									break;
								}
							}
							else
							{
								break;
							}
						}
					}
					
					while (true)
					{
						auto b = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y, blockPos.z);

						if (!b)
						{
							worldSaver.spawnPosition.y = blockPos.y;
							break;
						}

						if (!b->isColidable())
						{
							auto bunder = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y - 1, blockPos.z);
							if (bunder && bunder->isColidable())
							{
								auto bUp = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y + 1, blockPos.z);
								if (!bUp || !bUp->isColidable())
								{
									//good
									worldSaver.spawnPosition.y = blockPos.y;
									break;
								}
							}
						}
						blockPos.y++;
					}

				}
			}

		}
	#pragma endregion



		sd.tickTimer -= (1.f / targetTicksPerSeccond);
		sd.ticksPerSeccond++;

		if(settings.perClientSettings.size())
		{
			if (settings.perClientSettings.begin()->second.spawnZombie)
			{
				settings.perClientSettings.begin()->second.spawnZombie = false;


				auto c = getAllClients();

				Zombie z;
				glm::dvec3 position = c.begin()->second.playerData.entity.position;
				z.position = position;
				z.lastPosition = position;
				spawnZombie(sd.chunkCache, z, getEntityIdAndIncrement(worldSaver, 
					EntityType::zombies));
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

			if (settings.perClientSettings.begin()->second.spawnGoblin)
			{
				settings.perClientSettings.begin()->second.spawnGoblin = false;
				auto c = getAllClients();

				Goblin p;
				glm::dvec3 position = c.begin()->second.playerData.entity.position;
				p.position = position;
				p.lastPosition = position;
				spawnGoblin(sd.chunkCache, p, worldSaver, rng);
			}

			if (settings.perClientSettings.begin()->second.resendInventory)
			{
				settings.perClientSettings.begin()->second.resendInventory = false;
				auto &c = getAllClientsReff();

				sendPlayerInventoryAndIncrementRevision(c.begin()->second);
			}

			if (settings.perClientSettings.begin()->second.damage)
			{
				settings.perClientSettings.begin()->second.damage = false;
				auto &c = getAllClientsReff();

				applyDamageOrLifeToPlayer(-3, c.begin()->second);
			}

			if (settings.perClientSettings.begin()->second.heal)
			{
				settings.perClientSettings.begin()->second.heal = false;
				auto &c = getAllClientsReff();

				applyDamageOrLifeToPlayer(3, c.begin()->second);
			}

			if (settings.perClientSettings.begin()->second.killApig)
			{
				settings.perClientSettings.begin()->second.killApig = false;

				//TODO chunks shouldn't be nullptrs so why check them?
				// so maybe just perma assert comment at the beginning

				for (auto &c : sd.chunkCache.savedChunks)
				{
					if (c.second && c.second->entityData.pigs.size())
					{
						killEntity(worldSaver, c.second->entityData.pigs.begin()->first);
						break;
					}
				}

			}
		}


		sd.chunkCache.unloadChunksThatNeedUnloading(worldSaver, 10);

		//todo error and warning logs for server.


		//todo get all clients should probably dissapear.
		auto &clients = getAllClientsReff();

		for (auto &c : clients)
		{
			c.second.playerData.inventory.sanitize();
		}

		splitUpdatesLogic(sd.tickDeltaTime, currentTimer, sd.chunkCache, rng(), clients, worldSaver);

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
	WorldSaver &worldSaver, bool generateNewChunks)
{

	constexpr const int MAX_GENERATE = 1;

	for (auto &c : sd.chunkCache.savedChunks)
	{
		c.second->otherData.shouldUnload = true;
	}

	auto clientsCopy = getAllClients();


	for (auto &c : clientsCopy)
	{

		//if (c.second.playerData.killed) { continue; }

		int geenratedThisFrame = 0;

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
					SavedChunk *c = 0;

					//always generate the chunk that the player is in
					if ((generateNewChunks && (geenratedThisFrame < MAX_GENERATE)) || (i == 0 && j == 0))
					{
						bool generated = 0;

						c = sd.chunkCache.getOrCreateChunk(finalPos.x, finalPos.y,
							wg, structureManager, biomesManager, sendNewBlocksToPlayers, true,
							nullptr, worldSaver, &generated
						);

						if (generated)
						{
							geenratedThisFrame++;
						}
					}
					else
					{
						c = sd.chunkCache.getChunkOrGetNull(finalPos.x, finalPos.y);
					}
					
					if (c)
					{
						c->otherData.shouldUnload = false;
					}
					
				}
			}
	}
	


}

 