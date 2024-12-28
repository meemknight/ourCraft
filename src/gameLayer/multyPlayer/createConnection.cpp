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
	case Task::generateChunk:
	{
		p.header = headerRequestChunk;
		Packet_RequestChunk packetData = {};
		packetData.chunkPosition = {t.pos.x, t.pos.z};
		packetData.playersPositionAtRequest = t.playerPosForChunkGeneration;

		sendPacket(data.server, p, (char *)&packetData, sizeof(packetData), 1, 
			channelChunksAndBlocks);
		break;
	}
	case Task::placeBlock:
	{
		//todo remove
		permaAssert(0);
		break;
	}
	default:

	//todo remove
	case Task::droppedItemEntity:
	{
		permaAssert(0);
		//p.header = headerClientDroppedItem;
		//Packet_ClientDroppedItem packetData = {};
		//packetData.position = t.doublePos;
		//packetData.blockType = t.blockType;
		//packetData.count = t.blockCount;
		//packetData.eventId = t.eventId;
		//packetData.entityID = t.entityId;
		//packetData.motionState = t.motionState;
		//packetData.timer = t.timer;
		//
		//sendPacket(data.server, p, (char *)&packetData, sizeof(packetData), 1,
		//	channelChunksAndBlocks);
	}

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

std::vector<Packet_PlaceBlocks> getRecievedBlocks()
{
	//auto ret = std::move(recievedBlocks);
	auto ret = clientData.recievedBlocks;
	clientData.recievedBlocks.clear();
	return ret;
}


ConnectionData getConnectionData()
{
	return clientData;
}

#define CASE_UPDATE(I) case I: { \
if (size != sizeof(Packet_UpdateGenericEntity) + sizeof(decltype((*entityManager.entityGetter<I>())[0].entity))) { break; } \
auto *p = (decltype((*entityManager.entityGetter<I>())[0].entity) *)(data + sizeof(Packet_UpdateGenericEntity)); \
float restantTimer = computeRestantTimer(firstPart->timer, yourTimer); \
entityManager.addOrUpdateGenericEntity< I >(firstPart->eid, *p, undoQueue, restantTimer);\
} break;

