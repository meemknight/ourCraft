#include "enetServerFunction.h"
#include <atomic>
#include <thread>
#include <enet/enet.h>
#include <iostream>
#include <vector>
#include "packet.h"
#include <unordered_map>
#include "threadStuff.h"
#include <mutex>
#include <queue>

ENetHost *server = 0;
std::mutex connectionsMutex;
std::unordered_map<int32_t, Client> connections;
int pids = 1;


Client getClient(int32_t cid)
{
	connectionsMutex.lock();
	Client rez = connections[cid];
	connectionsMutex.unlock();
	return rez;
}

void createConnection(int32_t cid, Client c)
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

void addConnection(ENetHost *server, ENetEvent &event)
{
	createConnection(pids, Client{event.peer});

	Packet p;
	p.header = headerReceiveCIDAndData;
	p.cid = pids;

	pids++;

	Packet_ReceiveCIDAndData packetToSend = {};

	//send own cid
	sendPacket(event.peer, p, (const char *)&packetToSend, sizeof(packetToSend), true, 0);

	//todo send info of other players to this new connection

	//broadcast data of new connection
	

}

void recieveData(ENetHost *server, ENetEvent &event)
{
	Packet p;
	size_t size = 0;
	auto data = parsePacket(event, p, size);

	//validate data
	//no need for mutex here fortunatelly
	if (connections[p.cid].peer != event.peer)
	{
		std::cout << "invalid data!\n";
		return;
	}

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
			serverTask.t.type = Task::generateChunk;
			serverTask.t.pos = {packetData.blockPos};
			serverTask.t.blockType = {packetData.blockType};

			submitTaskForServer(serverTask);
			break;
		}

		default:

		break;
	}



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


void enetServerFunction()
{
	std::cout << "Successfully started server!\n";

	ENetEvent event = {};

	while (true)
	{
		while (enet_host_service(server, &event, 0) > 0)
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


static std::atomic<bool> enetServerRunning = false;

bool startEnetListener(ENetHost *_server)
{
	server = _server;
	connections = {};
	pids = 1;

	bool expected = 0;
	if (enetServerRunning.compare_exchange_strong(expected, 0))
	{
		std::thread serverThread(enetServerFunction);
		serverThread.detach();
		return 1;
	}
	else
	{
		return 0;
	}
}