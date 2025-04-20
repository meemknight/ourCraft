#pragma once

#include "structureBaseBlock.h"
#include "chestBlock.h"
#include <unordered_map>
#include <blocks.h>



std::uint16_t
fromBlockPosInChunkToHashValue(unsigned char x, unsigned char y, unsigned char z);
glm::ivec3 fromHashValueToBlockPosinChunk(std::uint16_t hashValue);

//used for saving data
struct BlockDataHeader
{
	//todo hold the version inside the pos y or something
	glm::ivec3 pos = {};
	BlockType blockType = 0;
	std::uint16_t dataSize = 0;
};

struct BlocksWithDataHolder
{

	std::unordered_map<std::uint16_t, BaseBlock> baseBlocks;
	std::unordered_map<std::uint16_t, ChestBlock> chestBlocks;

	BaseBlock *getBaseBlock(unsigned char x, unsigned char y, unsigned char z);
	BaseBlock *getOrCreateBaseBlock(unsigned char x, unsigned char y, unsigned char z);


	ChestBlock *getChestBlock(unsigned char x, unsigned char y, unsigned char z);
	ChestBlock *getOrCreateChestBlock(unsigned char x, unsigned char y, unsigned char z);

	void formatBlockData(std::vector<unsigned char> &dataToAppend, int chunkXChunkSpace, int chunkZChunkSpace);

	void loadBlockData(std::vector<unsigned char> &data, int chunkXChunkSpace, int chunkZChunkSpace);

};


void appendChestBlock(std::vector<unsigned char> &dataToAppend,
	glm::ivec3 position, ChestBlock &chestBlock);
	
struct InteractionData
{
	unsigned char blockInteractionType = 0;
	unsigned short block = 0;
	glm::ivec3 blockInteractionPosition = {0, -1, 0};
	BaseBlock baseBlockHolder = {};
};