void recieveDataClient(ENetEvent &event, 
	EventCounter &validatedEvent, 
	RevisionNumber &invalidateRevision,
	glm::ivec3 playerPosition, int squareDistance,
	ClientEntityManager &entityManager,
	UndoQueue &undoQueue, ChunkSystem &chunkSystem, 
	LightSystem &lightSystem,
	std::uint64_t &yourTimer,
	unsigned char revisionNumberBlockInteraction,
	bool &shouldExitBlockInteraction, bool &killedPlayer, bool &respawn
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
				std::cout << "Size error " << size << "\n";
				break;
			}

			if (chunkSystem.isChunkInRadiusAndBounds({playerPosition.x, playerPosition.z},
				{chunkPacket->chunk.x, chunkPacket->chunk.z}))
			{
				

				{
					//remove from recently requested chunks so we can request again
					//if needed for some reason
					{
						auto f = chunkSystem.recentlyRequestedChunks.find({chunkPacket->chunk.x, chunkPacket->chunk.z});
						if (f != chunkSystem.recentlyRequestedChunks.end())
						{
							chunkSystem.recentlyRequestedChunks.erase(f);

						}
					}

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

		case headerRecieveBlockData:
		{
			if (size == 0) { break; }
			
				
			size_t pointer = 0;
			while (size - pointer > 0)
			{
				if (size < sizeof(BlockDataHeader)) { break; } //todo request hard reset here

				BlockDataHeader blockHeader = {};
				memcpy(&blockHeader, data + pointer, sizeof(BlockDataHeader));
				pointer += sizeof(blockHeader);

				//check if corupted data
				if(blockHeader.pos.y < 0 || blockHeader.pos.y >= CHUNK_HEIGHT) { break; } //todo request hard reset here

				if (blockHeader.blockType == BlockTypes::structureBase)
				{

					if (blockHeader.dataSize)
					{

					}
					else
					{

					}

				}
				else
				{
					{ break; } //todo request hard reset here
				}

				
			}


			break;
		}

		case headerPlaceBlock:
		{
			Packet_PlaceBlocks b;
			b.blockPos = ((Packet_PlaceBlocks *)data)->blockPos;
			b.blockType = ((Packet_PlaceBlocks *)data)->blockType;
			clientData.recievedBlocks.push_back(b);
			break;
		}

		case headerPlaceBlocks:
		{
			for (int i = 0; i < size / sizeof(Packet_PlaceBlocks); i++)
			{
				clientData.recievedBlocks.push_back( ((Packet_PlaceBlocks*)data)[i] );

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
			}
			else
			{
				if (checkIfPlayerShouldGetEntity({playerPosition.x, playerPosition.z},
					entity->entity.position, squareDistance, 0))
				{

					auto found = entityManager.players.find(entity->eid);
					
					if (found == entityManager.players.end())
					{
						entityManager.players[entity->eid] = {};
						found = entityManager.players.find(entity->eid);
					}
					
					found->second.rubberBand
						.addToRubberBand(found->second.entity.position - entity->entity.position);

					found->second.entity = entity->entity;

					float restantTimer = computeRestantTimer(entity->timer, yourTimer);
					
					found->second.restantTime = restantTimer;

				}
				else
				{
					//dropped recieved entity
				}
				


			}

			break;
		}

		case headerClientRecieveDroppedItemUpdate:
		{

			Packet_RecieveDroppedItemUpdate *p = (Packet_RecieveDroppedItemUpdate *)data;

			if (sizeof(Packet_RecieveDroppedItemUpdate) != size) { break; } //todo logs or something
			
			//todo dont break if reliable
			if (p->timer + 16 < yourTimer)
			{
				break; //drop too old packets
			}

			float restantTimer = computeRestantTimer(p->timer, yourTimer);

			entityManager.addOrUpdateDroppedItem(p->eid, p->entity, undoQueue,
				restantTimer);

			//std::cout << restantTimer << "\n";

			break;
		}

		case headerUpdateZombie:
		{
			Packet_UpdateZombie *p = (Packet_UpdateZombie *)data;
			if (sizeof(Packet_UpdateZombie) != size) { break; }

			//todo dont break if reliable
			if (p->timer + 16 < yourTimer)
			{
				break; //drop too old packets
			}
			float restantTimer = computeRestantTimer(p->timer, yourTimer);

			entityManager.addOrUpdateZombie(p->eid, p->entity, restantTimer);

			break;
		}

		case headerUpdatePig:
		{
			Packet_UpdatePig *p = (Packet_UpdatePig *)data;
			if (sizeof(Packet_UpdatePig) != size) { break; }

			//todo dont break if reliable
			if (p->timer + 16 < yourTimer)
			{
				break; //drop too old packets
			}
			float restantTimer = computeRestantTimer(p->timer, yourTimer);

			entityManager.addOrUpdatePig(p->eid, p->entity, restantTimer);

			break;
		}

		case headerUpdateCat:
		{
			Packet_UpdateCat *c = (Packet_UpdateCat *)data;
			if (sizeof(Packet_UpdateCat) != size) { break; }
			if (c->timer + 16 < yourTimer)
			{
				break; //drop too old packets
			}
			float restantTimer = computeRestantTimer(c->timer, yourTimer);

			entityManager.addOrUpdateCat(c->eid, c->entity, restantTimer);

			break;
		}

		case headerUpdateGenericEntity:
		{

			Packet_UpdateGenericEntity *firstPart = (Packet_UpdateGenericEntity *)data;
			if (size < sizeof(Packet_UpdateGenericEntity)) { break; }

			auto entityType = getEntityTypeFromEID(firstPart->eid);

			if (firstPart->timer + 16 < yourTimer)
			{
				break; //drop too old packets
			}

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

			yourTimer = p->timer;
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
			auto player = entityManager.players.find(p.cid);

			if (player != entityManager.players.end())
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
					found->second.entity.position = packetData->pos;
					found->second.entity.lastPosition = packetData->pos;
					found->second.entity.forces = {};
				}
			}

			break;
		}

		case headerUpdateEffects:
		{
			if (sizeof(Packet_UpdateEffects) != size) { break; }
			Packet_UpdateEffects *packetData = (Packet_UpdateEffects *)data;

			int timer = yourTimer - packetData->timer;

			if (timer > 15'000) { break; } //probably coruption or the packet is somehow too old

			if (timer > 0)
			{
				packetData->effects.passTimeMs(timer);
			}
			
			entityManager.localPlayer.effects = packetData->effects;

			
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
	bool &killedPlayer, bool &respawn
	)
{
	ENetEvent event;

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
						respawn);
					
					enet_packet_destroy(event.packet);

					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				{
					disconnect = 1;
					break;
				}

			}
		}
		else
		{
			break;
		}
	}

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

	auto test = clientData.server;

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