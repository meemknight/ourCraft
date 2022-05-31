#pragma once
#include <enet/enet.h>


bool startEnetListener(ENetHost *_server);

struct Client
{
	ENetPeer *peer = {};
	//phisics::Entity entityData = {};
	//bool changed = 1;
	//char clientName[56] = {};
};