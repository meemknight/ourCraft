#include "multyPlayer/createConnection.h"
#include "enet/enet.h"
#include "multyPlayer/packet.h"
#include <iostream>
#include <multyPlayer/enetServerFunction.h>
#include <gameplay/entityManagerClient.h>
#include <multyPlayer/undoQueue.h>
#include <errorReporting.h>
#include <platformTools.h>
#include <gameplay/items.h>
#include <audioEngine.h>
#include <gameplay/blocks/blocksWithData.h>
#include <lightSystem.h>

static ConnectionData clientData;

//todo this will get removed, for some things at least
void submitTaskClient(Task &t)
{
	auto data = getConnectionData();

	Packet p;
	p.cid = data.cid;

	switch (t.taskType)
	{
		
		case Task::placeBlock:
		{
			//todo remove
			permaAssert(0);
			break;
		}
		default:

			permaAssert(0);
		break;
	}


}

void submitTaskClient(std::vector<Task> &t)
{
	//todo can be merged into less requests.

	for (auto &i : t)
	{
		submitTaskClient(i);
	}
}

Packet formatPacket(int header)
{
	Packet p;
	p.cid = clientData.cid;
	p.header = header;
	return p;
}

ENetPeer *getServer()
{
	return clientData.server;
}


ConnectionData getConnectionData()
{
	return clientData;
}

#define CASE_UPDATE(I) case I: { \
if (size != sizeof(Packet_UpdateGenericEntity) + sizeof(decltype((*entityManager.entityGetter<I>())[0].entityBuffered))) { break; } \
auto *p = (decltype((*entityManager.entityGetter<I>())[0].entityBuffered) *)(data + sizeof(Packet_UpdateGenericEntity)); \
float restantTimer = computeRestantTimer(firstPart->timer, serverTimer); \
entityManager.addOrUpdateGenericEntity< I >(firstPart->eid, *p, undoQueue, restantTimer, serverTimer, firstPart->timer);\
} break;

