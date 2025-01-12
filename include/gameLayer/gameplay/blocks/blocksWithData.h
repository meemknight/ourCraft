#pragma once

#include "structureBaseBlock.h"
#include <unordered_map>
#include <blocks.h>



std::uint16_t
fromBlockPosInChunkToHashValue(unsigned char x, unsigned char y, unsigned char z);
glm::ivec3 fromHashValueToBlockPosinChunk(std::uint16_t hashValue);

//used for saving data
struct BlockDataHeader
{
	glm::ivec3 pos = {};
	BlockType blockType = 0;
	std::uint16_t dataSize = 0;
};

struct BlocksWithDataHolder
{

	std::unordered_map<std::uint16_t, BaseBlock> baseBlocks;

	BaseBlock *getBaseBlock(unsigned char x, unsigned char y, unsigned char z);
	BaseBlock *getOrCreateBaseBlock(unsigned char x, unsigned char y, unsigned char z);

	void formatBlockData(std::vector<unsigned char> &dataToAppend, int chunkXChunkSpace, int chunkZChunkSpace);

	void loadBlockData(std::vector<unsigned char> &data, int chunkXChunkSpace, int chunkZChunkSpace);

};


struct InteractionData
{
	unsigned char blockInteractionType = 0;
	glm::ivec3 blockInteractionPosition = {0, -1, 0};
	BaseBlock baseBlockHolder = {};
};