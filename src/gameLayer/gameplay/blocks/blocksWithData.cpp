#include <gameplay/blocks/blocksWithData.h>
#include <chunk.h>

BaseBlock *BlocksWithDataHolder::getBaseBlock
(unsigned char x, unsigned char y, unsigned char z)
{
	auto hash = fromBlockPosInChunkToHashValue(x, y, z);

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
	auto hash = fromBlockPosInChunkToHashValue(x, y, z);

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

void BlocksWithDataHolder::formatBlockData
	(std::vector<unsigned char> &dataToAppend, int chunkXChunkSpace, int chunkZChunkSpace)
{

	size_t extraSize = baseBlocks.size() * (sizeof(BaseBlock) + sizeof(BlockDataHeader));
	glm::ivec3 chunkBlockPos = glm::ivec3(chunkXChunkSpace * CHUNK_SIZE, 0, chunkZChunkSpace * CHUNK_SIZE);

	if (extraSize)
	{
		dataToAppend.reserve(dataToAppend.size() + extraSize);

		for (auto &b : baseBlocks)
		{

			size_t headerStart = dataToAppend.size();
			dataToAppend.resize(dataToAppend.size() + sizeof(BlockDataHeader));

			//write the data
			size_t wroteData = b.second.formatIntoData(dataToAppend);

			BlockDataHeader header = {};

			header.pos = chunkBlockPos + fromHashValueToBlockPosinChunk(b.first);
			header.blockType = BlockTypes::structureBase;
			header.dataSize = wroteData;

			std::memcpy(dataToAppend.data() + headerStart, &header, sizeof(header));
		}

	}

}

std::uint16_t fromBlockPosInChunkToHashValue(unsigned char x, unsigned char y, unsigned char z)
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
