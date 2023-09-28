#pragma once
#include <memory.h>
#include <vector>
#include <stdint.h>
#include <glad/glad.h>
#include <glm/vec3.hpp>
struct WorldGenerator;

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
	water,
	BlocksCount
};

using BlockType = uint16_t;

bool isBlockMesh(BlockType type);

bool isCrossMesh(BlockType type);

bool isOpaque(BlockType type);

bool isTransparentGeometry(BlockType type);

bool isGrassMesh(BlockType type);

struct Block
{
	BlockType type;
	unsigned char lightLevel; //first 4 bytes represent the sun level and bottom 4 bytes the other lights level
	unsigned char notUsed;

	bool air() { return type == 0; }
	bool isOpaque()
	{
		return ::isOpaque(type);
	}

	bool isAnimatedBlock()
	{
		return
			type == BlockTypes::leaves;
	}

	bool isTransparentGeometry()
	{
		return ::isTransparentGeometry(type);
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

struct Chunk
{
	ChunkData data;
	
	//std::vector<int> opaqueGeometry;
	GLuint opaqueGeometryBuffer = 0;
	GLuint vao = 0;
	size_t elementCountSize = 0;

	GLuint transparentGeometryBuffer = 0;
	GLuint transparentVao = 0;
	size_t transparentElementCountSize = 0;

	char dirty = 1;
	char dirtyTransparency = 1;
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

	//returns true if it changed anything, it will also return true if the newly baked
	//geometry is 0 because that means that it took very little time.
	bool bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back,
		glm::ivec3 playerPosition);

	bool shouldBakeOnlyBecauseOfTransparency(Chunk *left, Chunk *right, Chunk *front, Chunk *back);

	//void create(int x, int y, WorldGenerator &wg);

	void createGpuData();

	void clearGpuData();
};
