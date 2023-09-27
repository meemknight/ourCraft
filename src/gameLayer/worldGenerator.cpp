#include "worldGenerator.h"
#include "FastNoiseSIMD.h"
#include "FastNoise/FastNoise.h"
#include <cmath>



void generateChunk(int seed, Chunk &c, WorldGenerator &wg)
{
	generateChunk(seed, c.data, wg);
}


void generateChunk(int seed, ChunkData& c, WorldGenerator &wg)
{
	c.clear();
	
	int xPadd = c.x * 16;
	int zPadd = c.z * 16;

	float* testNoise
		= wg.continentalnessNoise->GetSimplexFractalSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	auto getNoiseVal = [testNoise](int x, int y, int z)
	{
		return testNoise[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
	};

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			int height = int(50 + getNoiseVal(x, 0, z) * 30);
			for (int y = 0; y < height; y++)
			{
				c.unsafeGet(x, y, z).type = BlockTypes::dirt;
			}
		}
	
	FastNoiseSIMD::FreeNoiseSet(testNoise);

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			int counter = 0;
			for (int y = CHUNK_HEIGHT - 1; y >= 0; y--)
			{
				if (counter == 0)
				{
					if (!c.unsafeGet(x, y, z).air())
					{
						c.unsafeGet(x, y, z).type = BlockTypes::grassBlock;
						counter = 1;
					}
				}
				else if (counter < 4)
				{
					counter++;
				}
				else
				{
					c.unsafeGet(x, y, z).type = BlockTypes::stone;
				}
			}
		}


}

