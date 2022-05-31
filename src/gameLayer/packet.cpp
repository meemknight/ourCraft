#include "packet.h"
#include <algorithm>


void sendPacket(ENetPeer *to, Packet p, const char *data, size_t size, bool reliable, int channel)
{

	//resize(size + sizeof(Packet));

	char *dataPool = new char[size + sizeof(Packet)];
	//size_t dataPoolSize;


	memcpy(dataPool, &p, sizeof(Packet));

	if (data && size)
	{
		memcpy(dataPool + sizeof(Packet), data, size);
	}

	size_t flag = 0;

	if (reliable)
	{
		flag = ENET_PACKET_FLAG_RELIABLE;
	}
	else
	{
		flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;// | ENET_PACKET_FLAG_UNSEQUENCED;
	}

	ENetPacket *packet = enet_packet_create(dataPool, size + sizeof(Packet), flag);
	enet_peer_send(to, channel, packet);

	delete[] dataPool;

}

char *parsePacket(ENetEvent &event, Packet &p, size_t &dataSize)
{
	size_t size = event.packet->dataLength;
	void *data = event.packet->data;
	dataSize = std::max(size_t(0), size - sizeof(Packet));

	memcpy(&p, data, sizeof(Packet));

	if (size <= sizeof(Packet))
	{
		return nullptr;
	}
	else
	{
		return (char *)data + sizeof(Packet);
	}

}
