#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <gameplay/entity.h>
#include <multyPlayer/tick.h>
#include <chunkSystem.h>
#include <iostream>
#include <multyPlayer/enetServerFunction.h>
#include <deque>
#include <platformTools.h>
#include <gameplay/gameplayRules.h>
#include <gameplay/crafting.h>
#include <gameplay/food.h>

template <class T, class E>
void genericBroadcastEntityUpdateFromServerToPlayer(E &e, bool reliable,
	std::uint64_t currentTimer, int Packet_Type)
{

	Packet packet;
	packet.header = Packet_Type;

	T packetData;
	packetData.eid = e.first;

	if constexpr (hasGetDataToSend<decltype(e.second)>)
	{
		packetData.entity = e.second.getDataToSend();
	}else
	{
		packetData.entity = e.second.entity;
	}

	packetData.timer = currentTimer;

	broadCast(packet, &packetData, sizeof(packetData),
		nullptr, reliable, channelEntityPositions);
}


template <class E>
void genericBroadcastEntityUpdateFromServerToPlayer2(E &e, bool reliable,
	std::uint64_t currentTimer)
{

	Packet packet;
	packet.header = headerUpdateGenericEntity;

	static thread_local unsigned char data[sizeof(Packet_UpdateGenericEntity) + 100000];

	Packet_UpdateGenericEntity firstPart;
	firstPart.eid = e.first;
	firstPart.timer = currentTimer;

	memcpy(data, &firstPart, sizeof(firstPart));

	if constexpr (hasGetDataToSend<decltype(e.second)>)
	{
		auto entity = e.second.getDataToSend();
		memcpy(data + sizeof(Packet_UpdateGenericEntity), &entity, sizeof(entity));

		broadCast(packet, data, sizeof(Packet_UpdateGenericEntity) + sizeof(entity),
			nullptr, reliable, channelEntityPositions);
	}
	else
	{
		auto entity = e.second.entity;
		memcpy(data + sizeof(Packet_UpdateGenericEntity), &entity, sizeof(entity));

		broadCast(packet, data, sizeof(Packet_UpdateGenericEntity) + sizeof(entity),
			nullptr, reliable, channelEntityPositions);
	}



}



void entityDeleteFromServerToPlayer(std::uint64_t clientToSend,
	std::uint64_t eid, bool reliable)
{

	auto client = getClientSafe(clientToSend);

	if (client)
	{
		entityDeleteFromServerToPlayer(*client, eid, reliable);
	}

}

void entityDeleteFromServerToPlayer(Client &client, 
	std::uint64_t eid, bool reliable)
{

	Packet packet;
	packet.header = headerRemoveEntity;

	Packet_RemoveEntity data;
	data.EID = eid;

	sendPacket(client.peer, packet, (const char *)&data, sizeof(data),
		reliable, channelEntityPositions);
}


template<class T>
bool genericCallUpdateForEntity(T &e,
	float deltaTime, ChunkData *(chunkGetter)(glm::ivec2),
	ServerChunkStorer &chunkCache, std::minstd_rand &rng, 
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPositionSurvival,
	std::unordered_map < std::uint64_t, Client *> &allClients
	)
{
	float time = deltaTime;
	if constexpr (hasRestantTimer<decltype(e.second)>)
	{
		time = deltaTime + e.second.restantTime;
	}

	bool rez = 1;
	if (time > 0)
	{
		//todo pack things into a struct
		rez = e.second.update(time, chunkGetter, chunkCache, rng, e.first,
			othersDeleted, pathFinding, playersPositionSurvival, allClients);
	}


	if constexpr (hasLookDirectionAnimation<decltype(e.second.entity)>)
	{
		//e.second.entity.lookDirectionAnimation = e.second.wantToLookDirection;
		//e.second.entity.lookDirectionAnimation = {0,0,-1};
		//e.second.entity.bodyOrientation = {0,-1};
		glm::vec3 finalVector = orientVectorTowards(e.second.entity.getLookDirection(), 
			e.second.wantToLookDirection, deltaTime * glm::radians(180.f));
		//finalVector = e.second.wantToLookDirection;
		
		//glm::vec3 finalVector = orientVectorTowards(e.second.entity.lookDirectionAnimation, e.second.wantToLookDirection, deltaTime * glm::radians(70.f));

		if (glm::dot(glm::vec2(finalVector.x, finalVector.z), e.second.entity.bodyOrientation) > 0.5f)
		{
			lookAtDirection(finalVector, e.second.entity.lookDirectionAnimation,
				 e.second.entity.bodyOrientation,
				glm::radians(65.f));
		}
		else
		{
			lookAtDirectionWithBodyOrientation(finalVector, e.second.entity.lookDirectionAnimation,
				e.second.entity.bodyOrientation,
				glm::radians(65.f));
		}

	}

	if constexpr (hasRestantTimer<decltype(e.second)>)
	{
		e.second.restantTime = 0;
	}

	return rez;
};





template<class T, class U>
void genericResetEntitiesInTheirNewChunk(T &container, U memberSelector, ServerChunkStorer &chunkCache)
{
	for (auto it = container.begin();
		it != container.end();)
	{
		auto &e = *it;

		auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
		auto chunk = chunkCache.getChunkOrGetNull(pos.x, pos.y);

		if (chunk)
		{
			std::cout << "Found after orhpaned\n";
			auto member = memberSelector(chunk->entityData);
			(*member)[e.first] = e.second;
			it = container.erase(it);
		}
		else
		{
			//save entity to disk outside the thread.
			it++;
		}
	}
};


template <int... Is>
void callGenericResetEntitiesInTheirNewChunk(std::integer_sequence<int, Is...>, EntityData &c,
	ServerChunkStorer &chunkCache)
{
	(genericResetEntitiesInTheirNewChunk(*c.template entityGetter<Is + 1>(),
		[](auto &entityData) { return entityData.template entityGetter<Is + 1>(); },
		chunkCache), ...);
}


