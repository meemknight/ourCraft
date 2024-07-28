#include "multyPlayer/enetServerFunction.h"
#include <atomic>
#include <thread>
#include <enet/enet.h>
#include <iostream>
#include <vector>
#include "multyPlayer/packet.h"
#include <unordered_map>
#include "threadStuff.h"
#include <mutex>
#include <queue>
#include <multyPlayer/server.h>
#include <chrono>
#include <gameplay/entityManagerClient.h>
#include <multyPlayer/server.h>
#include <errorReporting.h>
#include <biome.h>
#include <multyPlayer/chunkSaver.h>
#include <worldGenerator.h>
#include <fstream>
#include <sstream>
#include <multyPlayer/splitUpdatesLogic.h>
#include <filesystem>

//todo add to a struct
ENetHost *server = 0;
std::unordered_map<std::uint64_t, Client> connections;
static std::uint64_t entityId = RESERVED_CLIENTS_ID + 1;
static std::thread enetServerThread;


std::uint64_t getEntityIdAndIncrement(WorldSaver &worldSaver)
{
	std::uint64_t id = entityId;
	entityId++;

	worldSaver.saveEntityId(entityId);

	return entityId;
}

std::uint64_t getCurrentEntityId()
{
	return entityId;
}

void broadCastNotLocked(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel)
{
	for (auto it = connections.begin(); it != connections.end(); it++)
	{
		if (!peerToIgnore || (it->second.peer != peerToIgnore))
		{
			sendPacket(it->second.peer, p, (const char *)data, size, true, channel);
		}
	}
}

void broadCast(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel)
{
	broadCastNotLocked(p, data, size, peerToIgnore, reliable, channel);
}

