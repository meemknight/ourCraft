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

//todo add to a struct
ENetHost *server = 0;
std::mutex connectionsMutex;
std::unordered_map<CID, Client> connections;
int pids = 1;
static std::thread serverThread;

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

Client getClient(CID cid)
{
	connectionsMutex.lock();
	Client rez = connections[cid];
	connectionsMutex.unlock();
	return rez;
}

std::unordered_map<CID, Client> getAllClients()
{
	connectionsMutex.lock();
	auto rez = connections;
	connectionsMutex.unlock();
	return rez;
}

void createConnection(CID cid, Client c)
{
	connectionsMutex.lock();
	connections.insert({cid, c});
	connectionsMutex.unlock();
}


std::mutex taskMutex;
std::condition_variable taskCondition;
std::queue<ServerTask> tasks;

void submitTaskForServer(ServerTask t)
{
	std::unique_lock<std::mutex> locker(taskMutex);
	tasks.push(t);
	locker.unlock();
	taskCondition.notify_one();
}

//todo remove conection
void addConnection(ENetHost *server, ENetEvent &event)
{
	createConnection(pids, Client{event.peer});

	Packet p;
	p.header = headerReceiveCIDAndData;
	p.cid = pids;

	pids++;

	Packet_ReceiveCIDAndData packetToSend = {};

	//send own cid
	sendPacket(event.peer, p, (const char *)&packetToSend, 
		sizeof(packetToSend), true, channelHandleConnections);

	//todo send info of other players to this new connection

	//broadcast data of new connection
	addCidToServerSettings(p.cid);

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

		std::cout << "invalid data!\n";
		return;
	}
	//connectionsMutex.unlock();


	ServerTask serverTask = {};
	serverTask.cid = p.cid;

	switch (p.header)
	{
		case headerRequestChunk:
		{
			Packet_RequestChunk packetData = *(Packet_RequestChunk *)data;
			serverTask.t.type = Task::generateChunk;
			serverTask.t.pos = {packetData.chunkPosition.x, 0, packetData.chunkPosition.y};

			submitTaskForServer(serverTask);
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

			connectionsMutex.lock();
			
			auto it = connections.find(p.cid);
			if (it == connections.end())
			{
				std::cout << "invalid cid error";
			}
			else
			{
				it->second.playerData = packetData.playerData;

				//todo something better here...
				auto s = getServerSettingsCopy();
				s.perClientSettings[p.cid].outPlayerPos = packetData.playerData.position;
				setServerSettings(s);
			}

			connectionsMutex.unlock();

		}


		default:

		break;
	}



}

void signalWaitingFromServer()
{
	taskCondition.notify_one();
}

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

	ENetEvent event = {};

	while (enetServerRunning)
	{
		while (enet_host_service(server, &event, 0) > 0 && enetServerRunning)
		{
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
					//removeConnection(server, event);
					break;
				}


			}
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
		serverThread = std::move(std::thread(enetServerFunction));
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
	serverThread.join();
}