bool spawnDroppedItemEntity(
	ServerChunkStorer &chunkManager, WorldSaver &worldSaver,
	unsigned char counter, unsigned short type,
	std::vector<unsigned char> *metaData, glm::dvec3 pos, MotionState motionState = {},
	std::uint64_t newId = 0,
	float restantTimer = 0, float dontPickUpTimer = 1)
{

	if (newId == 0) { newId = getEntityIdAndIncrement(worldSaver, EntityType::droppedItems); }



	DroppedItemServer newEntity = {};
	newEntity.dontPickTimer = dontPickUpTimer;
	newEntity.item = itemCreator(type, counter);
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
		chunk->entityData.droppedItems[newId] = newEntity;
		chunkManager.entityChunkPositions[newId] = chunkPosition;
	}
	else
	{
		return 0;
	}

	return 1;

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
		chunkManager.entityChunkPositions[newId] = determineChunkThatIsEntityIn(serverZombie.getPosition());

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
		auto newId = getEntityIdAndIncrement(worldSaver, EntityType::pigs);
		c->entityData.pigs.insert({newId, e});
		chunkManager.entityChunkPositions[newId] = determineChunkThatIsEntityIn(e.getPosition());

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
		auto newId = getEntityIdAndIncrement(worldSaver, EntityType::goblins);
		c->entityData.goblins.insert({newId, e});
		chunkManager.entityChunkPositions[newId] = determineChunkThatIsEntityIn(e.getPosition());

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

		auto newId = getEntityIdAndIncrement(worldSaver, EntityType::cats);
		c->entityData.cats.insert({newId, e});
		chunkManager.entityChunkPositions[newId] = determineChunkThatIsEntityIn(e.getPosition());

	}
	else
	{
		return 0;
	}
	return 1;
}


void killEntity(WorldSaver &worldSaver, std::uint64_t entity, ServerChunkStorer &chunkCache)
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
			found->second.playerData.newLife.life = 0;
		};
	}
	else
	{
		if (chunkCache.removeEntity(worldSaver, entity))
		{
			genericBroadcastEntityKillFromServerToPlayer(entity, true);
		}
	}


}



#define ENTITY_UPDATES(X) genericLoopOverEntities(*entityData.entityGetter<X>(), *orphanEntities.entityGetter<X>(), [](auto &entityData) { return entityData.template entityGetter<X>(); });
#define ENTITY_MARK_NOTPDATED(X) genericMarkEntitiesNotUpdated(*entityData.entityGetter<X>());
#define ENTITY_ADD_TO_CACHE(X) genericAdd(*entityData.entityGetter<X>());