bool checkIfPlayerShouldGetChunk(glm::ivec2 playerPos2D,
	glm::ivec2 chunkPos, int playerSquareDistance)
{
	glm::ivec2 playerChunk = fromBlockPosToChunkPos({playerPos2D.x, 0, playerPos2D.y});
	float dist = glm::length(glm::vec2(playerChunk - chunkPos));
	if (dist > (playerSquareDistance / 2.f) * std::sqrt(2.f) + 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

//todo this should dissapear
Client getClient(std::uint64_t cid)
{
	Client rez = connections[cid];
	return rez;
}

Client *getClientSafe(std::uint64_t cid)
{
	auto found = connections.find(cid);

	if (found != connections.end())
	{
		return &found->second;
	}

	return nullptr;
}

Client *getClientNotLocked(std::uint64_t cid)
{
	auto it = connections.find(cid);
	if (it == connections.end()) { return 0; }
	return &it->second;
}

//todo, check if every use of this is good, and that it is ok that it is a copy
std::unordered_map<std::uint64_t, Client> getAllClients()
{
	auto rez = connections;
	return rez;
}

std::unordered_map<std::uint64_t, Client> &getAllClientsReff()
{
	return connections;
}

void insertConnection(std::uint64_t cid, Client &c)
{
	connections.insert({cid, c});
}

void sentNewConnectionMessage(ENetPeer *peer, Client c, std::uint64_t eid)
{
	Packet p;
	p.header = headerClientRecieveOtherPlayerPosition;
	
	Packet_ClientRecieveOtherPlayerPosition data;
	data.entity = c.playerData.entity;
	data.eid = eid;
	data.timer = getTimer();

	sendPacket(peer, p, (const char *)&data,
		sizeof(data), true, channelHandleConnections);
}

void broadcastNewConnectionMessage(ENetPeer *peerToIgnore, Client c, std::uint64_t cid)
{
	Packet p;
	p.header = headerClientRecieveOtherPlayerPosition;

	Packet_ClientRecieveOtherPlayerPosition data;
	data.entity = c.playerData.entity;
	data.eid = cid;

	//no need to lock because this is the thread to modify the data
	broadCastNotLocked(p, &data,
		sizeof(data), peerToIgnore, true, channelHandleConnections);
}

void sendPlayerInventoryAndIncrementRevision(Client &client, int channel)
{
	client.playerData.inventory.revisionNumber++;

	sendPlayerInventoryNotIncrementRevision(client, channel);
}

void sendPlayerInventoryNotIncrementRevision(Client &client, int channel)
{
	std::vector<unsigned char> data;
	client.playerData.inventory.formatIntoData(data);

	//todo adaptive package here, compress
	sendPacket(client.peer, headerClientRecieveAllInventory, data.data(), data.size(), true,
		channel);
}

void sendPlayerExitInteraction(Client &client, unsigned char revisionNumber)
{
	//notify we don't allow interaction!
	Packet packet;
	packet.header = headerRecieveExitBlockInteraction;

	Packet_RecieveExitBlockInteraction packetData;
	packetData.revisionNumber = revisionNumber;

	sendPacket(client.peer, packet,
		(char *)&packetData, sizeof(Packet_RecieveExitBlockInteraction),
		true, channelChunksAndBlocks);
}

void addConnection(ENetHost *server, ENetEvent &event, WorldSaver &worldSaver)
{

	std::uint64_t id = getEntityIdAndIncrement(worldSaver);

	glm::dvec3 spawnPosition(0, 107, 0);

	{
		Client c{event.peer}; 
		c.playerData.entity.position = spawnPosition;
		c.playerData.entity.lastPosition = spawnPosition;

		c.playerData.otherPlayerSettings.gameMode = OtherPlayerSettings::CREATIVE;

		c.playerData.inventory.items[0] = Item(grassBlock, 20);
		c.playerData.inventory.items[1] = Item(grassBlock, 2);
		c.playerData.inventory.items[2] = Item(grassBlock, 64);
		c.playerData.inventory.items[3] = Item(grassBlock, 1);
		c.playerData.inventory.items[4] = Item(testBlock);
		c.playerData.inventory.items[6] = Item(torch);
		c.playerData.inventory.items[7] = Item(spruce_leaves_red);
		c.playerData.inventory.items[8] = Item(rose);
		c.playerData.inventory.items[9] = Item(BlockTypes::bookShelf);
		c.playerData.inventory.items[17] = Item(BlockTypes::diamond_ore);
		c.playerData.inventory.items[18] = Item(BlockTypes::cactus_bud);
		c.playerData.inventory.items[19] = Item(BlockTypes::wooden_plank, 64);
		c.playerData.inventory.items[20] = Item(BlockTypes::jungle_planks, 64);
		c.playerData.inventory.items[21] = Item(BlockTypes::woodLog, 64);
		c.playerData.inventory.items[27] = Item(BlockTypes::birch_wood);
		c.playerData.inventory.items[28] = itemCreator(ItemTypes::wooddenSword);
		c.playerData.inventory.items[29] = itemCreator(ItemTypes::wooddenSword);
		c.playerData.inventory.items[30] = itemCreator(ItemTypes::wooddenSword);
		c.playerData.inventory.items[31] = itemCreator(ItemTypes::catSpawnEgg);
		c.playerData.inventory.items[32] = itemCreator(ItemTypes::zombieSpawnEgg);
		c.playerData.inventory.items[33] = itemCreator(ItemTypes::pigSpawnEgg);
		c.playerData.inventory.items[34] = Item(BlockTypes::clay);
		c.playerData.inventory.items[35] = Item(BlockTypes::glass);

		insertConnection(id, c);
	}

	{
		Packet p;
		p.header = headerReceiveCIDAndData;
		p.cid = id;
	
		Packet_ReceiveCIDAndData packetToSend = {};
		packetToSend.entity = getClient(id).playerData.entity;
		packetToSend.yourPlayerEntityId = id;
		packetToSend.timer = getTimer();
		packetToSend.otherSettings = getClient(id).playerData.otherPlayerSettings;

		//send own cid
		sendPacket(event.peer, p, (const char *)&packetToSend,
			sizeof(packetToSend), true, channelHandleConnections);


	}

	//todo maybe send entities to this new connection?


	{
		auto timer = getTimer();

		Packet packet;
		packet.header = headerClientUpdateTimer;

		Packet_ClientUpdateTimer packetData;
		packetData.timer = timer;

		sendPacket(event.peer, packet, (const char *)&packetData,
			sizeof(packetData), true, channelHandleConnections);
	}

	//send inventory
	{
		sendPlayerInventoryAndIncrementRevision(connections[id], channelHandleConnections);
	}

	//send others to this
	for (auto &c : connections)
	{
		if (c.first != id)
		{
			sentNewConnectionMessage(event.peer, c.second, c.first);
		}

	}

	addCidToServerSettings(id);

	//send this to others
	Client c = *getClientNotLocked(id);
	broadcastNewConnectionMessage(event.peer, c, id);

}

void removeConnection(ENetHost *server, ENetEvent &event)
{
	
	for (auto &c : connections)
	{
		if (c.second.peer == event.peer)
		{
			Packet p = {};
			p.header = headerDisconnectOtherPlayer;
			p.cid = c.first;

			Packet_DisconectOtherPlayer data;
			data.EID = c.first;

			broadCastNotLocked(p, &data, sizeof(data), 0, true, channelHandleConnections);

			connections.erase(connections.find(c.first));
			break;
		}
	}


}

void recieveData(ENetHost *server, ENetEvent &event, std::vector<ServerTask> &serverTasks)
{
	Packet p;
	size_t size = 0;
	auto data = parsePacket(event, p, size);


	if (connections.find(p.cid) == connections.end())
	{
		reportError((std::string("packet recieved with a CID that doesn't exist: ") + std::to_string(p.cid) + "\n").c_str());
		return;
	}

	//validate data
	//no need for mutex here fortunatelly because this thread is the one that modifies the connections?
	//connectionsMutex.lock();
	if (connections[p.cid].peer != event.peer)
	{
		//connectionsMutex.unlock();
		reportError("invalidData: connections[p.cid].peer != event.peer");
		return;
	}
	//connectionsMutex.unlock();


	ServerTask serverTask = {};
	serverTask.cid = p.cid;

	//todo check invalid cids here instead of every time later

	switch (p.header)
	{
		case headerRequestChunk:
		{
			Packet_RequestChunk packetData = *(Packet_RequestChunk *)data;
			
			int squareDistance = 0;
			bool error = 0;

			{

				auto it = connections.find(p.cid);
				if (it == connections.end())
				{
					reportError("invalid cid error");
					error = true;
				}
				else
				{
					it->second.positionForChunkGeneration = packetData.playersPositionAtRequest;
					//std::cout << packetData.playersPositionAtRequest.x << "\n";
					squareDistance = it->second.playerData.entity.chunkDistance;
				}

			}

			if (!error)
			{
				serverTask.t.taskType = Task::generateChunk;
				serverTask.t.pos = {packetData.chunkPosition.x, 0, packetData.chunkPosition.y};

				serverTasks.push_back(serverTask);
			}
			
			
			
			break;
		}

		//todo inventory revision here and consume blocks if not creative
		case headerPlaceBlock:
		{
			Packet_ClientPlaceBlock packetData = *(Packet_ClientPlaceBlock *)data;
			serverTask.t.taskType = Task::placeBlock;
			serverTask.t.pos = {packetData.blockPos};
			serverTask.t.blockType = {packetData.blockType};
			serverTask.t.eventId = packetData.eventId;
			serverTask.t.revisionNumber = packetData.inventoryRevision;
			serverTask.t.inventroySlot = packetData.inventorySlot;

			serverTasks.push_back(serverTask);
			break;
		}

		case headerBreakBlock:
		{
			Packet_ClientBreakBlock packetData = *(Packet_ClientBreakBlock *)data;
			serverTask.t.taskType = Task::breakBlock;
			serverTask.t.pos = {packetData.blockPos};
			serverTask.t.eventId = packetData.eventId;

			serverTasks.push_back(serverTask);
			break;

		}

		case headerSendPlayerData:
		{
			Packer_SendPlayerData packetData = *(Packer_SendPlayerData *)data;


			Client clientCopy = {};
			std::uint64_t clientCopyCid = 0;
			
			auto it = connections.find(p.cid);
			if (it == connections.end())
			{
				reportError("invalid cid error in headerSendPlayerData");
			}
			else
			{

				if (it->second.playerData.entity.position != packetData.playerData.position)
				{


					clientCopy = it->second;
					clientCopyCid = it->first;

					//todo something better here...
					auto s = getServerSettingsCopy();
					s.perClientSettings[p.cid].outPlayerPos = packetData.playerData.position;
					setServerSettings(s);
				}
				else
				{
					//nothing changed
				}

				it->second.playerData.entity = packetData.playerData;

			}


			if (packetData.timer + 16 * 4 > getTimer())
			{

				//broadcast player movement
				if (clientCopyCid)
				{
					Packet_ClientRecieveOtherPlayerPosition sendData;

					sendData.timer = getTimer();
					sendData.eid = clientCopyCid;
					sendData.entity = clientCopy.playerData.entity;

					Packet p;
					p.cid = 0;
					p.header = headerClientRecieveOtherPlayerPosition;

					broadCast(p, &sendData, sizeof(sendData), clientCopy.peer, false, channelPlayerPositions);
				}

			}

			break;
		}

		//todo inventory revision here
		case headerClientDroppedItem:
		{
			if (size != sizeof(Packet_ClientDroppedItem))
			{
				//error checking + cick clients that send corrupted data?
				reportError("corrupted packet or something Packet_ClientDroppedItem");
				break;
			}

			Packet_ClientDroppedItem *packetData = (Packet_ClientDroppedItem *)data;

			serverTask.t.taskType = Task::droppedItemEntity;
			serverTask.t.doublePos = packetData->position;
			serverTask.t.blockCount = packetData->count;
			serverTask.t.from = packetData->inventorySlot;
			serverTask.t.entityId = packetData->entityID;
			serverTask.t.eventId = packetData->eventId;
			serverTask.t.motionState = packetData->motionState;
			serverTask.t.timer = packetData->timer;
			serverTask.t.blockType = packetData->type;
			serverTask.t.revisionNumber = packetData->revisionNumberInventory;

			serverTasks.push_back(serverTask);
			break;

		}

		//inventory stuff
		case headerClientMovedItem:
		{
			if (size != sizeof(Packet_ClientMovedItem))
			{
				//todo hard reset on errors.
				reportError("corrupted packet or something Packet_ClientDroppedItem");
				break;
			}
			Packet_ClientMovedItem *packetData = (Packet_ClientMovedItem *)data;

			serverTask.t.taskType = Task::clientMovedItem;
			serverTask.t.itemType = packetData->itemType;
			serverTask.t.from = packetData->from;
			serverTask.t.to = packetData->to;
			serverTask.t.blockCount = packetData->counter;
			serverTask.t.revisionNumber = packetData->revisionNumber;
			serverTasks.push_back(serverTask);

			break;
		}

		case headerClientCraftedItem:
		{

			if (size != sizeof(Packet_ClientCraftedItem))
			{
				//todo hard reset on errors.
				reportError("corrupted packet or something Packet_ClientCraftedItem");
				break;
			}
			Packet_ClientCraftedItem *packetData = (Packet_ClientCraftedItem *)data;

			serverTask.t.taskType = Task::clientCraftedItem;
			serverTask.t.itemType = packetData->itemType;
			serverTask.t.blockCount = packetData->counter;
			serverTask.t.to = packetData->to;
			serverTask.t.revisionNumber = packetData->revisionNumber;
			serverTasks.push_back(serverTask);

			break;
		}

		case headerClientOverWriteItem:
		{
			if (size != sizeof(Packet_ClientOverWriteItem))
			{
				break;
			}

			Packet_ClientOverWriteItem *packetData = (Packet_ClientOverWriteItem *)data;
			serverTask.t.taskType = Task::clientOverwriteItem;
			serverTask.t.itemType = packetData->itemType;
			serverTask.t.to = packetData->to;
			serverTask.t.blockCount = packetData->counter;
			serverTask.t.revisionNumber = packetData->revisionNumber;
			serverTasks.push_back(serverTask);

			break;
		}

		case headerClientSwapItems:
		{
			if (size != sizeof(Packet_ClientSwapItems))
			{
				break; //todo hard reset stuff everywhere
			}

			Packet_ClientSwapItems *packetData = (Packet_ClientSwapItems *)data;
			serverTask.t.taskType = Task::clientSwapItems;
			serverTask.t.from = packetData->from;
			serverTask.t.to = packetData->to;
			serverTask.t.revisionNumber = packetData->revisionNumber;
			serverTasks.push_back(serverTask);

			break;

		}

		case headerClientUsedItem:
		{

			if (size != sizeof(Packet_ClientUsedItem))
			{
				break;
			}

			Packet_ClientUsedItem *packetData = (Packet_ClientUsedItem *)data;
			serverTask.t.taskType = Task::clientUsedItem;
			serverTask.t.from = packetData->from;
			serverTask.t.itemType = packetData->itemType;
			serverTask.t.pos = packetData->position;
			serverTask.t.revisionNumber = packetData->revisionNumber;
			serverTasks.push_back(serverTask);

			break;
		}
		
		case headerClientInteractWithBlock:
		{

			if (size != sizeof(Packet_ClientInteractWithBlock))
			{
				break;
			}

			Packet_ClientInteractWithBlock *packetData = (Packet_ClientInteractWithBlock *)data;
			serverTask.t.taskType = Task::clientInteractedWithBlock;
			serverTask.t.blockType = packetData->blockType;
			serverTask.t.pos = packetData->blockPos;
			serverTask.t.revisionNumber = packetData->interactionCounter;
			serverTasks.push_back(serverTask);

			break;
		}

		case headerRecieveExitBlockInteraction:
		{
			if (size != sizeof(Packet_RecieveExitBlockInteraction))
			{
				break;
			}

			Packet_RecieveExitBlockInteraction *packetData = (Packet_RecieveExitBlockInteraction *)data;
			serverTask.t.taskType = Task::clientExitedInteractionWithBlock;
			serverTask.t.revisionNumber = packetData->revisionNumber;
			
			serverTasks.push_back(serverTask);

		}
		break;

		default:

		break;
	}





}



static std::atomic<bool> enetServerRunning = false;

void enetServerFunction()
{
	std::cout << "Successfully started server!\n";


	entityId = RESERVED_CLIENTS_ID + 1;


	StructuresManager structuresManager;
	BiomesManager biomesManager;
	WorldSaver worldSaver;

	worldSaver.savePath = RESOURCES_PATH "saves/";

	{
		std::error_code err = {};
		std::filesystem::create_directory(worldSaver.savePath, err);
		if (err) 
		{ 
			std::cout << err << "\n";
			//todo error report and close this thing gracefully please
			exit(0); 
		}
	}


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
	ENetEvent event = {};

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



	auto start = std::chrono::high_resolution_clock::now();

	float sendEntityTimer = 0.5;
	float sentTimerUpdateTimer = 1;

	float tickTimer = 0;

	std::vector<ServerTask> serverTasks;
	serverTasks.reserve(100);

	if (!worldSaver.loadEntityId(entityId))
	{
		//todo try to fix corupted data here.
		entityId = RESERVED_CLIENTS_ID + 1;
	}else
	if (entityId < RESERVED_CLIENTS_ID + 1)
	{
		entityId = RESERVED_CLIENTS_ID + 1;
	}

	while (enetServerRunning)
	{
		auto stop = std::chrono::high_resolution_clock::now();

		float deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(stop - start)).count() / 1000000.0f;
		start = std::chrono::high_resolution_clock::now();
		tickTimer += deltaTime;
		auto settings = getServerSettingsCopy();

		//int waitTimer = ((1.f/settings.targetTicksPerSeccond)-tickTimer) * 1000;
		//waitTimer = std::max(std::min(waitTimer-1, 10), 0);
		
		int waitTime = 3;
		int tries = 10;
		while (((enet_host_service(server, &event, waitTime) > 0) || (waitTime=0, tries-- > 0) ) 
			&& enetServerRunning)
		{
			//we wait only the first time, than we want to let the server update happen.
			waitTime = 0;

			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					addConnection(server, event, worldSaver);

					std::cout << "Successfully connected!\n";

					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					recieveData(server, event, serverTasks);

					enet_packet_destroy(event.packet);

					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{

					//std::cout << "disconnect: "
					//	<< event.peer->address.host << " "
					//	<< event.peer->address.port << "\n\n";
					removeConnection(server, event);
					break;
				}


			}
		}
		
		if (!enetServerRunning) { break; }

	#pragma region server send entity position data

		{
			sendEntityTimer -= deltaTime;

			if (sendEntityTimer < 0)
			{
				sendEntityTimer = 0.4;
			
				//todo make a different timer for each player			
				//todo maybe merge things into one packet

				//no need for mutex because this thread modifies the clients data
				auto connections = getAllClients();

				for (auto &c : connections)
				{

					// send players

					for (auto &other : connections)
					{
						if (other.first != c.first)
						{

							if (checkIfPlayerShouldGetEntity(
								{c.second.playerData.entity.position.x, c.second.playerData.entity.position.z},
								other.second.playerData.entity.position, c.second.playerData.entity.chunkDistance, 0)
								)
							{
								Packet_ClientRecieveOtherPlayerPosition sendData;
								sendData.eid = other.first;
								sendData.timer = getTimer();
								sendData.entity = other.second.playerData.entity;

								Packet p;
								p.cid = 0;
								p.header = headerClientRecieveOtherPlayerPosition;

								sendPacket(c.second.peer, p, (const char *)&sendData, sizeof(sendData),
									false, channelPlayerPositions);
							}



						}
					}


				}

			}

		}

	#pragma endregion

	#pragma region server sends timer updates
		{

			sentTimerUpdateTimer -= deltaTime;

			if (sentTimerUpdateTimer < 0)
			{
				sentTimerUpdateTimer = 1.f;
				auto timer = getTimer();
			
				Packet packet;
				packet.header = headerClientUpdateTimer;
					
				Packet_ClientUpdateTimer packetData;
				packetData.timer = timer;

				broadCast(packet, &packetData, sizeof(packetData), nullptr, false, channelHandleConnections);

			}



		}
	#pragma endregion

	serverWorkerUpdate(wg, structuresManager, biomesManager,
		worldSaver, serverTasks, deltaTime);

	
	}

	clearSD(worldSaver);
	wg.clear();
	structuresManager.clear();

	for (auto &c : connections)
	{
		enet_peer_disconnect(c.second.peer, 0);
		enet_peer_reset(c.second.peer);
	}

	enet_host_flush(server);

	int capCounter = 20;
	while (enet_host_service(server, &event, 100) > 0 && capCounter-->0)
	{
		if (event.type == ENET_EVENT_TYPE_RECEIVE)
		{
			enet_packet_destroy(event.packet);
		}
	}

}

bool startEnetListener(ENetHost *_server)
{
	server = _server;
	connections = {};

	bool expected = 0;
	if (enetServerRunning.compare_exchange_strong(expected, 1))
	{
		enetServerThread = std::move(std::thread(enetServerFunction));
		return 1;
	}
	else
	{
		return 0;
	}
}

void closeEnetListener()
{
	enetServerRunning = false;
	enetServerThread.join();
}
