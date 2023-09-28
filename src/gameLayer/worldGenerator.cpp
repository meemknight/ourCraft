#include "worldGenerator.h"
#include "FastNoiseSIMD.h"
#include "FastNoise/FastNoise.h"
#include <cmath>

constexpr int startLevel = 45;
constexpr int waterLevel = 65;
constexpr int maxMountainLevel = 220;
constexpr int heightDiff = maxMountainLevel - startLevel;

void calculateBlockPass1(int height, Block *startPos)
{

	if (waterLevel > height)
	{
		for (int y = waterLevel; y > height; y--)
		{
			startPos[y].type = BlockTypes::water;
		}
	
		for (int y = height; y >= 0; y--)
		{
			startPos[y].type = BlockTypes::stone;
		}
	}
	else
	{
		startPos[height].type = BlockTypes::grassBlock;

		for (int y = height-1; y > height-4; y--)
		{
			startPos[y].type = BlockTypes::dirt;
		}

		for (int y = height - 4; y >= 0; y--)
		{
			startPos[y].type = BlockTypes::stone;
		}
	}
}

void generateChunk(int seed, Chunk &c, WorldGenerator &wg)
{
	generateChunk(seed, c.data, wg);
}


void generateChunk(int seed, ChunkData& c, WorldGenerator &wg)
{
	c.clear();
	
	int xPadd = c.x * 16;
	int zPadd = c.z * 16;

	float* continentalness
		= wg.continentalnessNoise->GetSimplexFractalSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *peaksAndValies
		= wg.peaksValiesNoise->GetSimplexFractalSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *oceansAndTerases
		= wg.peaksValiesNoise->GetSimplexFractalSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		continentalness[i] += 1;
		continentalness[i] /= 2;
		continentalness[i] = std::pow(continentalness[i], wg.continentalPower);
		continentalness[i] = wg.continentalSplines.applySpline(continentalness[i]);
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		peaksAndValies[i] += 1;
		peaksAndValies[i] /= 2;
		peaksAndValies[i] = std::pow(peaksAndValies[i], wg.peaksValiesPower);

		float val = peaksAndValies[i];

		peaksAndValies[i] = wg.peaksValiesSplines.applySpline(peaksAndValies[i]);

		continentalness[i] = lerp(continentalness[i], peaksAndValies[i], wg.peaksValiesContributionSplines.applySpline(val));
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		oceansAndTerases[i] += 1;
		oceansAndTerases[i] /= 2;
		oceansAndTerases[i] = std::pow(oceansAndTerases[i], wg.oceansAndTerasesPower);

		float val = oceansAndTerases[i];

		oceansAndTerases[i] = wg.oceansAndTerasesSplines.applySpline(oceansAndTerases[i]);

		continentalness[i] = lerp(continentalness[i], oceansAndTerases[i], wg.oceansAndTerasesContributionSplines.applySpline(val));
	}

	auto getNoiseVal = [continentalness](int x, int y, int z)
	{
		return continentalness[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
	};


	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			int height = int(startLevel + getNoiseVal(x, 0, z) * heightDiff);
		
			calculateBlockPass1(height, &c.unsafeGet(x, 0, z));

		}
	
	FastNoiseSIMD::FreeNoiseSet(continentalness);
	FastNoiseSIMD::FreeNoiseSet(peaksAndValies);



}