//todo make sure a player can only be in only one tick
//chunkCache has only chunks that shouldn't be unloaded! And no null ptrs!
void doGameTick(float deltaTime, int deltaTimeMs, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache, EntityData &orphanEntities,
	unsigned int seed, std::vector<ServerTask> waitingTasks,
	WorldSaver &worldSaver, Profiler *profiler)
{
	if (profiler) { profiler->startFrame(); }

	//std::cout << "Tick deltaTime: " << deltaTime << "\n";

	std::minstd_rand rng(seed);

	//todo this will probably be refactored
	auto &settings = getServerSettingsReff();


	std::unordered_map<glm::ivec3, Block> modifiedBlocks;

	static thread_local ServerChunkStorer *chunkCacheGlobal = 0;

	chunkCacheGlobal = &chunkCache;

	auto chunkGetter = [](glm::ivec2 pos) -> ChunkData *
	{
		auto c = chunkCacheGlobal->getChunkOrGetNull(pos.x, pos.y);
		if (c)
		{
			return &c->chunk;
		}
		else
		{
			return nullptr;
		}
	};

	//todo also send to entity update, and use there
	std::unordered_map <std::uint64_t, PlayerServer *> allPlayers;
	std::unordered_map < std::uint64_t, Client *> allClients;
	std::unordered_map < std::uint64_t, Client *> allSurvivalClients;

#pragma region calculate all players

	for (auto &c : chunkCache.savedChunks)
	{
		auto &playersMap = c.second->entityData.players;
		for (auto &p : playersMap)
		{
			if (p.second && !p.second->killed)
			{
				allPlayers.insert(p);
			}
		}
	}

	{
		auto &reff = getAllClientsReff();
		for (auto &p : allPlayers)
		{
			auto found = reff.find(p.first);
			permaAssert(found != reff.end());
			allClients.insert({found->first, &found->second});

			if (found->second.playerData.otherPlayerSettings.gameMode ==
				OtherPlayerSettings::SURVIVAL)
			{
				allSurvivalClients.insert({found->first, &found->second});
			}

		}
	}

#pragma endregion



	if (profiler) { profiler->startSubProfile("Calculate entities chunk position cache"); }
#pragma region calculate all entities chunk positions cache

	chunkCache.entityChunkPositions.clear();

	for (auto &c : chunkCache.savedChunks)
	{
		auto &entityData = c.second->entityData;
		
		auto genericAdd = [&](auto &container)
		{
			for (auto &p : container)
			{
				chunkCache.entityChunkPositions.insert({p.first, c.first});
			}
		};

		//genericAdd(*entityData.entityGetter<0>());
		REPEAT_FOR_ALL_ENTITIES(ENTITY_ADD_TO_CACHE)
	}

#pragma endregion
	if (profiler) { profiler->endSubProfile("Calculate entities chunk position cache"); }


#pragma region check players killed
	auto &clients = allClients;
	for (auto &c : clients)
	{
		//kill players
		if (c.second->playerData.newLife.life <= 0 && !c.second->playerData.killed)
		{
			c.second->playerData.kill();
			c.second->playerData.killed = true;

			//todo only for local players!
			genericBroadcastEntityKillFromServerToPlayer(c.first, true);
		}
		else
		{
			//per client life updates happens later
		}

	}
#pragma endregion



	if (profiler) { profiler->startSubProfile("Tasks"); }
#pragma region tasks
	{
		int count = waitingTasks.size();
		for (int taskIndex = 0; taskIndex < count; taskIndex++)
		{
			auto &i = waitingTasks[taskIndex];

			//todo make sure only tasks for existing clients come to here and if so remove any check here
			if (i.t.taskType == Task::placeBlockForce)
			{

				bool wasGenerated = 0;
				auto chunk = chunkCache.getChunkOrGetNull(divideChunk(i.t.pos.x), divideChunk(i.t.pos.z));
				int convertedX = modBlockToChunk(i.t.pos.x);
				int convertedZ = modBlockToChunk(i.t.pos.z);

				auto client = getClientNotLocked(i.cid);

				if (client)
				{

					if (!chunk || client->playerData.otherPlayerSettings.gameMode !=
						OtherPlayerSettings::CREATIVE)
					{
						//this chunk isn't in this region so undo or player isn't in creative

						computeRevisionStuff(*client, false, i.t.eventId);

					}
					else
					{
						auto b = chunk->chunk.safeGet(convertedX, i.t.pos.y, convertedZ);
						bool good = 0;

						if (b)
						{
							auto block = i.t.block;
							block.lightLevel = 0;

							if (isBlock(block.getType()) || block.getType() == 0)
							{
								good = true;
							}

							bool legal = computeRevisionStuff(*client, good, i.t.eventId);

							if (legal)
							{
								auto lastBlock = b->getType();
								chunk->removeBlockWithData({convertedX,
									i.t.pos.y, convertedZ}, lastBlock);
								*b = block;
								chunk->otherData.dirty = true;

								{
									Packet packet;
									packet.cid = i.cid;
									packet.header = headerPlaceBlocks;

									Packet_PlaceBlocks packetData;
									packetData.blockPos = i.t.pos;
									packetData.blockInfo = *b;

									//todo broadcast only to local players from now on
									broadCastNotLocked(packet, &packetData, sizeof(Packet_PlaceBlocks),
										client->peer, true, channelChunksAndBlocks);
								}

							}

						}

					}

				};

			}
			else
				if (i.t.taskType == Task::placeBlock
					|| i.t.taskType == Task::breakBlock
					)
				{

					auto chunk = chunkCache.getChunkOrGetNull(divideChunk(i.t.pos.x), divideChunk(i.t.pos.z));
					int convertedX = modBlockToChunk(i.t.pos.x);
					int convertedZ = modBlockToChunk(i.t.pos.z);

					auto client = getClientNotLocked(i.cid);
					
					if (client)
					{

						if (!chunk)
						{
							//this chunk isn't in this region so undo
							computeRevisionStuff(*client, false, i.t.eventId);
						}
						else
						{
							//todo check if place is legal

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
								actualPlacedBLock.setColor(0);

								if (!b)
								{
									legal = false;
								}
								else
								{

									if (i.t.taskType == Task::placeBlock)
									{
										item = client->playerData.inventory.getItemFromIndex(i.t.inventroySlot);



										if (item && item->isBlock() &&
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

										if (chunkCache.anyEntityIntersectsWithBlock(i.t.pos))
										{
											legal = false;
										}

									}

									//validate ladder placement
									if (i.t.taskType == Task::placeBlock 
										&&
										(actualPlacedBLock.isWallMountedBlock()
											|| (actualPlacedBLock.isWallMountedOrStangingBlock() && 
												actualPlacedBLock.getRotatedOrStandingForWallOrStandingBlocks())
										)
										)
									{

										int rotation = actualPlacedBLock.getRotationFor365RotationTypeBlocks();
										
										glm::ivec3 directions[4] = {
										glm::ivec3(0, 0, 1),
										glm::ivec3(1, 0, 0),
										glm::ivec3(0, 0, -1),
										glm::ivec3(-1, 0, 0)};

										auto direction = directions[rotation];
										auto wallPos = i.t.pos - direction;

										Block *wallBlock = chunkCache.getBlockSafe(wallPos);

										if (!wallBlock)
										{
											legal = false;
										}
										else
										{
											if (!wallBlock->canWallMountedBlocksBePlacedOn())
											{
												legal = false;
											}
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
										packetData.blockInfo = *b;

										//todo only for local players
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

											spawnDroppedItemEntity(chunkCache,
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

					};

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
								getOnlyIdFromEID(i.t.entityId) >= RESERVED_CLIENTS_ID)
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
								spawnDroppedItemEntity(chunkCache,
									worldSaver, i.t.blockCount, i.t.blockType, &from->metaData,
									i.t.doublePos, i.t.motionState, newId,
									computeRestantTimer(i.t.timer, getTimer()));

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
							}
							else
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
						bool shouldUpdateRevisionStuff = false;
						bool allowed = false;
						Item *from = client->playerData.inventory.getItemFromIndex(i.t.from);

						if (from)
						{


							if (from->isPaint())
							{
								shouldUpdateRevisionStuff = true;
							}

							//if the revision number isn't good we don't do anything
							if (client->playerData.inventory.revisionNumber
								== i.t.revisionNumber
								)
							{

								//serverTask.t.pos = packetData->position;


								if (from && !client->playerData.killed)
								{
									allowed = true;

									

									if (from->counter <= 0) { from = {}; }

									if (from->type == i.t.itemType)
								{

									

									if (from->type == ItemTypes::pigSpawnEgg)
									{
										Pig p;
										glm::dvec3 position = glm::dvec3(i.t.pos) + glm::dvec3(0.0, -0.49, 0.0);
										p.position = position;
										p.lastPosition = position;
										spawnPig(chunkCache, p, worldSaver, rng);
									}
									else if (from->type == ItemTypes::zombieSpawnEgg)
									{
										Zombie z;
										glm::dvec3 position = glm::dvec3(i.t.pos) + glm::dvec3(0.0, -0.49, 0.0);
										z.position = position;
										z.lastPosition = position;
										spawnZombie(chunkCache, z, getEntityIdAndIncrement(worldSaver,
											EntityType::zombies));
									}
									else if (from->type == ItemTypes::catSpawnEgg)
									{
										Cat c;
										glm::dvec3 position = glm::dvec3(i.t.pos) + glm::dvec3(0.0, -0.49, 0.0);
										c.position = position;
										c.lastPosition = position;
										spawnCat(chunkCache, c, worldSaver, rng);
									}
									else if (from->type == ItemTypes::goblinSpawnEgg)
									{
										Goblin g;
										glm::dvec3 position = glm::dvec3(i.t.pos) + glm::dvec3(0.0, -0.49, 0.0);
										g.position = position;
										g.lastPosition = position;
										spawnGoblin(chunkCache, g, worldSaver, rng);
									}
									else if (from->isEatable())
									{

										auto effects = getItemEffects(*from);
										int healing = getItemHealing(*from);

										//can't eat if satiety doesn't allow it
										if (effects.allEffects[Effects::Saturated].timerMs > 0 &&
											client->playerData.effects.allEffects[Effects::Saturated].timerMs > 0
											)
										{
											allowed = 0;
										}
										else
										{
											client->playerData.applyDamageOrLife(healing);
											client->playerData.effects.applyEffects(effects);
										}


									}
									else if (from->isPaint())
									{

										SavedChunk *c = 0;
										auto b = chunkCache.getBlockSafeAndChunk(i.t.pos, c);

										if (b && c && b->canBePainted())
										{
											allowed = true;


											shouldUpdateRevisionStuff = false;
											if (computeRevisionStuff(*client, allowed, i.t.eventId))
											{
												
												c->otherData.dirtyBlockData = true;
												c->otherData.dirty = true;

												//
												//todo only for local players
												{
													Packet packet;
													packet.cid = i.cid;
													packet.header = headerPlaceBlocks;
													int paintType = from->type - soap;
													b->setColor(paintType);

													Packet_PlaceBlocks packetData;
													packetData.blockPos = i.t.pos;
													packetData.blockInfo = *b;

													//todo only for local players
													broadCastNotLocked(packet, &packetData, sizeof(Packet_PlaceBlocks),
														client->peer, true, channelChunksAndBlocks);
												}
											}

										}
										else
										{
											allowed = false;
										}

									}
									else
									{
										//todo player used an item that "can't be used" hard reset here
									}

									if (
										allowed &&
										from->isConsumedAfterUse() && client->playerData.otherPlayerSettings.gameMode ==
										OtherPlayerSettings::SURVIVAL)
									{
										from->counter--;
										if (from->counter <= 0)
										{
											*from = {};
										}
									}

									if (!allowed)
									{
										sendPlayerInventoryAndIncrementRevision(*client);
									}
								}
									else
									{
										sendPlayerInventoryAndIncrementRevision(*client);
										allowed = false;
									}

								}
								else
								{
									sendPlayerInventoryAndIncrementRevision(*client);
								}

							};

						};

						if (shouldUpdateRevisionStuff)
						{
							computeRevisionStuff(*client, allowed, i.t.eventId);
						}

						//the client might have eaten something so we update life anyway,
						// the same for the effects
						client->playerData.forceUpdateLife = true;
						client->playerData.updateEffectsTicksTimer = 0;

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
								auto chunk = chunkCache.getChunkOrGetNull(divideChunk(i.t.pos.x), divideChunk(i.t.pos.z));
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
				else if (i.t.taskType == Task::clientAttackedEntity)
				{
					unsigned char itemInventoryIndex = i.t.inventroySlot;
					std::uint64_t entityId = i.t.entityId;
					glm::vec3 dir = i.t.vector;
					HitResult hitResult = i.t.hitResult;

					auto client = getClientNotLocked(i.cid);

					if (client)
					{
						if (!client->playerData.killed)
						{
							auto item = client->playerData.inventory.getItemFromIndex(itemInventoryIndex);
							if (item)
							{
								int type = getEntityTypeFromEID(entityId);
								bool doNotHit = 0;

								//we don't want to hit creative players
								if (type == EntityType::player)
								{
									auto found = allClients.find(entityId);

									if (found == allClients.end()) { doNotHit = true; }
									else
									{
										if (found->second->playerData.otherPlayerSettings.gameMode
											== OtherPlayerSettings::CREATIVE)
										{
											doNotHit = true;
										}
									}
								}

								if (!doNotHit)
								{
									std::uint64_t wasKilled = 0;
									bool rez = chunkCache.hitEntityByPlayer(entityId, client->playerData.getPosition(),
										*item, wasKilled, dir, rng, hitResult.hitCorectness, hitResult.bonusCritChance);

									//todo  we have separate logic for killing players and
									//	maybe do the same for entities?
									if (wasKilled)
									{

										//sd.chunkCache

										auto pos = chunkCache.getEntityPosition(wasKilled);

										if (pos)
										{
											glm::vec3 p = *pos;
											p.y += 0.5;
											spawnDroppedItemEntity(chunkCache,
												worldSaver, 1, ItemTypes::apple, 0, p, {}, {}, 0, 0);
										}
										else
										{
											std::cout << "ERROR gettint entity position!\n";
										}

										killEntity(worldSaver, wasKilled, chunkCache);
									}
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

							client->playerData.effects = {};
							client->playerData.newLife = PLAYER_DEFAULT_LIFE;
							client->playerData.lifeLastFrame = PLAYER_DEFAULT_LIFE;
							client->playerData.killed = false;
							sendPlayerInventoryAndIncrementRevision(*client);
							sendUpdateLifeLifePlayerPacket(*client);

							Packet packet;
							packet.cid = i.cid;
							packet.header = headerRespawnPlayer;

							Packet_RespawnPlayer packetData;
							packetData.pos = worldSaver.spawnPosition;

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

						client->playerData.applyDamageOrLife(-i.t.damage);

					}

				}
				else if (i.t.taskType == Task::clientRecievedDamageLocallyAndDied)
				{
					auto client = getClientNotLocked(i.cid);

					if (client && !client->playerData.killed)
					{
						client->playerData.kill();

						genericBroadcastEntityKillFromServerToPlayer(i.cid, true,
							client->peer);
					}

				}
				else if (i.t.taskType == Task::clientChangedBlockData)
				{
					auto client = getClientNotLocked(i.cid);

					auto chunk = chunkCache.getChunkOrGetNull(divideChunk(i.t.pos.x), divideChunk(i.t.pos.z));
					int convertedX = modBlockToChunk(i.t.pos.x);
					int convertedZ = modBlockToChunk(i.t.pos.z);


					if (client)
					{

						if (!chunk)
						{
							//this chunk isn't in this region so undo
							computeRevisionStuff(*client, false, i.t.eventId);
						}
						else
						{

							auto b = chunk->chunk.safeGet(convertedX, i.t.pos.y, convertedZ);

							if (b && b->getType() == i.t.blockType)
							{

								if (i.t.blockType == BlockTypes::structureBase)
								{

									BaseBlock baseBlock;
									size_t _ = 0;
									if (!baseBlock.readFromBuffer(i.t.metaData.data(),
										i.t.metaData.size(), _))
									{
										//notify undo revision
										computeRevisionStuff(*client, false, i.t.eventId);
									}
									else
									{
										if (!baseBlock.isDataValid())
										{
											//notify undo revision
											computeRevisionStuff(*client, false, i.t.eventId);
										}
										else
										{
											
											if (computeRevisionStuff(*client, true, i.t.eventId))
											{

												//update the data
												auto pos = i.t.pos;
												pos.x = modBlockToChunk(pos.x);
												pos.z = modBlockToChunk(pos.z);
												auto hashPos = fromBlockPosInChunkToHashValue(pos.x, pos.y, pos.z);

												chunk->blockData.baseBlocks[hashPos] = baseBlock;

												chunk->otherData.dirtyBlockData = true;

												//todo notify other players

												std::vector<unsigned char> packetData;
												Packet_ChangeBlockData packetChangeBlockData;
												packetChangeBlockData.blockDataHeader.blockType = BlockTypes::structureBase;
												packetChangeBlockData.blockDataHeader.pos = i.t.pos;

												packetData.resize(sizeof(packetChangeBlockData));

												packetChangeBlockData.blockDataHeader.dataSize = baseBlock.formatIntoData(packetData);

												memcpy(packetData.data(), &packetChangeBlockData, sizeof(packetChangeBlockData));

												Packet p;
												p.header = headerChangeBlockData;
												p.cid = i.cid;
												broadCastNotLocked(p, packetData.data(), packetData.size(),
													client->peer, true, channelChunksAndBlocks);

											}

										}

									}


								}
								else
								{
									//notify undo revision
									computeRevisionStuff(*client, false, i.t.eventId);
								}


							}
							else
							{
								//notify undo revision
								computeRevisionStuff(*client, false, i.t.eventId);
							}

						}

					}

				}


		}
	}
#pragma endregion
	if (profiler) { profiler->endSubProfile("Tasks"); }


	if (profiler) { profiler->startSubProfile("Player stuff"); }

#pragma region player effects

	//todo this should be for all entities
	for (auto &c : allPlayers)
	{
		if (!c.second->killed)
		{
			c.second->effects.passTimeMs(deltaTimeMs);

			auto &effectsTimers = c.second->effectsTimers;


			//regen and others come here
			if (c.second->effects.allEffects[Effects::Regeneration].timerMs > 0)
			{

				effectsTimers.regen -= deltaTime;

				if (effectsTimers.regen < 0)
				{
					effectsTimers.regen += 1; //heal once every seccond;
					c.second->newLife.life += 5;
					c.second->newLife.sanitize();
				}

			}


		}
	}


#pragma endregion


#pragma region calculate player healing

	//TODO do a last frame life kinda stuff for players to make things easier

	for (auto &c : allSurvivalClients)
	{
		auto &playerData = c.second->playerData;

		if (playerData.killed) { continue; }

		if (playerData.healingDelayCounterSecconds >= playerData.calculateHealingDelayTime())
		{
			if (playerData.newLife.life < playerData.newLife.maxLife)
			{
				playerData.notIncreasedLifeSinceTimeSecconds += deltaTime;

				if (playerData.notIncreasedLifeSinceTimeSecconds > playerData.calculateHealingRegenTime())
				{
					playerData.notIncreasedLifeSinceTimeSecconds -= playerData.calculateHealingRegenTime();

					playerData.newLife.life++;
					playerData.newLife.sanitize();

				}
			}
			else
			{
				playerData.notIncreasedLifeSinceTimeSecconds = 0;
			}
		}
		else
		{
			playerData.healingDelayCounterSecconds += deltaTime;
			playerData.notIncreasedLifeSinceTimeSecconds = 0;
		}


	}


#pragma endregion
	if (profiler) { profiler->endSubProfile("Player stuff"); }

	if (profiler) { profiler->startSubProfile("Random Tick Update"); }
#pragma region to random tick update

	unsigned int randomTickSpeed = getRandomTickSpeed();

	//todo server spamming the client problem
	for (auto &c : chunkCache.savedChunks)
	{
		if (!c.second->otherData.withinSimulationDistance) { continue; }

		for (int h = 0; h < CHUNK_HEIGHT / 16; h++)
		{
			for (int i = 0; i < randomTickSpeed; i++)
			{
				int x = rng() % 16; //todo better rng here
				int y = rng() % 16;
				int z = rng() % 16;
				y += h * 16;

				//if the code crashes here, that means that we wrongly got a nullptr chunk!
				auto &b = c.second->chunk.unsafeGet(x,y,z);
				auto type = b.getType();

				//todo add yellow grass
				if (type == BlockTypes::dirt)
				{

					auto top = c.second->chunk.safeGet(x, y + 1, z);
					if (top && top->stopsGrassFromGrowingIfOnTop())
					{
						//don't try to update block
					}
					else
					{
						auto tryBlock = [&](int i, int j, int k)
						{
							auto b = chunkCache
								.getBlockSafe({x + c.first.x * CHUNK_SIZE + i,
								y + j, z + c.first.y * CHUNK_SIZE + k});

							if (b && b->getType() == grassBlock)
							{
								return true;
							}
							return false;
						};

						if (tryBlock(1, 0, 0) ||
							tryBlock(-1, 0, 0) ||
							tryBlock(0, 0, 1) ||
							tryBlock(0, 0, -1) ||
							tryBlock(1, 0, 1) ||
							tryBlock(-1, 0, 1) ||
							tryBlock(1, 0, -1) ||
							tryBlock(-1, 0, -1) ||

							tryBlock(1, -1, 0) ||
							tryBlock(-1, -1, 0) ||
							tryBlock(0, -1, 1) ||
							tryBlock(0, -1, -1) ||
							tryBlock(1, -1, 1) ||
							tryBlock(-1, -1, 1) ||
							tryBlock(1, -1, -1) ||
							tryBlock(-1, -1, -1) ||

							tryBlock(1, 1, 0) ||
							tryBlock(-1, 1, 0) ||
							tryBlock(0, 1, 1) ||
							tryBlock(0, 1, -1) ||
							tryBlock(1, 1, 1) ||
							tryBlock(-1,1, 1) ||
							tryBlock(1, 1, -1) ||
							tryBlock(-1, 1, -1)
							)
						{
							//update block
							b.setType(BlockTypes::grassBlock);
							modifiedBlocks[{x + c.first.x * CHUNK_SIZE, y, z + c.first.y * CHUNK_SIZE}] = b;
							c.second->otherData.dirty = true;
						}

					}

				}
				else if (type == BlockTypes::grassBlock)
				{
					auto top = c.second->chunk.safeGet(x, y + 1, z);
					if (top && top->stopsGrassFromGrowingIfOnTop())
					{
						//update block
						b.setType(BlockTypes::dirt);
						modifiedBlocks[{x + c.first.x * CHUNK_SIZE, y, z + c.first.y * CHUNK_SIZE}] = b;
						c.second->otherData.dirty = true;
					}

				}
				else if (type == BlockTypes::yellowGrass)
				{
					auto top = c.second->chunk.safeGet(x, y + 1, z);
					if (top && top->stopsGrassFromGrowingIfOnTop())
					{
						//update block
						b.setType(BlockTypes::dirt);
						modifiedBlocks[{x + c.first.x * CHUNK_SIZE, y, z + c.first.y * CHUNK_SIZE}] = b;
						c.second->otherData.dirty = true;
					}

				}
			}
		}
	};



#pragma endregion
	if (profiler) { profiler->endSubProfile("Random Tick Update"); }


	if (profiler) { profiler->startSubProfile("Path Finding"); }
#pragma region calculate player positions
	std::unordered_map<std::uint64_t, glm::dvec3> playersPositionSurvival;

	for (auto &p : allSurvivalClients)
	{
		playersPositionSurvival.insert({p.first, p.second->playerData.getPosition()});
	}



	//for (auto &c : chunkCache.savedChunks)
	//{
	//	for (auto &p : c.second->entityData.players)
	//	{
	//		playersPositionSurvival.insert({p.first, p.second->getPosition()});
	//	}
	//}
#pragma endregion


#pragma region calculate path finding


	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> pathFindingSurvivalClients;

	{
		std::deque<PathFindingNode> queue;
		std::unordered_map<glm::ivec3, PathFindingNode> positions;
		
		auto addNode = [&](PathFindingNode node, glm::ivec3 displacement)
		{
			PathFindingNode newEntry;
			newEntry.returnPos = node.returnPos;
			newEntry.level = node.level + 1;

			positions[node.returnPos + displacement] = newEntry;

			if (node.level < 40)
			{
				newEntry.returnPos = node.returnPos + displacement;
				queue.push_back(newEntry);
			}
		};

		auto checkDown = [&](PathFindingNode node, glm::ivec3 disp) //-> bool
		{
			glm::ivec3 displacement = glm::ivec3(0, -1, 0) + disp;

			auto found = positions.find(node.returnPos + displacement);
			if (found == positions.end())
			{
				auto b = chunkCache.getBlockSafe(node.returnPos + displacement);
				if (b && !b->isColidable())
				{
					auto b2 = chunkCache.getBlockSafe(node.returnPos + displacement + glm::ivec3(0, -1, 0));
					if (b2 && b2->isColidable())
					{
						addNode(node, displacement);
					}
				}
				//else
				//{
				//	return true;
				//}
			}

			//return false;
		};

		auto checkSides = [&](PathFindingNode node, glm::ivec3 displacement)
		{
			auto found = positions.find(node.returnPos + displacement);
			if (found == positions.end())
			{
				auto b = chunkCache.getBlockSafe(node.returnPos + displacement);
				if (b && !b->isColidable())
				{

					auto bUp = chunkCache.getBlockSafe(node.returnPos + displacement + glm::ivec3(0, 1, 0));
					if (!bUp || !bUp->isColidable())
					{
						auto bDown = chunkCache.getBlockSafe(node.returnPos + displacement + glm::ivec3(0, -1, 0));
						auto bDown2 = chunkCache.getBlockSafe(node.returnPos + displacement + glm::ivec3(0, -2, 0));

						if ((bDown && bDown->isColidable())
							|| (bDown2 && bDown2->isColidable())
							)
						{
							addNode(node, displacement);

							if((!bDown || !bDown->isColidable()) && bDown2 && bDown2->isColidable())
							{
								checkDown(node, displacement);
							}
						}

					}

				}
			}
		};

		auto checkUp = [&](PathFindingNode node, glm::ivec3 displacement)
		{
			auto found = positions.find(node.returnPos + displacement);
			if (found == positions.end())
			{
				auto b = chunkCache.getBlockSafe(node.returnPos + displacement);
				if (b && !b->isColidable())
				{
					addNode(node, displacement);
				}
			}
		};


		//if(0)
		for(auto &player : playersPositionSurvival)
		{
			queue.clear();
			positions.clear();
			
			glm::ivec3 pos = from3DPointToBlock(player.second);

			//project players position down down
			for(int i=1; i<4; i++)
			{

				auto b = chunkCache.getBlockSafe(pos - glm::ivec3(0,i,0));

				if (!b) { break; }

				if (b->isColidable())
				{
					PathFindingNode root;
					root.returnPos = pos - glm::ivec3(0, i-1, 0);
					root.level = 0;

					queue.push_back(root);
					positions[pos - glm::ivec3(0, i-1, 0)] = root;
					break;
				}
			}

			while (!queue.empty())
			{
				PathFindingNode node = queue.front();
				queue.pop_front();

				checkSides(node, {1,0,0});
				checkSides(node, {-1,0,0});
				checkSides(node, {0,0,1});
				checkSides(node, {0,0,-1});

				//checkDown(node, {});

				auto bDown = chunkCache.getBlockSafe(node.returnPos + glm::ivec3(0,-1,0));
				if (bDown && bDown->isColidable())
				{
					checkUp(node, {0,1,0});
					checkUp(node, {0,2,0});
					checkUp(node, {0,3,0});
					checkUp(node, {0,4,0});
					//checkUp(node, {0,5,0});
				}

			}

			pathFindingSurvivalClients[player.first] = std::move(positions);
		}
	
	};


#pragma endregion
	if (profiler) { profiler->endSubProfile("Path Finding"); }


	if (profiler) { profiler->startSubProfile("Entity Updates"); }
#pragma region mark entities as not updated

	for (auto &c : chunkCache.savedChunks)
	{
		if (!c.second->otherData.withinSimulationDistance) { continue; }
		if (c.second->otherData.shouldUnload) { continue; }

		auto &entityData = c.second->entityData;

		auto genericMarkEntitiesNotUpdated = [&](auto &container)
		{
			if constexpr (std::is_same<std::remove_reference_t<decltype(container[0])>, PlayerServer *>::value)
			{
				//don't iterate over players
				return;
			}
			else
			{
				for (auto &e :container )
				{
					e.second.hasUpdatedThisTick = 0;
				}
			}
		};

		REPEAT_FOR_ALL_ENTITIES(ENTITY_MARK_NOTPDATED);
	}

#pragma endregion

#pragma region entity updates

	std::unordered_set<std::uint64_t> othersDeleted;

	for (auto &c : chunkCache.savedChunks)
	{	
		if (!c.second->otherData.withinSimulationDistance) { continue; }
		if (c.second->otherData.shouldUnload) { continue; }

		auto &entityData = c.second->entityData;

		auto initialChunk = c.first;

		auto genericLoopOverEntities = [&](auto &container, 
			auto &orphanContainer, auto memberSelector
			)
		{

			if constexpr (std::is_same<std::remove_reference_t<decltype(container[0])>, PlayerServer*>::value)
			{
				//don't iterate over players
				return;
			}
			else
			{
				for (auto it = container.begin(); it != container.end(); )
				{
					auto &e = *it;

					if (e.second.hasUpdatedThisTick) { ++it; continue; }
					e.second.hasUpdatedThisTick = true;
					
					bool rez = genericCallUpdateForEntity(e, deltaTime, chunkGetter,
						chunkCache, rng, othersDeleted,
						pathFindingSurvivalClients, playersPositionSurvival, allClients);
					glm::ivec2 newChunk = determineChunkThatIsEntityIn(e.second.getPosition());

					if (!rez)
					{
						//todo only for local players!!!!!!
						genericBroadcastEntityDeleteFromServerToPlayer(it->first,
							true, allClients, e.second.lastChunkPositionWhenAnUpdateWasSent);

						//std::cout << "remove!!!!!!!\n";
						//remove entity
						it = container.erase(it);
					}
					else
					{
						//todo this should take into acount if that player should recieve it
						//todo only for local players!!!!!!
						//genericBroadcastEntityUpdateFromServerToPlayer
						//	< decltype(packetType)>(e, false, currentTimer, packetId);
						//std::cout << "Sent update ";
						genericBroadcastEntityUpdateFromServerToPlayer2(e, false, getTimer());

						if (initialChunk != newChunk)
						{
							//std::cout << "Prepare to move\n";
							auto chunk = chunkCache.getChunkOrGetNull(newChunk.x, newChunk.y);
							
							if (chunk)
							{
								//std::cout << "Found!\n";

								//move entity in another chunk
								auto member = memberSelector(chunk->entityData);
								member->insert({e.first, e.second});
								chunkCache.entityChunkPositions[e.first] = newChunk;

							}
							else
							{
								//std::cout << "Not Found!\n";

								//the entity left the region, we move it out,
								// so we save it to disk or to other chunks

								auto found = chunkCache.entityChunkPositions.find(e.first);
								if (found != chunkCache.entityChunkPositions.end())
								{
									chunkCache.entityChunkPositions.erase(found);
								}

								orphanContainer.insert(
									{e.first, e.second});
							}


							it = container.erase(it);
						}
						else
						{
							++it;
						}
					}

				}
			}

			
		};

		
		REPEAT_FOR_ALL_ENTITIES(ENTITY_UPDATES);
	


	}

	for (auto eid : othersDeleted)
	{
		//TODO!! set the position and last chunk position corectly
		genericBroadcastEntityDeleteFromServerToPlayer(eid, true, allClients, {});

	}
	
	//todo this is probably not usefull anymore but investigate.
	callGenericResetEntitiesInTheirNewChunk(std::make_integer_sequence<int, EntitiesTypesCount - 1>(),
		orphanEntities, chunkCache);


#pragma endregion
	if (profiler) { profiler->endSubProfile("Entity Updates"); }

	if (profiler) { profiler->startSubProfile("Send Block health and entity Packets"); }
#pragma region send packets


	//send new blocks
	//TODO CHANGE TO BLOCK rather than blockType
	if (!modifiedBlocks.empty())
	{

		//todo ring buffer
		Packet_PlaceBlocks *newBlocks = new Packet_PlaceBlocks[modifiedBlocks.size()];

		Packet packet;
		packet.cid = 0;
		packet.header = headerPlaceBlocks;

		int i = 0;
		for (auto &b : modifiedBlocks)
		{
			newBlocks[i].blockPos = b.first;
			newBlocks[i].blockInfo = b.second;
			i++;
		}

		for (auto it = allClients.begin(); it != allClients.end(); it++)
		{
			{
				sendPacket(it->second->peer, packet, (const char *)newBlocks, 
					sizeof(Packet_PlaceBlocks) *modifiedBlocks.size(), true, channelChunksAndBlocks);
			}
		}


		delete[] newBlocks;
	}



#pragma endregion

#pragma region send health packets and effects packets

	for (auto &c : allClients)
	{
		
		c.second->playerData.updateEffectsTicksTimer--;

		//we re-update the effects every 20 tick, to make sure things stay in sync
		if (c.second->playerData.updateEffectsTicksTimer <= 0)
		{
			c.second->playerData.updateEffectsTicksTimer = 20;

			updatePlayerEffects(*c.second);
		}

		if (c.second->playerData.newLife.life <= 0 || c.second->playerData.killed)
		{
			//the kill message will be send outside this thread	
		}
		else
		{

			c.second->playerData.newLife.sanitize();

			if (c.second->playerData.newLife.life != c.second->playerData.lifeLastFrame.life
				||
				c.second->playerData.newLife.maxLife != c.second->playerData.lifeLastFrame.maxLife
				|| c.second->playerData.forceUpdateLife
				)
			{
				c.second->playerData.forceUpdateLife = 0;

				if (c.second->playerData.newLife.life < c.second->playerData.lifeLastFrame.life)
				{
					sendDamagePlayerPacket(*c.second);
					c.second->playerData.healingDelayCounterSecconds = 0;
				}
				else if (c.second->playerData.newLife.life > c.second->playerData.lifeLastFrame.life)
				{
					sendIncreaseLifePlayerPacket(*c.second);
				}
				else
				{
					sendUpdateLifeLifePlayerPacket(*c.second);
				}

				c.second->playerData.lifeLastFrame = c.second->playerData.newLife;
			}

		}

	}

#pragma endregion

#pragma region server send entity position data
	{
		static thread_local float sendEntityTimer = 0;
		sendEntityTimer -= deltaTime;

		if (sendEntityTimer < 0)
		{
			sendEntityTimer = 0.4;

			//todo make a different timer for each player			
			//todo maybe merge things into one packet

			//no need for mutex because this thread modifies the clients data

			for (auto &c : allClients)
			{

				// send players

				for (auto &other : allClients)
				{
					if (other.first != c.first)
					{

						auto &loadedChunks = c.second->loadedChunks;
						
						glm::ivec2 lastChunkPos = other.second->playerData.lastChunkPositionWhenAnUpdateWasSent;
						glm::ivec2 currentChunkPos = {};
						currentChunkPos.x = divideChunk(other.second->playerData.getPosition().x);
						currentChunkPos.y = divideChunk(other.second->playerData.getPosition().z);

						//todo only players that have entities in the simulated region later
						if(loadedChunks.find(lastChunkPos) != loadedChunks.end()
							||
							loadedChunks.find(currentChunkPos) != loadedChunks.end()
							)
						//if (checkIfPlayerShouldGetEntity(
						//	{c.second.playerData.entity.position.x, c.second.playerData.entity.position.z},
						//	other.second.playerData.entity.position, c.second.playerData.entity.chunkDistance, 0)
						//	)
						{
							Packet_ClientRecieveOtherPlayerPosition sendData;
							sendData.eid = other.first;
							sendData.timer = getTimer();
							sendData.entity = other.second->playerData.entity;

							Packet p;
							p.cid = 0;
							p.header = headerClientRecieveOtherPlayerPosition;

							sendPacket(c.second->peer, p, (const char *)&sendData, sizeof(sendData),
								false, channelPlayerPositions);
						}

						other.second->playerData.lastChunkPositionWhenAnUpdateWasSent = currentChunkPos;


					}
				}


			}

		}



	}

#pragma endregion


	if (profiler) { profiler->startSubProfile("Send Block health end entity Packets"); }


	if (profiler) { profiler->endFrame(); }


	chunkCacheGlobal = 0;
}





void sendDamagePlayerPacket(Client &client)
{
	Packet_UpdateLife p;
	p.life = client.playerData.newLife;
	sendPacket(client.peer, headerRecieveDamage, &p, sizeof(p), true, channelChunksAndBlocks);
}

void sendIncreaseLifePlayerPacket(Client &client)
{
	Packet_UpdateLife p;
	p.life = client.playerData.newLife;
	sendPacket(client.peer, headerRecieveLife, &p, sizeof(p), true, channelChunksAndBlocks);
}

//sets the life of the player with no animations
void sendUpdateLifeLifePlayerPacket(Client &client)
{
	Packet_UpdateLife p;
	p.life = client.playerData.newLife;
	sendPacket(client.peer, headerUpdateLife, &p, sizeof(p), true, channelChunksAndBlocks);
}

