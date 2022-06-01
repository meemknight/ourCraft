#include "createConnection.h"
#include "enet/enet.h"
#include "packet.h"
#include <iostream>

ConnectionData clientData;
std::vector<Chunk *> recievedChunks;

std::vector<Chunk *> getRecievedChunks()
{
	//auto ret = std::move(recievedChunks);
	auto ret = recievedChunks;
	recievedChunks.clear();
	return ret;
}

ConnectionData getConnectionData()
{
	return clientData;
}


void recieveDataClient(ENetEvent &event)
{
	Packet p;
	size_t size = 0;
	auto data = parsePacket(event, p, size);

	switch(p.header)
	{
		case headerRecieveChunk:
		{
			Packet_RecieveChunk *chunkPacket = (Packet_RecieveChunk *)data;
			Chunk *c = new Chunk();
			c->data = chunkPacket->chunk;
			recievedChunks.push_back(c);

			break;
		}

		case headerPlaceBlock:
		{


			break;
		}


		default:
		break;

	}


}

//this is not multy threaded
void clientMessageLoop()
{
	ENetEvent event;

	for (int i = 0; i < 20; i++)
	{
		if (enet_host_service(clientData.client, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
			{


				recieveDataClient(event);


				enet_packet_destroy(event.packet);
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:
			{
				std::cout << "Disconected from server\n";
				exit(0);

				break;
			}

			}
		}
		else
		{
			break;
		}
	}

}

bool createConnection()
{
	clientData = {};
	recievedChunks = {};

	clientData.client = enet_host_create(nullptr, 1, 1, 0, 0);

	ENetAddress adress = {};
	ENetEvent event = {};
	
	enet_address_set_host(&adress, "127.0.0.1");
	adress.port = 7771; //todo port stuff

	//client, adress, channels, data to send rightAway
	clientData.server = enet_host_connect(clientData.client, &adress, SERVER_CHANNELS, 0);

	if (clientData.server == nullptr)
	{
		return false;
	}

	//see if we got events by server
	//client, event, ms to wait(0 means that we don't wait)
	if (enet_host_service(clientData.client, &event, 5000) > 0
		&& event.type == ENET_EVENT_TYPE_CONNECT)
	{
		std::cout << "client connected\n";
	}
	else
	{
		std::cout << "server timeout\n";
		enet_peer_reset(clientData.server);
		return false;
	}
	
	#pragma region handshake
	
	if (enet_host_service(clientData.client, &event, 5000) > 0
		&& event.type == ENET_EVENT_TYPE_RECEIVE)
	{
		Packet p = {};
		size_t size;
		auto data = parsePacket(event, p, size);

		if (p.header != headerReceiveCIDAndData)
		{
			enet_peer_reset(clientData.server);
			std::cout << "server sent wrong data\n";
			return false;
		}

		clientData.cid = p.cid;

		auto recievedData = *(Packet_ReceiveCIDAndData *)data;
		
		//send player own info or sthing
		//sendPlayerData(e, true);

		std::cout << "received cid: " << clientData.cid << "\n";
		enet_packet_destroy(event.packet);
		return true;
	}
	else
	{
		std::cout << "server handshake timeout\n";
		return 0;
	}

	#pragma endregion



	return true;
}

