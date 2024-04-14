#pragma once
#include <gameplay/entity.h>


struct DroppedItem: public PhysicalEntity
{
	BlockType type = 0;
	unsigned char count = 0;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
};


struct DroppedItemClient: public CleintEntity<DroppedItem>
{
	
};

struct DroppedItemServer : public ServerEntity<DroppedItem>
{

};

