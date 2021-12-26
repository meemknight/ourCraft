#pragma once
#include <memory.h>
#include <vector>

enum BlockTypes
{
	air = 0,
	grass,
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
	BlocksCount
};

struct Block
{
	short type;
	char lightLevel; //first 4 bytes represent the sun level and bottom 4 bytes the other lights level
	char notUsed;

	bool air() { return type == 0; }
	bool isOpaque()
	{
		return
			type != BlockTypes::air;
	}
};

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;

struct Chunk
{

	Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_HEIGHT];
	int x, z;
	
	std::vector<int> opaqueGeometry;
	char dirty = 1;
	char neighbourToLeft = 0;
	char neighbourToRight = 0;
	char neighbourToFront = 0;
	char neighbourToBack = 0;


	void clear()
	{
		memset(blocks, 0, sizeof(blocks));
	}

	Block& unsafeGet(int x, int y, int z)
	{
		return blocks[x][z][y];
	}

	Block* safeGet(int x, int y, int z);

	//todo will use a gpu buffer in the future
	//returns true if did work
	bool bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back);

	void create(int x, int y);
};
