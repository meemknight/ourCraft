#pragma once

#include "structureBaseBlock.h"
#include <unordered_map>




std::uint16_t 
getBlockPosInChunkToHashValue(unsigned char x, unsigned char y, unsigned char z);

struct BlocksWithDataHolder
{

	std::unordered_map<std::uint16_t, BaseBlock> baseBlocks;

	BaseBlock *getBaseBlock(unsigned char x, unsigned char y, unsigned char z);
	BaseBlock *getOrCreateBaseBlock(unsigned char x, unsigned char y, unsigned char z);

};
