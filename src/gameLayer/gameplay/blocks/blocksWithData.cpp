#include <gameplay/blocks/blocksWithData.h>
#include <chunk.h>
#include <iostream>

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

ChestBlock *BlocksWithDataHolder::getChestBlock
(unsigned char x, unsigned char y, unsigned char z)
{
	auto hash = fromBlockPosInChunkToHashValue(x, y, z);

	auto found = chestBlocks.find(hash);

	if (found != chestBlocks.end())
	{
		return &found->second;
	}
	else
	{
		return nullptr;
	}
}

ChestBlock *BlocksWithDataHolder::getOrCreateChestBlock
(unsigned char x, unsigned char y, unsigned char z)
{
	auto hash = fromBlockPosInChunkToHashValue(x, y, z);

	auto found = chestBlocks.find(hash);

	if (found != chestBlocks.end())
	{
		return &found->second;
	}
	else
	{
		chestBlocks[hash] = {};
		found = chestBlocks.find(hash);
		return &found->second;
	}
}


void appendBaseBlock(std::vector<unsigned char> &dataToAppend, 
	glm::ivec3 position, BaseBlock &baseBlock)
{

	size_t headerStart = dataToAppend.size();
	dataToAppend.resize(dataToAppend.size() + sizeof(BlockDataHeader));

	//write the data
	size_t wroteData = baseBlock.formatIntoData(dataToAppend);

	BlockDataHeader header = {};

	header.pos = position;
	header.blockType = BlockTypes::structureBase;
	header.dataSize = wroteData;

	std::memcpy(dataToAppend.data() + headerStart, &header, sizeof(header));

}

void appendChestBlock(std::vector<unsigned char> &dataToAppend,
	glm::ivec3 position, ChestBlock &chestBlock)
{
	size_t headerStart = dataToAppend.size();
	dataToAppend.resize(dataToAppend.size() + sizeof(BlockDataHeader));

	//write the data
	size_t wroteData = chestBlock.formatIntoData(dataToAppend);

	BlockDataHeader header = {};

	header.pos = position;
	header.blockType = BlockTypes::woddenChest;
	header.dataSize = wroteData;

	std::memcpy(dataToAppend.data() + headerStart, &header, sizeof(header));

}


void BlocksWithDataHolder::formatBlockData
	(std::vector<unsigned char> &dataToAppend, int chunkXChunkSpace, int chunkZChunkSpace)
{

	size_t extraSize = baseBlocks.size() * (sizeof(BaseBlock) + sizeof(BlockDataHeader));

	extraSize += chestBlocks.size() * sizeof(Item) * CHEST_CAPACITY; //estimate


	glm::ivec3 chunkBlockPos = glm::ivec3(chunkXChunkSpace * CHUNK_SIZE, 0, chunkZChunkSpace * CHUNK_SIZE);

	if (extraSize)
	{
		dataToAppend.reserve(dataToAppend.size() + extraSize);

		for (auto &b : baseBlocks)
		{
			glm::ivec3 pos = chunkBlockPos + fromHashValueToBlockPosinChunk(b.first);
			appendBaseBlock(dataToAppend, pos, b.second);
		}

		for (auto &b : chestBlocks)
		{
			glm::ivec3 pos = chunkBlockPos + fromHashValueToBlockPosinChunk(b.first);
			appendChestBlock(dataToAppend, pos, b.second);
		}

	}

}

void BlocksWithDataHolder::loadBlockData(std::vector<unsigned char> &data,
	int chunkXChunkSpace, int chunkZChunkSpace)
{

	*this = {};

	int pointer = 0;

	while (data.size() - pointer >= sizeof(BlockDataHeader))
	{

		BlockDataHeader header;
		memcpy(&header, data.data() + pointer, sizeof(BlockDataHeader));
		pointer += sizeof(BlockDataHeader);
		
		auto pos = header.pos;
		auto size = header.dataSize;

		glm::ivec3 posInChunk = pos;
		posInChunk.x = modBlockToChunk(posInChunk.x);
		posInChunk.z = modBlockToChunk(posInChunk.z);

		auto chunkPosX = divideChunk(pos.x);
		auto chunkPosZ = divideChunk(pos.z);

		if (chunkXChunkSpace != chunkPosX && chunkZChunkSpace != chunkPosZ)
		{
			std::cout << "Error chunk index size in loadBlockData!\n";
			break;
		}

		if (size > data.size() - pointer)
		{
			std::cout << "Error size in loadBlockData!\n";
			break;
		}

		if (header.blockType == BlockTypes::structureBase)
		{
			
			BaseBlock b;
			size_t _ = 0;

			if (!b.readFromBuffer(data.data() + pointer, size, _))
			{
				std::cout << "Error read from buffer in loadBlockData!\n";
				break;
			}
			
			pointer += size;

			if (!b.isDataValid())
			{
				std::cout << "Error is data valid in loadBlockData base block!\n";
				break;
			}

			baseBlocks[fromBlockPosInChunkToHashValue(posInChunk.x, posInChunk.y, posInChunk.z)] =
				b;


		}else if (header.blockType == BlockTypes::woddenChest)
		{
			ChestBlock c;
			size_t _ = 0;

			if (!c.readFromBuffer(data.data() + pointer, size, _))
			{
				std::cout << "Error read from buffer in loadBlockData!\n";
				break;
			}

			pointer += size;

			if (!c.isDataValid())
			{
				std::cout << "Error is data valid in loadBlockData chests!\n";
				break;
			}

			chestBlocks[fromBlockPosInChunkToHashValue(posInChunk.x, posInChunk.y, posInChunk.z)] = 
				c;

		}
		else
		{
			std::cout << "Error in loadBlockData!\n";
			break;
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
