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

	return true;
}

