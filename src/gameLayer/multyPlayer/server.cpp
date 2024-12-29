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
#include <gameplay/food.h>
#include <profiler.h>

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
	ENetHost *server = nullptr;
	ServerSettings settings = {};

	float tickTimer = 0;
	float tickDeltaTime = 0;
	int tickDeltaTimeMs = 0;
	int ticksPerSeccond = 0;
	int runsPerSeccond = 0;
	float seccondsTimer = 0;

	float saveEntitiesTimer = 5;
	std::uint64_t lastTimer = 0;


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

	sd.lastTimer = getTimer();

	return true;
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


ServerSettings getServerSettingsCopy()
{
	return sd.settings;
}

ServerSettings &getServerSettingsReff()
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

void genericBroadcastEntityDeleteFromServerToPlayer(std::uint64_t eid, bool reliable, 
	std::unordered_map<std::uint64_t, Client *> &allClients, 
	glm::ivec2 lastChunkClientsGotUpdates)
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
	float deltaTime, Profiler &serverProfiler
	)
{

#pragma region timers stuff
	auto currentTimer = getTimer();
	sd.tickTimer += deltaTime;
	sd.seccondsTimer += deltaTime;
	sd.tickDeltaTime += deltaTime;
	sd.saveEntitiesTimer -= deltaTime;
	auto deltaTimeMS = currentTimer - sd.lastTimer;
	sd.tickDeltaTimeMs += deltaTimeMS;
#pragma endregion

	auto &settings = sd.settings;

	static std::minstd_rand rng(std::random_device{}());
	static bool generateNewChunks = 0; //this is set to true only once per tick



#pragma region send chunks to players

	serverProfiler.startSubProfile("Send Chunks To players");

	std::vector<SendBlocksBack> sendNewBlocksToPlayers;
	updateLoadedChunks(wg, structuresManager, biomesManager, sendNewBlocksToPlayers,
		worldSaver, true);
	generateNewChunks = 0;

	serverProfiler.endSubProfile("Send Chunks To players");


#pragma endregion

#pragma region unload chunks
	serverProfiler.startSubProfile("Unload chunks");
	sd.chunkCache.unloadChunksThatNeedUnloading(worldSaver, 2);
	serverProfiler.startSubProfile("Unload chunks");
#pragma endregion


	serverProfiler.startSubProfile("Set players in chunks and check if they are killed");

#pragma region set players in their chunks
	for (auto &c : sd.chunkCache.savedChunks)
	{
		c.second->entityData.players.clear();
	}

	//todo move in tick probably
	//set players in their chunks, set players in chunks
	for (auto &client : getAllClientsReff())
	{

		auto cPos = determineChunkThatIsEntityIn(client.second.playerData.entity.position);

		auto chunk = sd.chunkCache.getChunkOrGetNull(cPos.x, cPos.y);

		permaAssertComment(chunk, "Error, A chunk that a player is in unloaded...");

		//if (!client.second.playerData.killed)
		{
			chunk->entityData.players[client.first] = &client.second.playerData;
			sd.chunkCache.entityChunkPositions[client.first] = cPos;
			auto ptr = &client.second.playerData;
		}

	}
#pragma endregion



	//todo this will be moved in the tick after refactoring
#pragma region check players killed


	auto &clients = getAllClientsReff();
	for (auto &c : clients)
	{
		//kill players
		if (c.second.playerData.newLife.life <= 0 && !c.second.playerData.killed)
		{
			//todo a method to reset multiple things
			c.second.playerData.kill();
			c.second.playerData.killed = true;

			genericBroadcastEntityKillFromServerToPlayer(c.first, true);
		}
		else
		{
			//per client life updates happen multi threaded
		}

	}


#pragma endregion

	serverProfiler.endSubProfile("Set players in chunks and check if they are killed");


	//here used to be the tasks


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
		//TODO this should run once at server startup, and also create this chunk,
		//just at start
		{

			glm::ivec3 spawnPos = worldSaver.spawnPosition;
			auto spawnChunk = sd.chunkCache.getChunkOrGetNull(divideChunk(spawnPos.x),
				divideChunk(spawnPos.z));

			//only if the chunk is loaded for now
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

				c.begin()->second.playerData.applyDamageOrLife(-10);
			}

			if (settings.perClientSettings.begin()->second.heal)
			{
				settings.perClientSettings.begin()->second.heal = false;
				auto &c = getAllClientsReff();

				c.begin()->second.playerData.applyDamageOrLife(10);
			}

			//TODO chunks shouldn't be nullptrs so why check them?
			//	// so maybe just perma assert comment at the beginning

			//if (settings.perClientSettings.begin()->second.killApig)
			//{
			//	settings.perClientSettings.begin()->second.killApig = false;
			//
			//	
			//
			//	for (auto &c : sd.chunkCache.savedChunks)
			//	{
			//		if (c.second && c.second->entityData.pigs.size())
			//		{
			//			killEntity(worldSaver, c.second->entityData.pigs.begin()->first);
			//			break;
			//		}
			//	}
			//
			//}
		}



		//todo error and warning logs for server.


		//todo get all clients should probably dissapear.
		auto &clients = getAllClientsReff();

		for (auto &c : clients)
		{
			c.second.playerData.inventory.sanitize();
		}

		splitUpdatesLogic(sd.tickDeltaTime, sd.tickDeltaTimeMs,
			currentTimer, sd.chunkCache, rng(), clients, worldSaver, serverTask,
			serverProfiler);

		sd.tickDeltaTime = 0;
		sd.tickDeltaTimeMs = 0;
	}

	//std::cout << deltaTime << " <- dt / 1/dt-> " << (1.f / (deltaTime)) << "\n";

	//std::cout << seccondsTimer << '\n';

	sd.runsPerSeccond++;

	if (sd.seccondsTimer >= 1)
	{
		sd.seccondsTimer -= 1;
		//std::cout << "Server ticks per seccond: " << sd.ticksPerSeccond << "\n";
		//std::cout << "Server runs per seccond: " << sd.runsPerSeccond << "\n";
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

#pragma region save stuff
	//save one chunk on disk
	serverProfiler.startSubProfile("Save chunk on disk");
	sd.chunkCache.saveNextChunk(worldSaver);
	serverProfiler.endSubProfile("Save chunk on disk");

	//mark all entities as dirty every 5 secconds, so we save them
	if (sd.saveEntitiesTimer <= 0)
	{
		sd.saveEntitiesTimer = 5;

		for (auto &c : sd.chunkCache.savedChunks)
		{
			c.second->otherData.dirtyEntity = true;
		}

	}
#pragma endregion

	sd.lastTimer = currentTimer;

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

	auto &clients = getAllClientsReff();


	//todo a better way to prioritize ordering and stuff
	std::vector<glm::ivec2> positions;
	positions.reserve(200);

	for (auto &c : clients)
	{

		//if (c.second.playerData.killed) { continue; }

		int geenratedThisFrame = 0;

		glm::ivec2 pos(divideChunk(c.second.playerData.entity.position.x),
			divideChunk(c.second.playerData.entity.position.z));
		
		glm::ivec2 playerPos(c.second.playerData.entity.position.x, c.second.playerData.entity.position.z);

		int distance = (c.second.playerData.entity.chunkDistance/2) + 1;

		auto &client = c.second;

		//drop chunks that are too far
		{
			for (auto it = client.loadedChunks.begin(); it != client.loadedChunks.end();)
			{

				if (!isChunkInRadius(playerPos, *it, c.second.playerData.entity.chunkDistance))
				{
					it = client.loadedChunks.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		positions.clear();

		for (int i = -distance; i <= distance; i++)
			for (int j = -distance; j <= distance; j++)
			{
				glm::vec2 vect(i, j);
				auto chunkPos = pos + glm::ivec2(i, j);


				if (isChunkInRadius(playerPos, chunkPos, c.second.playerData.entity.chunkDistance))
				{
					positions.push_back({chunkPos});
				}
			}

		std::sort(positions.begin(), positions.end(),
			[&](auto &a, auto &b)
		{
			
			glm::vec2 diff1 = a - pos;
			float distance1 = glm::dot(diff1, diff1);

			glm::vec2 diff2 = b - pos;
			float distance2 = glm::dot(diff2, diff2);

			return distance1 < distance2;
		});

		bool generatedChunkPlayerIsIn = 0;
		for (auto chunkPos : positions)
		{
			SavedChunk *c = 0;

			if ((generateNewChunks && (geenratedThisFrame < MAX_GENERATE)) 
				
				//always generate the chunk that the player is in
				|| (chunkPos == pos))
			{

				if (chunkPos == pos) { generatedChunkPlayerIsIn = true; }

				bool generated = 0;

				c = sd.chunkCache.getOrCreateChunk(chunkPos.x, chunkPos.y,
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
				c = sd.chunkCache.getChunkOrGetNull(chunkPos.x, chunkPos.y);
			}

			//optional
			//stop sending chunks, so we don't send far chunks to the player, instead we should
			//focus on loading close chunks
			if (geenratedThisFrame >= MAX_GENERATE && generatedChunkPlayerIsIn)
			{
				break;
			}

			if (c)
			{
				c->otherData.shouldUnload = false;

				//send chunk to player
			#pragma region send chunk to player

				if (client.loadedChunks.find(chunkPos) ==
					client.loadedChunks.end())
				{

					client.loadedChunks.insert(chunkPos);

					Packet packet;
					packet.header = headerRecieveChunk;

					//if you have modified Packet_RecieveChunk make sure you didn't break this!
					static_assert(sizeof(Packet_RecieveChunk) == sizeof(ChunkData));

					{
						sendPacketAndCompress(client.peer, packet, (char *)(&c->chunk),
							sizeof(Packet_RecieveChunk), true, channelChunksAndBlocks);


						std::vector<unsigned char> blockData;
						c->blockData.formatBlockData(blockData, c->chunk.x, c->chunk.z);

						if (blockData.size())
						{
							Packet packet;
							packet.header = headerRecieveBlockData;

							if (blockData.size() > 1000)
							{
								sendPacketAndCompress(client.peer, packet, (char *)blockData.data(),
									blockData.size(), true, channelChunksAndBlocks);
							}
							else
							{
								sendPacket(client.peer, packet, (char *)blockData.data(),
									blockData.size(), true, channelChunksAndBlocks);
							};

						}

					}
				}
			#pragma endregion




			}

		};


	}


};
	




 