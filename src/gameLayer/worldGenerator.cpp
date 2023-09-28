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

	int y = height;
	bool noWater = 0;

	//find grass
	for (; y >= waterLevel; y--)
	{
		if (startPos[y].type != BlockTypes::air)
		{
			startPos[y].type = BlockTypes::grassBlock;

			for (y--; y > height - 4; y--)
			{
				if (startPos[y].type != BlockTypes::air)
				{
					startPos[y].type = BlockTypes::dirt;
				}
			}
			//noWater = true;
			break;
		}
	}

	if(!noWater)
	for (y = waterLevel; y > 0; y--)
	{
		if (startPos[y].type == BlockTypes::air)
		{
			startPos[y].type = BlockTypes::water;
		}
		else
		{
			break;
		}
	}

}

void generateChunk(Chunk &c, WorldGenerator &wg, StructuresManager &structuresManager, std::vector<StructureToGenerate> &generateStructures)
{
	generateChunk(c.data, wg, structuresManager, generateStructures);
}


void generateChunk(ChunkData& c, WorldGenerator &wg, StructuresManager &structuresManager, std::vector<StructureToGenerate> &generateStructures)
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

	float *densityNoise
		= wg.stone3Dnoise->GetSimplexFractalSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	float *vegetationNoise
		= wg.vegetationNoise->GetSimplexFractalSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		vegetationNoise[i] += 1;
		vegetationNoise[i] /= 2;
		vegetationNoise[i] = std::powf(vegetationNoise[i], wg.vegetationPower);
		vegetationNoise[i] = wg.vegetationSplines.applySpline(vegetationNoise[i]);
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		continentalness[i] += 1;
		continentalness[i] /= 2;
		continentalness[i] = std::powf(continentalness[i], wg.continentalPower);
		continentalness[i] = wg.continentalSplines.applySpline(continentalness[i]);
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		peaksAndValies[i] += 1;
		peaksAndValies[i] /= 2;
		peaksAndValies[i] = std::powf(peaksAndValies[i], wg.peaksValiesPower);

		float val = peaksAndValies[i];

		peaksAndValies[i] = wg.peaksValiesSplines.applySpline(peaksAndValies[i]);

		continentalness[i] = lerp(continentalness[i], peaksAndValies[i], wg.peaksValiesContributionSplines.applySpline(val));
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		oceansAndTerases[i] += 1;
		oceansAndTerases[i] /= 2;
		oceansAndTerases[i] = std::powf(oceansAndTerases[i], wg.oceansAndTerasesPower);

		float val = oceansAndTerases[i];

		oceansAndTerases[i] = wg.oceansAndTerasesSplines.applySpline(oceansAndTerases[i]);

		continentalness[i] = lerp(continentalness[i], oceansAndTerases[i], wg.oceansAndTerasesContributionSplines.applySpline(val));
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		densityNoise[i] += 1;
		densityNoise[i] /= 2;
		densityNoise[i] = std::powf(densityNoise[i], wg.stone3Dpower);
		densityNoise[i] = wg.stone3DnoiseSplines.applySpline(densityNoise[i]);
	}

	auto getNoiseVal = [continentalness](int x, int y, int z)
	{
		return continentalness[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
	};
	
	auto getVegetationNoiseVal = [vegetationNoise](int x, int z)
	{
		return vegetationNoise[x * CHUNK_SIZE + z];
	};

	auto getDensityNoiseVal = [densityNoise](int x, int y, int z) //todo more cache friendly operation here please
	{
		return densityNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT) + y * CHUNK_SIZE + z];
	};

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{

			constexpr int stoneNoiseStartLevel = 1;
		
			float heightPercentage = getNoiseVal(x, 0, z);
			int height = int(startLevel + heightPercentage * heightDiff);
		
			float firstH = 1;
			for (int y = 0; y < height; y++)
			{

				auto density = getDensityNoiseVal(x, y, z);
				float bias = (y - stoneNoiseStartLevel) / (height - 1.f - stoneNoiseStartLevel);

				bias = std::powf(bias, wg.densityBiasPower);
				//density = std::powf(density, wg.densityBiasPower);

				if (y < stoneNoiseStartLevel)
				{
					c.unsafeGet(x, y, z).type = BlockTypes::stone;
				}
				else
				{
					if (density > wg.densityBias * bias)
					{
						firstH = y;
						c.unsafeGet(x, y, z).type = BlockTypes::stone;
					}
				}

			}

			
			calculateBlockPass1(firstH, &c.unsafeGet(x, 0, z));


			//add trees
			{

				if (getVegetationNoiseVal(x, z) > 0.5)
				{
					if (x % 3 == 0 && z % 3 == 0)
					{
						//generate tree
						StructureToGenerate str;
						str.type = Structure_Tree;
						str.pos = {x + xPadd, firstH, z + zPadd};

						generateStructures.push_back(str);
					}
				}
			}


		}
			

	
	FastNoiseSIMD::FreeNoiseSet(continentalness);
	FastNoiseSIMD::FreeNoiseSet(peaksAndValies);
	FastNoiseSIMD::FreeNoiseSet(oceansAndTerases);
	FastNoiseSIMD::FreeNoiseSet(densityNoise);



}

