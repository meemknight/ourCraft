#include "multyPlayer/packet.h"
#include <algorithm>


void sendPacket(ENetPeer *to, Packet p,
	const char *data, size_t size, bool reliable, int channel)
{
	size_t flag = 0;

	//memcpy(data, &p, sizeof(Packet));

	if (reliable)
	{
		flag = ENET_PACKET_FLAG_RELIABLE;
	}
	else
	{
		flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
	}

	ENetPacket *packet = enet_packet_create(nullptr, size + sizeof(packet), flag);

	memcpy(packet->data, &p, sizeof(Packet));
	memcpy(packet->data + sizeof(Packet), data, size);

	enet_peer_send(to, channel, packet);
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
