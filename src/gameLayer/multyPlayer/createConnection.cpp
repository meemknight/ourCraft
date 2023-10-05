#include "multyPlayer/createConnection.h"
#include "enet/enet.h"
#include "multyPlayer/packet.h"
#include <iostream>

//todo rename task client
void submitTaskClient(Task &t)
{
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
		packetData.eventId = t.eventId;

		sendPacket(data.server, p, (char *)&packetData, sizeof(packetData), 1, 1);
		break;
	}
	default:

	break;
	}


}

void submitTaskClient(std::vector<Task> &t)
{
	for (auto &i : t)
	{
		submitTaskClient(i);
	}
}


static ConnectionData clientData;
std::vector<Chunk *> getRecievedChunks()
{
	//auto ret = std::move(recievedChunks);
	auto ret = clientData.recievedChunks;
	clientData.recievedChunks.clear();
	return ret;
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


void recieveDataClient(ENetEvent &event, EventCounter &validatedEvent, RevisionNumber &invalidateRevision)
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
			clientData.recievedChunks.push_back(c);

			break;
		}

		case headerPlaceBlock:
		{
			Packet_PlaceBlocks b;
			b.blockPos = ((Packet_PlaceBlock *)data)->blockPos;
			b.blockType = ((Packet_PlaceBlock *)data)->blockType;
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

		case headerInValidateEvent:
		{
			invalidateRevision = std::max(invalidateRevision, ((Packet_InValidateEvent *)data)->eventId.revision);
			break;
		}

		default:
		break;

	}


}

//this is not multy threaded
void clientMessageLoop(EventCounter &validatedEvent, RevisionNumber &invalidateRevision)
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

					recieveDataClient(event, validatedEvent, invalidateRevision);

					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				{
					std::cout << "Disconected from server\n";
					exit(0);

					break;
				}

			}

			enet_packet_destroy(event.packet);

		}
		else
		{
			break;
		}
	}

}

void closeConnection()
{

	if (!clientData.conected) { return; }

	if(clientData.server)
	enet_peer_reset(clientData.server);
	
	if (clientData.client)
	enet_host_destroy(clientData.client);

	for (auto &i : clientData.recievedChunks)
	{
		delete i;
	}

	clientData = {};

}

bool createConnection()
{
	if (clientData.conected) { return false; }

	clientData = ConnectionData{};

	clientData.client = enet_host_create(nullptr, 1, 1, 0, 0);

	ENetAddress adress = {};
	ENetEvent event = {};
	
	enet_address_set_host(&adress, "127.0.0.1");
	adress.port = 7771; //todo port stuff

	//client, adress, channels, data to send rightAway
	clientData.server = enet_host_connect(clientData.client, &adress, SERVER_CHANNELS, 0);

	if (clientData.server == nullptr)
	{
		enet_host_destroy(clientData.client);
		return false;
	}

	//see if we got events by server
	//client, event, ms to wait(0 means that we don't wait)
	if (enet_host_service(clientData.client, &event, 3000) > 0
		&& event.type == ENET_EVENT_TYPE_CONNECT)
	{
		std::cout << "client connected\n";
	}
	else
	{
		std::cout << "server timeout\n";
		enet_peer_reset(clientData.server);
		enet_host_destroy(clientData.client);
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
			enet_host_destroy(clientData.client);
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
		enet_peer_reset(clientData.server);
		enet_host_destroy(clientData.client);
		std::cout << "server handshake timeout\n";
		return 0;
	}

	#pragma endregion

	clientData.conected = true;
	return true;
}

