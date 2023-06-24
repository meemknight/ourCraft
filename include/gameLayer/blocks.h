#pragma once
#include <memory.h>
#include <vector>
#include <stdint.h>

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

struct Block
{
	uint16_t type;
	char lightLevel; //first 4 bytes represent the sun level and bottom 4 bytes the other lights level
	char notUsed;

	bool air() { return type == 0; }
	bool isOpaque()
	{
		return
			type != BlockTypes::air
			&& type != BlockTypes::leaves
			&& !(isGrassMesh());
	}

	bool isAnimated()
	{
		return
			type == BlockTypes::leaves;
	}

	bool isGrassMesh()
	{
		return type == BlockTypes::grass
			|| type == BlockTypes::rose
			;
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
};

struct Chunk
{
	ChunkData data;
	
	std::vector<int> opaqueGeometry;
	char dirty = 1;
	char neighbourToLeft = 0;
	char neighbourToRight = 0;
	char neighbourToFront = 0;
	char neighbourToBack = 0;


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
};
