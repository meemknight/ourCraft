#include "worldGenerator.h"
#include "FastNoiseSIMD.h"
#include "FastNoise/FastNoise.h"
#include <cmath>

constexpr int startLevel = 45;
constexpr int waterLevel = 65;
constexpr int maxMountainLevel = 220;
constexpr int heightDiff = maxMountainLevel - startLevel;

void calculateBlockPass1(int height, Block *startPos, Biome &biome)
{

	int y = height;

	//find grass
	for (; y >= waterLevel; y--)
	{
		if (startPos[y].type != BlockTypes::air)
		{
			startPos[y].type = biome.surfaceBlock;

			for (y--; y > height - 4; y--)
			{
				if (startPos[y].type != BlockTypes::air)
				{
					startPos[y].type = biome.secondaryBlock;
				}
			}

			break;
		}
	}

	for (y = waterLevel; y >= 20; y--)
	{
		if (startPos[y].type == BlockTypes::air)
		{
			startPos[y].type = biome.waterType;
		}
		else
		{
			break;
		}
	}

}

void generateChunk(Chunk &c, WorldGenerator &wg, StructuresManager &structuresManager,
	BiomesManager &biomesManager, std::vector<StructureToGenerate> &generateStructures)
{
	generateChunk(c.data, wg, structuresManager, biomesManager, generateStructures);
}