void recieveDataClient(ENetEvent &event, 
	EventCounter &validatedEvent, 
	RevisionNumber &invalidateRevision,
	glm::ivec3 playerPosition, int squareDistance,
	ClientEntityManager &entityManager,
	UndoQueue &undoQueue, ChunkSystem &chunkSystem, 
	LightSystem &lightSystem,
	std::uint64_t &serverTimer,
	unsigned char revisionNumberBlockInteraction,
	bool &shouldExitBlockInteraction, bool &killedPlayer, bool &respawn,
	std::deque<std::string> &chat, float &chatTimer,
	InteractionData &playerInteraction,
	std::unordered_map<std::uint64_t, PlayerConnectionData> &playersConnectionData
	)
{
	Packet p;
	size_t size = 0;
	auto data = parsePacket(event, p, size);

	bool wasCompressed = 0;

	if(p.isCompressed())
	{
		//std::cout << "Decompressing\n";
		size_t newSize = {};
		auto deCompressedData = unCompressData(data, size, newSize);
		
		if (deCompressedData)
		{
			wasCompressed = true;
			data = (char*)deCompressedData;
			size = newSize;
		}
		else
		{
			//todo hard error request a hard reset.
			permaAssertComment(0, "decompression failed");
		}

		p.setNotCompressed();
	}

	switch(p.header)
	{
		case headerRecieveChunk:
		{

			Packet_RecieveChunk *chunkPacket = (Packet_RecieveChunk *)data;

			if (size != sizeof(Packet_RecieveChunk))
			{
				std::cout << "Size error in recieve chunk!!!!!!!!!!!!!!!!!!! " << size << "\n";
				break;
			}


			if (chunkSystem.isChunkInRadiusAndBounds({playerPosition.x, playerPosition.z},
				{chunkPacket->chunk.x, chunkPacket->chunk.z}))
			{
				

				{
		

					int x = chunkPacket->chunk.x - chunkSystem.cornerPos.x;
					int z = chunkPacket->chunk.z - chunkSystem.cornerPos.y;

					//if (x < 0 || z < 0 || x >= chunkSystem.squareSize || z >= chunkSystem.squareSize
					//	|| !checkChunkInRadius({x,z})
					//	)
					//{
					//	delete i; // ignore chunk, not of use anymore
					//	continue;
					//}
					//else
					{
						if (chunkSystem.loadedChunks[x * chunkSystem.squareSize + z] 
							!= nullptr)
						{
									//we already have the chunk, ignore it
						}
						else
						{
							//Chunk *c = new Chunk();
							//c->data = chunkPacket->chunk;

							if (dontUpdateLightSystem)
							{
								chunkSystem.shouldUpdateLights = true;
							}


							//permaAssert(loadedChunks[x * squareSize + z] == nullptr); //no need for assert we check the if above 
							
							chunkSystem.loadedChunks[x * chunkSystem.squareSize + z] = new Chunk();
							chunkSystem.loadedChunks[x * chunkSystem.squareSize + z]->data = chunkPacket->chunk;
							chunkSystem.loadedChunks[x * chunkSystem.squareSize + z]->createGpuData();

							Chunk *chunk = chunkSystem.loadedChunks[x * chunkSystem.squareSize + z];

							int xBegin = chunkPacket->chunk.x * CHUNK_SIZE;
							int zBegin = chunkPacket->chunk.z * CHUNK_SIZE;

							if (!dontUpdateLightSystem)
							{
								chunk->setDontDrawYet(true);

								//c is in chunk system coordonates space, not chunk space!
								chunkSystem.chunksToAddLight.push_back({x, z});

								for (int x = 0; x < CHUNK_SIZE; x++)
									for (int z = 0; z < CHUNK_SIZE; z++)
										for (int y = 0; y < CHUNK_HEIGHT; y++)
										{

											auto &b = chunk->unsafeGet(x, y, z);

											if (isLightEmitor(b.getType()))
											{
												lightSystem.addLight(chunkSystem,
													{chunk->data.x * CHUNK_SIZE + x, y, chunk->data.z * CHUNK_SIZE + z},
													15);

												chunkSystem.shouldUpdateLights = true;
											}

										}

							}

						}

					}



				}

			}
			else
			{
				//we reject the chunk but we have to notify the server

				Packet p = {};
				p.cid = getConnectionData().cid;
				p.header = headerClientDroppedChunk; 

				Packet_ClientDroppedChunk packetData = {};
				packetData.chunkPos.x = chunkPacket->chunk.x;
				packetData.chunkPos.y = chunkPacket->chunk.z;

				sendPacket(getConnectionData().server,
					p, (char *)&packetData, sizeof(packetData), 1,
					channelPlayerPositions);
			}
			

			break;
		}

		case headerRecieveEntireBlockDataForChunk:
		{
			//todo request hard reset
			if (size == 0) { break; }
			
			bool firstTime = 1;
				
			size_t pointer = 0;
			while (size - pointer > 0)
			{
				if (size - pointer < sizeof(BlockDataHeader)) { break; } //todo request hard reset here

				BlockDataHeader blockHeader = {};
				memcpy(&blockHeader, data + pointer, sizeof(BlockDataHeader));
				pointer += sizeof(blockHeader);

				//check if corupted data
				if(blockHeader.pos.y < 0 || blockHeader.pos.y >= CHUNK_HEIGHT) { break; } //todo request hard reset here

				if (blockHeader.blockType == BlockTypes::structureBase)
				{

					Chunk *chunk = 0;
					auto b = chunkSystem.getBlockSafeAndChunk(blockHeader.pos.x, blockHeader.pos.y, blockHeader.pos.z, chunk);

					if (!b) { break; }

					if (b->getType() != BlockTypes::structureBase)
					{
						//todo request hard reset
					}
					else
					{
						if (firstTime)
						{
							firstTime = 0;
							chunk->blockData = {}; //clear the block data for this chunk
						}

						glm::ivec3 posInChunk = blockHeader.pos;
						posInChunk.x = modBlockToChunk(posInChunk.x);
						posInChunk.z = modBlockToChunk(posInChunk.z);

						auto blockHash = fromBlockPosInChunkToHashValue(posInChunk.x, posInChunk.y, posInChunk.z);


						if (blockHeader.dataSize)
						{
							BaseBlock baseBlock;
							size_t outSize = 0;
							if (!baseBlock.readFromBuffer((unsigned char*)data + pointer, blockHeader.dataSize, outSize))
							{
								//hard reset
							}
							else
							{
								pointer += outSize;

								if (baseBlock.isDataValid())
								{
									chunk->blockData.baseBlocks[blockHash] = baseBlock;
								}
								else
								{
									//todo hardReset
								}

							}

						}
						else
						{
							chunk->blockData.baseBlocks[blockHash] = {};
						}
					}

					

				}
				else
				{
					{ break; } //todo request hard reset here
				}

				
			}


			break;
		}

		case headerChangeBlockData:
		{
			//todo request hard reset
			if (size < sizeof(Packet_ChangeBlockData)) { break; }

			Packet_ChangeBlockData *changeBlockData = (Packet_ChangeBlockData *)data;

			//check if corupted data
			if (changeBlockData->blockDataHeader.pos.y < 0 || changeBlockData->blockDataHeader.pos.y >= CHUNK_HEIGHT) { break; } //todo request hard reset here

			if (changeBlockData->blockDataHeader.blockType == BlockTypes::structureBase)
			{

				Chunk *chunk = 0;
				auto b = chunkSystem.getBlockSafeAndChunk(changeBlockData->blockDataHeader.pos.x, changeBlockData->blockDataHeader.pos.y, changeBlockData->blockDataHeader.pos.z, chunk);
			
				if (b)
				{

					BaseBlock baseBlock;
					size_t _ = 0;
					if (!baseBlock.readFromBuffer((unsigned char *)data + sizeof(Packet_ChangeBlockData),
						size - sizeof(Packet_ChangeBlockData), _))
					{
						//todo hard reset
					}
					else
					{
						if(!baseBlock.isDataValid())
						{
							//todo hard reset
						}
						else
						{
							glm::ivec3 posInChunk = changeBlockData->blockDataHeader.pos;
							posInChunk.x = modBlockToChunk(posInChunk.x);
							posInChunk.z = modBlockToChunk(posInChunk.z);

							auto blockHash = fromBlockPosInChunkToHashValue(posInChunk.x, posInChunk.y, posInChunk.z);

							chunk->blockData.baseBlocks[blockHash] = baseBlock;
						}
					}

				}
				else
				{
					//todo request hard reset
				}

			}
			else
			{
				//todo request hard reset
			}

			break;
		}

		case headerPlaceBlock:
		{
			//chunkSystem.placeBlockNoClient
			Packet_PlaceBlocks b = *(Packet_PlaceBlocks *)data;

			Block bl;
			bl.typeAndFlags = b.blockType;

			chunkSystem.placeBlockByServerAndRemoveFromUndoQueue(b.blockPos, bl, lightSystem,
				playerInteraction, undoQueue);

			break;
		}

		case headerPlaceBlocks:
		{
			//std::cout << "headerPlaceBlocks ! ";
			for (int i = 0; i < size / sizeof(Packet_PlaceBlocks); i++)
			{

				Packet_PlaceBlocks b = ((Packet_PlaceBlocks *)data)[i];

				Block bl;
				bl.typeAndFlags = b.blockType;

				chunkSystem.placeBlockByServerAndRemoveFromUndoQueue(b.blockPos, bl, lightSystem,
					playerInteraction, undoQueue);

			}
			//std::cout << "Placed blocks..." << size / sizeof(Packet_PlaceBlocks) << "\n";

			break;
		}

		case headerValidateEvent:
		{
			validatedEvent = std::max(validatedEvent, ((Packet_ValidateEvent *)data)->eventId.counter);
			break;
		}

		case headerValidateEventAndChangeID:
		{
			Packet_ValidateEventAndChangeId &p = *(Packet_ValidateEventAndChangeId *)data;

			auto found = entityManager.droppedItems.find(p.oldId);

			if (found != entityManager.droppedItems.end())
			{
				auto entity = found->second;
				entityManager.droppedItems.erase(found);
				entityManager.droppedItems.insert({p.newId, entity});
			}

			validatedEvent = std::max(validatedEvent, p.eventId.counter);
			break;
		}

		case headerInValidateEvent:
		{
			invalidateRevision = std::max(invalidateRevision, ((Packet_InValidateEvent *)data)->eventId.revision);
			break;
		}

		case headerClientRecieveOtherPlayerPosition:
		{

			Packet_ClientRecieveOtherPlayerPosition *entity =
				(Packet_ClientRecieveOtherPlayerPosition *)data;

			//todo add the timer
			//if (p->timer + 16 < yourTimer)
			//{
			//	break; //drop too old packets
			//}

			if (entityManager.localPlayer.entityId == entity->eid)
			{
				//update local player
				entityManager.localPlayer.entity = entity->entity;
				std::cout << "YESSSSSSSSSSSSSSSSSSSSSSSSSSSS LMAO " << entity->entity.chunkDistance << "\n";
			}
			else
			{

				if (chunkSystem.shouldRecieveEntity(entity->entity.position))
				{

					auto found = entityManager.players.find(entity->eid);
					
					if (found == entityManager.players.end())
					{
						entityManager.players[entity->eid] = {};
						found = entityManager.players.find(entity->eid);
						float restantTimer = computeRestantTimer(entity->timer, serverTimer);
						found->second.restantTime = restantTimer;
						found->second.entityBuffered = entity->entity;


						//we have a new player potentially
						auto connection = playersConnectionData.find(entity->eid);
						if (connection == playersConnectionData.end())
						{
							playersConnectionData[entity->eid] = {};
						}


					}
					else
					{
						//entityManager.players[entity->eid].entityBuffered = entity->entity;
						//float restantTimer = computeRestantTimer(entity->timer, serverTimer);
						//found->second.restantTime = restantTimer;
						//found->second.rubberBand
						//	.addToRubberBand(found->second.entityBuffered.position - entity->entity.position);

						found->second.bufferedEntityData.addElement(entity->entity, serverTimer, entity->timer);
					}
					//float restantTimer = computeRestantTimer(entity->timer, serverTimer);

					//if (restantTimer > 0)
					//{
					//	found->second.oldPositionForRubberBand = found->second.entity.position;
					//}
					//else
					//{
					//	found->second.rubberBand
					//	.addToRubberBand(found->second.entity.position - entity->entity.position);
					//}
					//found->second.rubberBand
					//	.addToRubberBand(found->second.entityBuffered.position - entity->entity.position);

					//found->second.bufferedEntityData.addElement(entity->entity, serverTimer);

					
					//found->second.restantTime = restantTimer;

				}
				else
				{
					//dropped recieved entity
				}
				


			}

			break;
		}

		case headerUpdateGenericEntity:
		{

			Packet_UpdateGenericEntity *firstPart = (Packet_UpdateGenericEntity *)data;
			if (size < sizeof(Packet_UpdateGenericEntity)) { break; }

			auto entityType = getEntityTypeFromEID(firstPart->eid);

			//todo we shouldn'd do this if we haven't recieved updates recently....
			//if (firstPart->timer + 16 < serverTimer)
			//{
			//	break; //drop too old packets
			//}

			switch (entityType)
			{

				REPEAT_FOR_ALL_ENTITIES(CASE_UPDATE);

			}




			break;
		}

		case headerClientUpdateTimer:
		{
			Packet_ClientUpdateTimer *p = (Packet_ClientUpdateTimer *)data;
			if (sizeof(Packet_ClientUpdateTimer) != size) { break; } //todo logs or something

			serverTimer = p->timer;
			break;
		}

		case headerRemoveEntity:
		{
			if (sizeof(Packet_RemoveEntity) != size) { break; } //todo logs or something

			Packet_RemoveEntity *p = (Packet_RemoveEntity *)data;

			entityManager.removeEntity(p->EID);

			break;
		}

		case headerKillEntity:
		{
			if (sizeof(Packet_KillEntity) != size) { break; } //todo logs or something
			Packet_KillEntity *p = (Packet_KillEntity *)data;

			if (p->EID == entityManager.localPlayer.entityId)
			{
				killedPlayer = true;
			}
			else
			{
				entityManager.killEntity(p->EID);
			}

			break;
		}

		//disconnect other player
		case headerDisconnectOtherPlayer:
		{
			Packet_DisconectOtherPlayer *eid = (Packet_DisconectOtherPlayer *)data;

			if (size == sizeof(Packet_DisconectOtherPlayer))
			{
				entityManager.removePlayer(eid->EID);

				auto found = playersConnectionData.find(eid->EID);

				if (found != playersConnectionData.end())
				{
					found->second.cleanup();
					playersConnectionData.erase(found);
				}
			}

		}
		break;

		case headerClientRecieveAllInventory:
		{
			//todo request a hard reset if this fails
			entityManager.localPlayer.inventory.readFromData(data, size);

		}
		break;

		case headerRecieveExitBlockInteraction:
		{
			Packet_RecieveExitBlockInteraction *exitInteraction = (Packet_RecieveExitBlockInteraction *)data;
			if (size == sizeof(Packet_RecieveExitBlockInteraction))
			{
				if (exitInteraction->revisionNumber == revisionNumberBlockInteraction)
				{
					shouldExitBlockInteraction = true;
				}
			}
		}
		break;

		case headerUpdateOwnOtherPlayerSettings:
		{
			Packet_UpdateOwnOtherPlayerSettings *packet = (Packet_UpdateOwnOtherPlayerSettings *)data;
			if (size == sizeof(Packet_UpdateOwnOtherPlayerSettings))
			{
				entityManager.localPlayer.otherPlayerSettings = packet->otherPlayerSettings;
			}

		}
		break;

		case headerSendPlayerSkin:
		{
			auto player = playersConnectionData.find(p.cid);

			if (player == playersConnectionData.end())
			{
				playersConnectionData[p.cid] = {};
				player = playersConnectionData.find(p.cid);
			}

			{
				player->second.skin.cleanup();

				player->second.skin.createFromBuffer(data, PLAYER_SKIN_SIZE, PLAYER_SKIN_SIZE,
					true, true);
				
				//todo repeating code
				player->second.skin.bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2.f);

				glGenerateMipmap(GL_TEXTURE_2D);

				player->second.skinBindlessTexture = glGetTextureHandleARB(player->second.skin.id);
				glMakeTextureHandleResidentARB(player->second.skinBindlessTexture);
			}


		}
		break;

		case headerUpdateLife:
		{
			if (sizeof(Packet_UpdateLife) != size) { break; }
			Packet_UpdateLife *p = (Packet_UpdateLife *)data;

			entityManager.localPlayer.life = p->life;

			entityManager.localPlayer.lastLife = {};
			entityManager.localPlayer.justHealedTimer = 0;
			entityManager.localPlayer.justRecievedDamageTimer = 0;

			entityManager.localPlayer.life.sanitize();
		}
		break;

		case headerRecieveDamage:
		{
			if (sizeof(Packet_UpdateLife) != size) { break; }
			Packet_UpdateLife *p = (Packet_UpdateLife *)data;

			if (entityManager.localPlayer.justRecievedDamageTimer <= 0)
			{
				entityManager.localPlayer.lastLife = entityManager.localPlayer.life;
			}

			entityManager.localPlayer.life = p->life;

			entityManager.localPlayer.justHealedTimer = 0;
			entityManager.localPlayer.justRecievedDamageTimer = 0.7;

			entityManager.localPlayer.life.sanitize();

			AudioEngine::playHurtSound();
		}
		break;

		case headerRecieveLife:
		{
			if (sizeof(Packet_UpdateLife) != size) { break; }
			Packet_UpdateLife *p = (Packet_UpdateLife *)data;

			entityManager.localPlayer.lastLife = entityManager.localPlayer.life;

			entityManager.localPlayer.life = p->life;

			entityManager.localPlayer.justHealedTimer = 0.7;
			entityManager.localPlayer.justRecievedDamageTimer = 0;

			entityManager.localPlayer.life.sanitize();
		}
		break;

		case headerRespawnPlayer:
		{
			if (sizeof(Packet_RespawnPlayer) != size) { break; }
			Packet_RespawnPlayer *packetData = (Packet_RespawnPlayer *)data;

			//respawn yourself
			if (p.cid == entityManager.localPlayer.entityId)
			{
				entityManager.localPlayer.entity.position = packetData->pos;
				entityManager.localPlayer.entity.lastPosition = packetData->pos;
				entityManager.localPlayer.entity.forces = {};
				respawn = true;
			}else
			{
				auto found = entityManager.players.find(p.cid);

				if (found != entityManager.players.end())
				{
					found->second.wasKilled = 0;
					found->second.flushCircularBuffer();

					found->second.entityBuffered.position = packetData->pos;
					found->second.entityBuffered.lastPosition = packetData->pos;
					found->second.entityBuffered.forces = {};
				}
			}

			break;
		}

		case headerUpdateEffects:
		{
			if (sizeof(Packet_UpdateEffects) != size) { break; }
			Packet_UpdateEffects *packetData = (Packet_UpdateEffects *)data;

			int timer = serverTimer - packetData->timer;

			if (timer > 15'000) { break; } //probably coruption or the packet is somehow too old
			if (timer < -4'000) { break; } //probably coruption

			if (timer > 0)
			{
				packetData->effects.passTimeMs(timer);
			}
			
			entityManager.localPlayer.effects = packetData->effects;

			
		}
		break;

		case headerSendChat:
		{

			if (size <= 1) { break; }
			data[size - 1] = 0;

			chat.push_front(std::string(data));
			chatTimer = 5;

		}
		break;


		default:
		break;

	}

	if (wasCompressed)
	{
		delete[] data;
	}
}

//this is not multy threaded
void clientMessageLoop(EventCounter &validatedEvent, RevisionNumber &invalidateRevision
	,glm::ivec3 playerPosition, int squareDistance, ClientEntityManager &entityManager,
	UndoQueue &undoQueue, ChunkSystem &chunkSystem,
	LightSystem &lightSystem,
	std::uint64_t &serverTimer, bool &disconnect
	, unsigned char revisionNumberBlockInteraction, bool &shouldExitBlockInteraction,
	bool &killedPlayer, bool &respawn,
	std::deque<std::string> &chat, float &chatTimer,
	InteractionData &playerInteraction,
	std::unordered_map<std::uint64_t, PlayerConnectionData> &playersConnectionData
	)
{
	ENetEvent event;
	int packetCount = 0;

	//ENetPacket *nextPacket = clientData.server->incomingDataTotal;
	for (int i = 0; i < 50; i++)
	{
		if (enet_host_service(clientData.client, &event, 0) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_RECEIVE:
				{
					
					recieveDataClient(event, validatedEvent, invalidateRevision,
						playerPosition, squareDistance, entityManager, undoQueue,
						chunkSystem, lightSystem, serverTimer,
						revisionNumberBlockInteraction, shouldExitBlockInteraction, killedPlayer,
						respawn, chat, chatTimer, playerInteraction, playersConnectionData);
					
					enet_packet_destroy(event.packet);

					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				{
					std::cout << "Disconect from client\n";
					disconnect = 1;
					break;
				}

			}
			packetCount++;
		}
		else
		{
			break;
		}
	}

	//if (packetCount) { std::cout << "Recieved: " << packetCount << "  "; }

	//int counter = 0;
	//auto nextPacket = clientData.client->dispatchQueue.sentinel.next;
	//
	//while (nextPacket)
	//{
	//	counter++;
	//	nextPacket = nextPacket->next;
	//}
	//
	//if (counter)
	//{
	//	std::cout << "Restant packets: " << counter << '\n';
	//}

}

void attackEntity(std::uint64_t eid, unsigned char inventorySlot, glm::vec3 direction,
	HitResult hitResult)
{
	Packet_AttackEntity packet;
	packet.entityID = eid;
	packet.inventorySlot = inventorySlot;
	packet.direction = direction;
	packet.hitResult = hitResult;

	sendPacket(getServer(), headerAttackEntity,
		clientData.cid,
		&packet, sizeof(packet), true, channelChunksAndBlocks);
}

void sendBlockInteractionMessage(std::uint64_t playerID, 
	glm::ivec3 pos, BlockType block, unsigned char revisionNumber)
{

	Packet_ClientInteractWithBlock data;
	data.blockPos = pos;
	data.blockType = block;
	data.interactionCounter = revisionNumber;

	sendPacket(getServer(), headerClientInteractWithBlock,
		playerID,
		&data, sizeof(data), true, channelChunksAndBlocks);

}

void closeConnection()
{

	if (!clientData.conected) { return; }
	
	if (clientData.server)
	{
		enet_peer_disconnect(clientData.server, 0);

		enet_host_flush(clientData.client);
			
		ENetEvent event = {};

		while (enet_host_service(clientData.client, &event, 1000) > 0)
		{
			if (event.type == ENET_EVENT_TYPE_RECEIVE)
			{
				enet_packet_destroy(event.packet);
			}
			else if (event.type == ENET_EVENT_TYPE_RECEIVE)
			{
				break;
			}
		}

		enet_peer_reset(clientData.server);
	}

	if (clientData.client)
	enet_host_destroy(clientData.client);
	

}

bool createConnection(Packet_ReceiveCIDAndData &playerData, const char *c)
{
	if (clientData.conected) { return false; }

	clientData = ConnectionData{};

	clientData.client = enet_host_create(nullptr, 1, 1, 0, 0);

	ENetAddress adress = {};
	ENetEvent event = {};
	
	if (c && c[0] != 0)
	{
		enet_address_set_host(&adress, c);
	}
	else
	{
		enet_address_set_host(&adress, "127.0.0.1");
	}

	adress.port = 7771; //todo port stuff

	//client, adress, channels, data to send rightAway
	clientData.server = enet_host_connect(clientData.client, &adress, SERVER_CHANNELS, 0);

	if (clientData.server == nullptr)
	{
		enet_host_destroy(clientData.client);
		return false;
	}

	clientData.server->timeoutMinimum = 10'000;
	clientData.server->timeoutMaximum = 30'000;
	clientData.server->timeoutLimit = 64;

	{

		// Set maximum throttle parameters for a specific client
		//clientData.server->packetThrottle = ENET_PEER_PACKET_THROTTLE_SCALE;
		//clientData.server->packetThrottleLimit = ENET_PEER_PACKET_THROTTLE_SCALE;
		//clientData.server->packetThrottleAcceleration = 2;
		//clientData.server->packetThrottleDeceleration = 2;
	}

	//see if we got events by server
	//client, event, ms to wait(0 means that we don't wait)
	if (enet_host_service(clientData.client, &event, 4000) > 0
		&& event.type == ENET_EVENT_TYPE_CONNECT)
	{
		std::cout << "client connected\n";
	}
	else
	{
		reportError("server timeout...");
		enet_peer_reset(clientData.server);
		enet_host_destroy(clientData.client);
		return false;
	}
	

	enet_peer_throttle_configure(clientData.server,
		ENET_PEER_PACKET_THROTTLE_INTERVAL, 6, 3);



	#pragma region handshake
	
	if (enet_host_service(clientData.client, &event, 2500) > 0
		&& event.type == ENET_EVENT_TYPE_RECEIVE)
	{
	
		{
			Packet p = {};
			size_t size;
			auto data = parsePacket(event, p, size);

			if (p.header != headerReceiveCIDAndData)
			{
				enet_peer_reset(clientData.server);
				enet_host_destroy(clientData.client);
				reportError("server sent wrong data");
				return false;
			}

			clientData.cid = p.cid;

			playerData = *(Packet_ReceiveCIDAndData *)data;

			//send player own info or sthing
			//sendPlayerData(e, true);

			std::cout << "received cid: " << clientData.cid << "\n";
			enet_packet_destroy(event.packet);
		};

		return true;
	}
	else
	{
		enet_peer_reset(clientData.server);
		enet_host_destroy(clientData.client);
		reportError("server handshake timeout");
		return 0;
	}

	#pragma endregion

	clientData.conected = true;
	return true;
}


bool placeItem(PlayerInventory &inventory, int from, int to, int counter)
{

	auto fromItem = inventory.getItemFromIndex(from);
	auto toItem = inventory.getItemFromIndex(to);

	if (fromItem && toItem)
	{

		if (counter == 0)
		{
			counter = fromItem->counter;
		}

		if (counter > fromItem->counter) { return 0; }

		if (toItem->type == 0)
		{

			*toItem = *fromItem;
			toItem->counter = counter;
			fromItem->counter -= counter;

			if (fromItem->counter == 0) { *fromItem = {}; }

			Packet_ClientMovedItem packet;
			packet.counter = counter;
			packet.from = from;
			packet.to = to;
			packet.itemType = toItem->type;
			packet.revisionNumber = inventory.revisionNumber;

			sendPacket(clientData.server, headerClientMovedItem, clientData.cid,
				&packet, sizeof(packet), true, channelChunksAndBlocks);
			return 1;
		}
		else if(areItemsTheSame(*toItem, *fromItem))
		{

			//see how many I can move

			if (toItem->counter < toItem->getStackSize())
			{
				if (toItem->counter + counter > toItem->getStackSize())
				{
					int moved = toItem->getStackSize() - toItem->counter;
					int overrun = toItem->counter + counter - toItem->getStackSize();
					toItem->counter = toItem->getStackSize();
					fromItem->counter = overrun;

					std::cout << moved << "\n";

					Packet_ClientMovedItem packet;
					packet.counter = moved;
					packet.from = from;
					packet.to = to;
					packet.itemType = toItem->type;
					packet.revisionNumber = inventory.revisionNumber;

					sendPacket(clientData.server, headerClientMovedItem, clientData.cid,
						&packet, sizeof(packet), true, channelChunksAndBlocks);
					return 1;
				}
				else
				{
					int addCounter = counter;
					toItem->counter += counter;
					fromItem->counter -= counter;

					if (fromItem->counter == 0) { *fromItem = {}; }

					Packet_ClientMovedItem packet;
					packet.counter = addCounter;
					packet.from = from;
					packet.to = to;
					packet.itemType = toItem->type;
					packet.revisionNumber = inventory.revisionNumber;

					sendPacket(clientData.server, headerClientMovedItem, clientData.cid,
						&packet, sizeof(packet), true, channelChunksAndBlocks);
					return 1;
				}
			}

		}



	}

	return 0;
}


bool swapItems(PlayerInventory &inventory, int from, int to)
{

	auto fromPtr = inventory.getItemFromIndex(from);
	auto toPtr = inventory.getItemFromIndex(to);

	if (fromPtr && toPtr && fromPtr != toPtr)
	{
		auto item = *fromPtr;
		*fromPtr = *toPtr;
		*toPtr = std::move(item);

		Packet_ClientSwapItems packet;
		packet.from = from;
		packet.to = to;
		packet.revisionNumber = inventory.revisionNumber;
		sendPacket(clientData.server, headerClientSwapItems, clientData.cid,
			&packet, sizeof(packet), true, channelChunksAndBlocks);

		return true;
	}

	return false;
}

bool grabItem(PlayerInventory &inventory, int from, int to, int counter)
{

	auto fromItem = inventory.getItemFromIndex(from);
	auto toItem = inventory.getItemFromIndex(to);

	if (fromItem && toItem && (fromItem != toItem))
	{

		if (counter == 0)
		{
			counter = fromItem->counter;
		}

		if (counter > fromItem->counter) { return 0; }


		if (toItem->type == 0 && fromItem->type != 0)
		{
			*toItem = *fromItem;
			toItem->counter = counter;
			fromItem->counter -= counter;

			if (fromItem->counter == 0) { *fromItem = {}; }

			Packet_ClientMovedItem packet;
			packet.counter = counter;
			packet.from = from;
			packet.to = to;
			packet.itemType = toItem->type;
			packet.revisionNumber = inventory.revisionNumber;

			sendPacket(clientData.server, headerClientMovedItem, clientData.cid,
				&packet, sizeof(packet), true, channelChunksAndBlocks);
			return 1;
		}

	}

	return 0;
}


bool forceOverWriteItem(PlayerInventory &inventory, int index, Item &item)
{
	static std::vector<unsigned char> tempData;

	auto to = inventory.getItemFromIndex(index);

	if (to)
	{
		*to = item;

		Packet_ClientOverWriteItem packet;
		packet.counter = item.counter;
		packet.itemType = item.type;
		packet.to = index;
		packet.revisionNumber = inventory.revisionNumber;
		packet.metadataSize = item.metaData.size();

		tempData.resize(sizeof(Packet_ClientOverWriteItem) + item.metaData.size());

		memcpy(tempData.data(), &packet, sizeof(packet));
		memcpy(tempData.data() + sizeof(packet), item.metaData.data(), item.metaData.size());

		sendPacket(clientData.server, headerClientOverWriteItem, clientData.cid,
			tempData.data(), tempData.size(), true, channelChunksAndBlocks);

		return 1;
	}
	else
	{
		return 0;
	}

}