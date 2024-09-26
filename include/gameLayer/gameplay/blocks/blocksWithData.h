#pragma once

#include "structureBaseBlock.h"
#include <unordered_map>




std::uint16_t 
fromBlockPosInChunkToHashValue(unsigned char x, unsigned char y, unsigned char z);
glm::ivec3 fromHashValueToBlockPosinChunk(std::uint16_t hashValue);

struct BlocksWithDataHolder
{

	std::unordered_map<std::uint16_t, BaseBlock> baseBlocks;

	BaseBlock *getBaseBlock(unsigned char x, unsigned char y, unsigned char z);
	BaseBlock *getOrCreateBaseBlock(unsigned char x, unsigned char y, unsigned char z);

};


struct InteractionData
{
	unsigned char blockInteractionType = 0;
	glm::ivec3 blockInteractionPosition = {0, -1, 0};
	BaseBlock baseBlockHolder = {};
};