void generateChunk(ChunkData& c, WorldGenerator &wg, StructuresManager &structuresManager, BiomesManager &biomesManager,
	std::vector<StructureToGenerate> &generateStructures)
{
	c.clear();
	
	int xPadd = c.x * 16;
	int zPadd = c.z * 16;

	float* continentalness
		= wg.continentalnessNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *peaksAndValies
		= wg.peaksValiesNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *oceansAndTerases
		= wg.peaksValiesNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *densityNoise
		= wg.stone3Dnoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	float *spagettiNoise
		= wg.spagettiNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		spagettiNoise[i] += 1;
		spagettiNoise[i] /= 2;
		spagettiNoise[i] = std::powf(spagettiNoise[i], wg.spagettiNoisePower);
		spagettiNoise[i] = wg.spagettiNoiseSplines.applySpline(spagettiNoise[i]);
	}
	
	float *vegetationNoise
		= wg.vegetationNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *whiteNoise
		= wg.whiteNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *whiteNoise2
		= wg.whiteNoise2->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1, 1);

	float *temperatureNoise
		= wg.temperatureNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		temperatureNoise[i] += 1;
		temperatureNoise[i] /= 2;
		temperatureNoise[i] = std::powf(temperatureNoise[i], wg.temperaturePower);
		temperatureNoise[i] = wg.temperatureSplines.applySpline(temperatureNoise[i]);
	}

	float *humidityNoise
		= wg.humidityNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		humidityNoise[i] += 1;
		humidityNoise[i] /= 2;
		humidityNoise[i] = std::powf(humidityNoise[i], wg.humidityPower);
		humidityNoise[i] = wg.humiditySplines.applySpline(humidityNoise[i]);
	}


	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		whiteNoise[i] += 1;
		whiteNoise[i] /= 2;
	}

	for (int i = 0; i < (CHUNK_SIZE+1) * (CHUNK_SIZE+1); i++)
	{
		whiteNoise2[i] += 1;
		whiteNoise2[i] /= 2;
	}

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

	auto getTemperature = [temperatureNoise](int x, int z)
	{
		return temperatureNoise[x * CHUNK_SIZE + z];
	};

	auto getHumidity = [humidityNoise](int x, int z)
	{
		return humidityNoise[x * CHUNK_SIZE + z];
	};
	
	auto getWhiteNoiseVal = [whiteNoise](int x, int z)
	{
		return whiteNoise[x * CHUNK_SIZE + z];
	};

	auto getWhiteNoise2Val = [whiteNoise2](int x, int z)
	{
		return whiteNoise2[x * (CHUNK_SIZE + 1) + z];
	};

	auto getWhiteNoiseChance = [getWhiteNoiseVal](int x, int z, float chance)
	{
		return getWhiteNoiseVal(x, z) < chance;
	};

	auto getWhiteNoise2Chance = [getWhiteNoise2Val](int x, int z, float chance)
	{
		return getWhiteNoise2Val(x, z) < chance;
	};

	auto getVegetationNoiseVal = [vegetationNoise](int x, int z)
	{
		return vegetationNoise[x * CHUNK_SIZE + z];
	};

	auto getDensityNoiseVal = [densityNoise](int x, int y, int z) //todo more cache friendly operation here please
	{
		return densityNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT) + y * CHUNK_SIZE + z];
	};

	auto getSpagettiNoiseVal = [spagettiNoise](int x, int y, int z) //todo more cache friendly operation here please
	{
		return spagettiNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};


	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{


			auto &biome = *biomesManager.determineBiome(getTemperature(x, z), getHumidity(x, z));


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

			
			calculateBlockPass1(firstH, &c.unsafeGet(x, 0, z), biome);

			for (int y = 1; y < firstH; y++)
			{
				auto density = getSpagettiNoiseVal(x, y, z);
				float bias = (y - 1) / (256 - 1.f - 1);
				bias = glm::clamp(bias, 0.f, 1.f);
				bias = 1.f - bias;

				bias = std::powf(bias, wg.spagettiNoiseBiasPower);

				if (density > wg.spagettiNoiseBias * bias)
				{
					//stone
				}
				else
				{
					if (c.unsafeGet(x, y, z).type != BlockTypes::water)
					{
						c.unsafeGet(x, y, z).type = BlockTypes::air;
					}
				}

			}


			//add trees and grass
			bool generateTree = biome.growTreesOn != BlockTypes::air && biome.treeType;

			if(generateTree || biome.growGrassOn != BlockTypes::air)
			if(firstH < CHUNK_HEIGHT-1)
			{
				auto b = c.unsafeGet(x, firstH, z).type;


				if (b == biome.growTreesOn || b == biome.growGrassOn)
				{
					float currentNoise = getVegetationNoiseVal(x, z);

					if (currentNoise > biome.forestTresshold)
					{
						float chance = linearRemap(currentNoise, biome.forestTresshold, 1, 
							biome.treeChanceRemap.x, biome.treeChanceRemap.y);

						float chance2 = linearRemap(currentNoise, biome.jusGrassTresshold, biome.forestTresshold,
							biome.grassChanceForestRemap.x, biome.grassChanceForestRemap.y);
						
						if (getWhiteNoiseChance(x, z, chance) && b == biome.growTreesOn && generateTree)
						{
							//generate tree
							StructureToGenerate str;
							if (biome.treeType == Biome::treeNormal)
							{
								str.type = Structure_Tree;
							}
							else if (biome.treeType == Biome::treeJungle)
							{
								str.type = Structure_JungleTree;
							}
							else if (biome.treeType == Biome::treePalm)
							{
								str.type = Structure_PalmTree;
							}
							else
							{
								assert(0);
							}

							str.pos = {x + xPadd, firstH, z + zPadd};
							str.randomNumber1 = getWhiteNoise2Val(x, z);
							str.randomNumber2 = getWhiteNoise2Val(x+1, z);
							str.randomNumber3 = getWhiteNoise2Val(x+1, z+1);
							str.randomNumber4 = getWhiteNoise2Val(x, z+1);

							generateStructures.push_back(str);
						}
						else if(b == biome.growGrassOn && biome.growGrassOn != BlockTypes::air)
						{
							if(getWhiteNoise2Chance(x, z, chance2))
							{
								c.unsafeGet(x, firstH+1, z).type = biome.grassType;
							}
						}
					}
					else if (currentNoise > biome.jusGrassTresshold)
					{
						float chance = linearRemap(currentNoise, biome.jusGrassTresshold, biome.forestTresshold,
							biome.justGrassChanceRemap.x, biome.justGrassChanceRemap.y);

						if (b == biome.growGrassOn && biome.growGrassOn != BlockTypes::air && getWhiteNoiseChance(x, z, chance))
						{
							c.unsafeGet(x, firstH+1, z).type = biome.grassType;;
						}
					}
				}

				
			}


		}
			

	
	FastNoiseSIMD::FreeNoiseSet(continentalness);
	FastNoiseSIMD::FreeNoiseSet(peaksAndValies);
	FastNoiseSIMD::FreeNoiseSet(oceansAndTerases);
	FastNoiseSIMD::FreeNoiseSet(densityNoise);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise2);
	FastNoiseSIMD::FreeNoiseSet(humidityNoise);
	FastNoiseSIMD::FreeNoiseSet(temperatureNoise);
	FastNoiseSIMD::FreeNoiseSet(spagettiNoise);



}

