#pragma once

#include <enet/enet.h>
#include <multyPlayer/packet.h>
#include <vector>
#include <unordered_set>

struct Client
{
	ENetPeer *peer = {};
	//phisics::Entity entityData = {};
	//bool changed = 1;
	//char clientName[56] = {};
	RevisionNumber revisionNumber = 1;

	PlayerServer playerData;

	std::vector<unsigned char> skinData;
	bool skinDataCompressed = false;

	std::unordered_set<glm::ivec2, Ivec2Hash> loadedChunks;

	std::unordered_set<unsigned int> chunksPacketPendingConfirmation;
};