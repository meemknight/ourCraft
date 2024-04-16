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


//todo add to a struct
ENetHost *server = 0;
std::mutex connectionsMutex; //todo remove 
std::unordered_map<CID, Client> connections;
static CID pids = 1;
static std::uint64_t entityId = 1;
static std::thread enetServerThread;

std::mutex entityIdMutex; //todo remove
std::uint64_t getEntityIdNowLocked() { return entityId; }
void lockEntityIdMutex() { entityIdMutex.lock(); }
void unlockEntityIdMutex() { entityIdMutex.unlock(); }

std::uint64_t getEntityIdSafeAndIncrement()
{
	lockEntityIdMutex();
	std::uint64_t id = entityId;
	entityId++;
	unlockEntityIdMutex();
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
	connectionsMutex.lock();
	broadCastNotLocked(p, data, size, peerToIgnore, reliable, channel);
	connectionsMutex.unlock();
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

Client getClient(CID cid)
{
	connectionsMutex.lock();
	Client rez = connections[cid];
	connectionsMutex.unlock();
	return rez;
}

Client *getClientNotLocked(CID cid)
{
	auto it = connections.find(cid);
	if (it == connections.end()) { return 0; }
	return &it->second;
}

void lockConnectionsMutex()
{
	connectionsMutex.lock();
}

void unlockConnectionsMutex()
{
	connectionsMutex.unlock();
}

std::unordered_map<CID, Client> getAllClients()
{
	connectionsMutex.lock();
	auto rez = connections;
	connectionsMutex.unlock();
	return rez;
}

void insertConnection(CID cid, Client c)
{
	connectionsMutex.lock();
	connections.insert({cid, c});
	connectionsMutex.unlock();
}


std::mutex taskMutex; //todo remove
std::condition_variable taskCondition; //todo remove
std::queue<ServerTask> tasks; //todo remove

void submitTaskForServer(ServerTask t)
{
	std::unique_lock<std::mutex> locker(taskMutex);
	tasks.push(t);
	locker.unlock();
	taskCondition.notify_one();
}

void sentNewConnectionMessage(ENetPeer *peer, Client c, CID cid)
{
	Packet p;
	p.header = headerConnectOtherPlayer;
	
	Packet_HeaderConnectOtherPlayer data;
	data.cid = cid;
	data.position = c.playerData.position;
	data.entityId = c.entityId;

	sendPacket(peer, p, (const char *)&data,
		sizeof(data), true, channelHandleConnections);
}

void broadcastNewConnectionMessage(ENetPeer *peerToIgnore, Client c, CID cid)
{
	Packet p;
	p.header = headerConnectOtherPlayer;

	Packet_HeaderConnectOtherPlayer data;
	data.cid = cid;
	data.position = c.playerData.position;
	data.entityId = c.entityId;

	//no need to lock because this is the thread to modify the data
	broadCastNotLocked(p, &data,
		sizeof(data), peerToIgnore, true, channelHandleConnections);
}

void addConnection(ENetHost *server, ENetEvent &event)
{

	auto id = getEntityIdSafeAndIncrement();

	glm::dvec3 spawnPosition(0, 107, 0);

	{
		Client c{event.peer};
		c.playerData.position = spawnPosition;
		c.entityId = id;
		insertConnection(pids, c);
	}

	CID currentCid = pids;
	{
		Packet p;
		p.header = headerReceiveCIDAndData;
		p.cid = pids;
		pids++;
	
		Packet_ReceiveCIDAndData packetToSend = {};
		packetToSend.playersPosition = spawnPosition;
		packetToSend.yourPlayerEntityId = id;
	
		//send own cid
		sendPacket(event.peer, p, (const char *)&packetToSend,
			sizeof(packetToSend), true, channelHandleConnections);
	}

	//todo send entities to this new connection


	{
		auto timer = getTimer();

		Packet packet;
		packet.header = headerClientUpdateTimer;

		Packet_ClientUpdateTimer packetData;
		packetData.timer = timer;

		sendPacket(event.peer, packet, (const char *)&packetData,
			sizeof(packetData), true, channelHandleConnections);
	}

	//send others to this
	for (auto &c : connections)
	{
		if (c.first != currentCid)
		{
			sentNewConnectionMessage(event.peer, c.second, c.first);
		}

	}

	addCidToServerSettings(currentCid);

	//send this to others
	Client c = *getClientNotLocked(currentCid);
	broadcastNewConnectionMessage(event.peer, c, currentCid);

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

			broadCastNotLocked(p, 0, 0, 0, true, channelHandleConnections);

			lockConnectionsMutex();
			connections.erase(connections.find(c.first));
			unlockConnectionsMutex();
			break;
		}
	}


}

