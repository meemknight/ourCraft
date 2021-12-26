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
			type == grass ||
			type == dirt ||
			type == stone ||
			type == ice;
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
		x = 0;
		z = 0;
	}

	Block& unsafeGet(int x, int y, int z)
	{
		return blocks[x][z][y];
	}

	Block* safeGet(int x, int y, int z);

	//todo will use a gpu buffer in the future
	void bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back);

	void create(int x, int y);
};
