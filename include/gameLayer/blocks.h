#pragma once
#include <memory.h>
#include <vector>
#include <stdint.h>
#include <glad/glad.h>

enum BlockTypes
{
	air = 0,
	grassBlock,
	dirt,
	stone,
	ice,
	woodLog,
	wooden_plank,
	cobblestone,
	gold_block,
	bricks,
	sand,
	sand_stone,
	snow_dirt,
	leaves,
	gold_ore,
	coar_ore,
	stoneBrick,
	iron_ore,
	diamond_ore,
	bookShelf,
	birch_wood,
	gravel,
	grass,
	rose,
	BlocksCount
};

bool isBlockMesh(uint16_t type);

bool isCrossMesh(uint16_t type);

bool isOpaque(uint16_t type);

bool isGrassMesh(uint16_t type);

struct Block
{
	uint16_t type;
	unsigned char lightLevel; //first 4 bytes represent the sun level and bottom 4 bytes the other lights level
	unsigned char notUsed;

	bool air() { return type == 0; }
	bool isOpaque()
	{
		return ::isOpaque(type);
	}

	bool isAnimated()
	{
		return
			type == BlockTypes::leaves;
	}

	bool isGrassMesh()
	{
		return ::isGrassMesh(type);
	}

	unsigned char getSkyLight()
	{
		return (lightLevel >> 4);
	}

	void setSkyLevel(unsigned char s)
	{
		s = s & 0b1111;
		s <<= 4;
		lightLevel &= 0b0000'1111;
		lightLevel |= s;
	}

	bool isBlockMesh()
	{
		return ::isBlockMesh(type);
	}

	bool isCrossMesh()
	{
		return ::isCrossMesh(type);
	}

};


constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;

struct ChunkData
{
	Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_HEIGHT];
	int x, z;

	Block &unsafeGet(int x, int y, int z)
	{
		return blocks[x][z][y];
	}

	void clear()
	{
		memset(blocks, 0, sizeof(blocks));
	}

	//todo refactor
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

struct Chunk
{
	ChunkData data;
	
	//std::vector<int> opaqueGeometry;
	GLuint opaqueGeometryBuffer = 0;
	size_t elementCountSize = 0;

	char dirty = 1;
	char neighbourToLeft = 0;
	char neighbourToRight = 0;
	char neighbourToFront = 0;
	char neighbourToBack = 0;
	char dontDrawYet = 0; //first time ever don't draw it yet so we have time to process the light

	void clear()
	{
		memset(data.blocks, 0, sizeof(data.blocks));
	}

	Block& unsafeGet(int x, int y, int z)
	{
		return data.blocks[x][z][y];
	}

	Block* safeGet(int x, int y, int z);

	//todo will use a gpu buffer in the future
	//returns true if it changed anything
	bool bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back);

	void create(int x, int y);

	void createGpuData();

	void clearGpuData();
};
