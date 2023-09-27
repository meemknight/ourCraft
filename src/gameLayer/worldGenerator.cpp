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

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		testNoise[i] += 1;
		testNoise[i] /= 2;
		//testNoise[i] = std::pow(testNoise[i], wg.continentalPower);
		testNoise[i] = wg.continentalSplines.applySpline(testNoise[i]);
	}

	auto getNoiseVal = [testNoise](int x, int y, int z)
	{
		return testNoise[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
	};

	constexpr int startLevel = 45;
	constexpr int waterLevel = 65;

	constexpr int maxMountainLevel = 220;

	constexpr int heightDiff = maxMountainLevel - startLevel;

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			int height = int(startLevel + getNoiseVal(x, 0, z) * heightDiff);
			int y = 0;
			for (; y < height; y++)
			{
				c.unsafeGet(x, y, z).type = BlockTypes::stone;
			}

			for (; y < waterLevel; y++)
			{
				c.unsafeGet(x, y, z).type = BlockTypes::water;
			}

		}
	
	FastNoiseSIMD::FreeNoiseSet(testNoise);

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			int counter = 0;
			for (int y = CHUNK_HEIGHT - 1; y >= 0; y--)
			{
				if (c.unsafeGet(x, y, z).type == water) { break; }

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
					c.unsafeGet(x, y, z).type = BlockTypes::dirt;
					counter++;
				}
				else
				{
				}
			}
		}


}