void recieveData(ENetHost *server, ENetEvent &event)
{
	Packet p;
	size_t size = 0;
	auto data = parsePacket(event, p, size);

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
				connectionsMutex.lock();
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
					squareDistance = it->second.playerData.chunkDistance;
				}
				connectionsMutex.unlock();
			}

			if (!error)
			{
				serverTask.t.type = Task::generateChunk;
				serverTask.t.pos = {packetData.chunkPosition.x, 0, packetData.chunkPosition.y};

				submitTaskForServer(serverTask);
			}
			
			
			
			break;
		}

		case headerPlaceBlock:
		{
			Packet_PlaceBlock packetData = *(Packet_PlaceBlock *)data;
			serverTask.t.type = Task::placeBlock;
			serverTask.t.pos = {packetData.blockPos};
			serverTask.t.blockType = {packetData.blockType};
			serverTask.t.eventId = packetData.eventId;

			submitTaskForServer(serverTask);
			break;
		}

		case headerSendPlayerData:
		{
			Packer_SendPlayerData packetData = *(Packer_SendPlayerData *)data;


			Client clientCopy = {};

			connectionsMutex.lock();
			
			auto it = connections.find(p.cid);
			if (it == connections.end())
			{
				reportError("invalid cid error in headerSendPlayerData");
			}
			else
			{

				if (it->second.playerData.position != packetData.playerData.position)
				{


					clientCopy = it->second;

					//todo something better here...
					auto s = getServerSettingsCopy();
					s.perClientSettings[p.cid].outPlayerPos = packetData.playerData.position;
					setServerSettings(s);
				}
				else
				{
					//nothing changed
				}
				it->second.playerData = packetData.playerData;

			}

			connectionsMutex.unlock();

			if (packetData.timer + 16 * 4 > getTimer())
			{

				//broadcast player movement
				if (clientCopy.entityId)
				{
					Packet_ClientRecieveOtherPlayerPosition sendData;
					sendData.entityId = clientCopy.entityId;
					sendData.position = packetData.playerData.position;

					Packet p;
					p.cid = 0;
					p.header = headerClientRecieveOtherPlayerPosition;

					broadCast(p, &sendData, sizeof(sendData), clientCopy.peer, false, channelPlayerPositions);
				}

			}

			break;
		}

		case headerClientDroppedItem:
		{
			if (size != sizeof(Packet_ClientDroppedItem))
			{
				//error checking + cick clients that send corrupted data?
				reportError("corrupted packet or something Packet_ClientDroppedItem");
				break;
			}

			Packet_ClientDroppedItem *packetData = (Packet_ClientDroppedItem *)data;

			serverTask.t.type = Task::droppedItemEntity;
			serverTask.t.doublePos = packetData->position;
			serverTask.t.blockCount = packetData->count;
			serverTask.t.blockType = packetData->blockType;
			serverTask.t.entityId = packetData->entityID;
			serverTask.t.eventId = packetData->eventId;
			serverTask.t.motionState = packetData->motionState;
			serverTask.t.timer = packetData->timer;

			submitTaskForServer(serverTask);
			break;

		}

		default:

		break;
	}





}

void signalWaitingFromServer()
{
	taskCondition.notify_one();
}

//todo remove
std::vector<ServerTask> waitForTasksServer()
{
	std::unique_lock<std::mutex> locker(taskMutex);
	if (tasks.empty())
	{
		taskCondition.wait(locker);
	}

	auto size = tasks.size();
	std::vector<ServerTask> retVector;
	retVector.reserve(size);

	for (int i = 0; i < size; i++)
	{
		retVector.push_back(tasks.front());
		tasks.pop();
	}

	locker.unlock();

	return retVector;
}

std::vector<ServerTask> tryForTasksServer()
{
	std::unique_lock<std::mutex> locker(taskMutex);
	if (tasks.empty())
	{
		locker.unlock();
		return {};
	}

	auto size = tasks.size();
	std::vector<ServerTask> retVector;
	retVector.reserve(size);

	for (int i = 0; i < size; i++)
	{
		retVector.push_back(tasks.front());
		tasks.pop();
	}

	locker.unlock();

	return retVector;
}

static std::atomic<bool> enetServerRunning = false;

void enetServerFunction()
{
	std::cout << "Successfully started server!\n";


	StructuresManager structuresManager;
	BiomesManager biomesManager;
	WorldSaver worldSaver;

	worldSaver.savePath = RESOURCES_PATH "saves/";
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
		while (enet_host_service(server, &event, waitTime) > 0 && enetServerRunning)
		{
			//we wait only the first time, than we want to let the server update happen.
			waitTime = 0;

			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					addConnection(server, event);

					std::cout << "Successfully connected!\n";

					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					recieveData(server, event);

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
								{c.second.playerData.position.x, c.second.playerData.position.z},
								other.second.playerData.position, c.second.playerData.chunkDistance, 5)
								)
							{
								Packet_ClientRecieveOtherPlayerPosition sendData;
								sendData.entityId = other.second.entityId;
								sendData.position = other.second.playerData.position;

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
		worldSaver, deltaTime);

	
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
	pids = 1;

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
