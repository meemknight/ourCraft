#include "enetServerFunction.h"

#include <atomic>
#include <thread>
#include <enet/enet.h>
#include <iostream>


ENetHost *server = 0;

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
					//addConnection(server, event);

					std::cout << "Successfully connected!\n";

					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					//recieveData(server, event);

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