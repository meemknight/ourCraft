#include "createConnection.h"
#include "enet/enet.h"
#include "packet.h"
#include <iostream>

ConnectionData clientData;

ConnectionData getConnectionData()
{
	return clientData;
}

void clientMessageLoop()
{
	ENetEvent event;
	if (enet_host_service(clientData.client, &event, 0) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_RECEIVE:
			{


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
}

bool createConnection()
{
	clientData = {};

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

		auto recievedData = *(Packet_headerReceiveCIDAndData *)data;
		
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

