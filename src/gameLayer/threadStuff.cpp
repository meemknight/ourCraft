#include "threadStuff.h"
#include <thread>
#include <mutex>
#include <queue>
#include "worldGenerator.h"
#include <thread>
#include <unordered_map>
#include <iostream>
#include <packet.h>
#include "createConnection.h"

void submitTaskClient(Task& t)
{
	//std::unique_lock<std::mutex> locker(taskMutex);
	//tasks.push(t);
	//locker.unlock();
	//taskCondition.notify_one();
	auto data = getConnectionData();

	Packet p;
	p.cid = data.cid;
	
	switch (t.type)
	{
		case Task::generateChunk:
		{
			p.header = headerRequestChunk;
			Packet_RequestChunk packetData = {};
			packetData.chunkPosition = {t.pos.x, t.pos.z};

			sendPacket(data.server, p, (char *)&packetData, sizeof(packetData), 1, 0);
			break;
		}
		case Task::placeBlock:
		{
			p.header = headerPlaceBlock;
			Packet_PlaceBlock packetData = {};
			packetData.blockPos = t.pos;
			packetData.blockType = t.blockType;

			sendPacket(data.server, p, (char *)&packetData, sizeof(packetData), 1, 1);
			break;
		}
		default:

		break;
	}
	

}

void submitTaskClient(std::vector<Task>& t)
{
	for (auto& i : t)
	{
		submitTaskClient(i);
	}
}


//std::mutex chunkMutex;
//std::queue<Message> chunks;
//
//void submitMessage(Message m)
//{
//	chunkMutex.lock();
//
//	chunks.push(m);
//
//	chunkMutex.unlock();
//}

//std::vector<Message> getMessages()
//{
//	std::vector<Message> retVector;
//
//	chunkMutex.lock();
//
//	auto size = chunks.size();
//	for (int i = 0; i < size; i++)
//	{
//		retVector.push_back(chunks.front());
//		chunks.pop();
//	}
//
//	chunkMutex.unlock();
//
//	return retVector;
//}

