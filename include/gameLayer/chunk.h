#pragma once
#include "blocks.h"

constexpr int CHUNK_SIZE = 16;
constexpr int META_CHUNK_SIZE = 32;
constexpr int CHUNK_HEIGHT = 256;

struct ChunkData
{

	Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_HEIGHT];

	unsigned char cachedBiomes[CHUNK_SIZE][CHUNK_SIZE];

	int x, z;

	Block &unsafeGet(int x, int y, int z)
	{
		return blocks[x][z][y];
	}

	unsigned char &unsafeGetCachedBiome(int x, int z)
	{
		return cachedBiomes[x][z];
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

	Block &unsafeGet(int x, int y, int z)
	{
		return data.blocks[x][z][y];
	}

	Block *safeGet(int x, int y, int z);

	//returns true if it changed anything, it will also return true if the newly baked
	//geometry is 0 because that means that it took very little time.
	bool bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back,
		glm::ivec3 playerPosition);

	bool shouldBakeOnlyBecauseOfTransparency(Chunk *left, Chunk *right, Chunk *front, Chunk *back);

	void createGpuData();

	void clearGpuData();
};
