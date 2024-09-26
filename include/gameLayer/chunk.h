#pragma once
#include "blocks.h"
#include <bitset>
#include <glm/glm.hpp>

#include <metrics.h>
#include <gameplay/blocks/blocksWithData.h>

struct BigGpuBuffer;


struct ChunkData
{

	Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_HEIGHT];


	//todo send this to the player and save it in the future, or just remove it
	//also save it.
	//unsigned char cachedBiomes[CHUNK_SIZE][CHUNK_SIZE];
	unsigned char flags[CHUNK_SIZE][CHUNK_SIZE] = {};
	float vegetation = 0;

	void setBorderFlag(int x, int z)
	{
		flags[x][z] |= 0b1;
	}

	bool isBorder(int x, int z)
	{
		return flags[x][z] & 0b1;
	}

	int x, z;

	Block &unsafeGet(int x, int y, int z)
	{
		return blocks[x][z][y];
	}

	//unsigned char &unsafeGetCachedBiome(int x, int z)
	//{
	//	return cachedBiomes[x][z];
	//}

	void clear()
	{
		memset(blocks, 0, sizeof(blocks));
	}

	Block *safeGet(glm::ivec3 b)
	{
		return safeGet(b.x, b.y, b.z);
	}

	Block *safeGet(int x, int y, int z)
	{
		if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || z >= CHUNK_SIZE || y >= CHUNK_HEIGHT)
		{
			return nullptr;
		}
		else
		{
			return &unsafeGet(x, y, z);
		}
	}

	void clearLightLevels();
};

#define DECLARE_FLAG(N, I)	bool is##N () { return flags[ I ]; } \
void set##N (bool flag) { flags[ I ] = flag; }

//this is for the client
struct Chunk
{
	Chunk()
	{
		setDirty(true);
		setDirtyTransparency(true);
	}

	ChunkData data;

	BlocksWithDataHolder blockData;


	GLuint opaqueGeometryBuffer = 0;
	GLuint opaqueGeometryIndex = 0;
	GLuint vao = 0;
	size_t elementCountSize = 0;

	GLuint transparentGeometryBuffer = 0;
	GLuint transparentGeometryIndex = 0;
	GLuint transparentVao = 0;
	size_t transparentElementCountSize = 0;

	GLuint lightsBuffer = 0;
	size_t lightsElementCountSize = 0;

	std::bitset<16> flags = {};
	
	DECLARE_FLAG(Dirty, 0);
	DECLARE_FLAG(DirtyTransparency, 1);
	DECLARE_FLAG(NeighbourToLeft, 2);
	DECLARE_FLAG(NeighbourToRight, 3);
	DECLARE_FLAG(NeighbourToFront, 4);
	DECLARE_FLAG(NeighbourToBack, 5);
	DECLARE_FLAG(DontDrawYet, 6);
	DECLARE_FLAG(Culled, 7);



	void clear()
	{
		memset(data.blocks, 0, sizeof(data.blocks));
	}

	Block &unsafeGet(int x, int y, int z)
	{
		return data.blocks[x][z][y];
	}

	Block *safeGet(int x, int y, int z);

	//returns true if it changed anything, it will also return true if the newly baked
	//geometry is 0 because that means that it took very little time.
	bool bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back,
		Chunk *frontLeft, Chunk *frontRight, Chunk *backLeft, Chunk *backRight,
		glm::ivec3 playerPosition, BigGpuBuffer &gpuBuffer);

	bool shouldBakeOnlyBecauseOfTransparency(Chunk *left, Chunk *right, Chunk *front, Chunk *back);

	void createGpuData();

	void clearGpuData(BigGpuBuffer *gpuBuffer);

	void removeBlockDataFromThisPos(Block lastBlock, std::uint8_t x, 
		std::uint8_t y, std::uint8_t z);

	std::vector<unsigned char> getExtraDataForThisPosAndRemoveIt(Block lastBlock, std::uint8_t x,
		std::uint8_t y, std::uint8_t z);

	//the block already needs to be there!
	void addExtraDataToBlock(std::vector<unsigned char> &data, unsigned char x, unsigned char y, unsigned char z);
};

#undef DECLARE_FLAG



