#include <gameplay/blocks/blocksWithData.h>


BaseBlock *BlocksWithDataHolder::getBaseBlock
(unsigned char x, unsigned char y, unsigned char z)
{
	auto hash = getBlockPosInChunkToHashValue(x, y, z);

	auto found = baseBlocks.find(hash);

	if (found != baseBlocks.end())
	{
		return &found->second;
	}
	else
	{
		return nullptr;
	}
}

BaseBlock *BlocksWithDataHolder::getOrCreateBaseBlock(unsigned char x, unsigned char y, unsigned char z)
{
	auto hash = getBlockPosInChunkToHashValue(x, y, z);

	auto found = baseBlocks.find(hash);

	if (found != baseBlocks.end())
	{
		return &found->second;
	}
	else
	{
		baseBlocks[hash] = {};
		found = baseBlocks.find(hash);
		return &found->second;
	}
}

std::uint16_t getBlockPosInChunkToHashValue(unsigned char x, unsigned char y, unsigned char z)
{
	assert(x < 16);
	assert(z < 16);

	std::uint16_t rezult = 0;

	rezult = x;
	rezult <<= 4;
	rezult |= z;
	rezult <<= 8;
	rezult |= y;
	return rezult;
}

glm::ivec3 fromHashValueToBlockPosinChunk(std::uint16_t hashValue)
{
	glm::ivec3 rez = {};

	rez.x = (hashValue & 0b1111'0000'0000'0000) >> 12;
	rez.z = (hashValue & 0b0000'1111'0000'0000) >> 8;
	rez.y = (hashValue & 0b0000'0000'1111'1111);

	return rez;
}
