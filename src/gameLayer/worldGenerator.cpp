#include <platformTools.h>
#include "worldGenerator.h"
#include "FastNoiseSIMD.h"
#include "FastNoise/FastNoise.h"
#include <cmath>
#include <math.h>
#include <algorithm>
#include <iostream>

const int waterLevel = 65;

void calculateBlockPass1(int height, Block *startPos, Biome &biome, bool road, float roadValue,
	float randomNumber, bool sandShore, bool stonePatch)
{


	BlockType surfaceBlock = biome.surfaceBlock;
	BlockType secondBlock = biome.secondaryBlock;

	if (sandShore)
	{
		surfaceBlock = BlockTypes::sand;
		secondBlock = BlockTypes::sand;
	}

	if (stonePatch)
	{
		surfaceBlock = BlockTypes::stone;
		secondBlock = BlockTypes::stone;

		if (sandShore)
		{
			surfaceBlock = BlockTypes::cobblestone;
		}
	}


#pragma region road
	if(road)
	{
		BlockVariation roadShape;
		roadShape.block.push_back(BlockTypes::grassBlock);
		roadShape.block.push_back(BlockTypes::gravel);
		roadShape.block.push_back(BlockTypes::coarseDirt);

		BlockType roadCenter = BlockTypes::coarseDirt;
		
		if (roadValue < 0.25)
		{
			surfaceBlock = roadCenter;
		}
		else
		{
			surfaceBlock = roadShape.getRandomBLock(randomNumber);
		}

	}
#pragma endregion


	int y = height;

	//find grass
	for (; y >= waterLevel; y--)
	{
		//if (startPos[y].getType() != BlockTypes::air)
		if (startPos[y].getType() == BlockTypes::stone)
		{
			startPos[y].setType(surfaceBlock);

			for (y--; y > height - 4; y--)
			{
				if (startPos[y].getType() != BlockTypes::air)
				{
					startPos[y].setType(secondBlock);
				}
			}

			break;
		}
	}

	for (y = waterLevel; y >= 20; y--)
	{
		if (startPos[y].getType() == BlockTypes::air)
		{
			if(road && y == waterLevel)
			{
				startPos[y].setType(BlockTypes::wooden_plank);
			}
			else
			{
				startPos[y].setType(biome.waterType);
			}
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


int fromFloatNoiseValToIntegers(float noise, int maxExclusive)
{
	if (noise >= 1) { noise = 0.999; }
	return noise * maxExclusive;
}

void generateChunk(ChunkData& c, WorldGenerator &wg, StructuresManager &structuresManager, BiomesManager &biomesManager,
	std::vector<StructureToGenerate> &generateStructures)
{


	//super flat
	//for (int x = 0; x < CHUNK_SIZE; x++)
	//	for (int z = 0; z < CHUNK_SIZE; z++)
	//	{
	//		c.cachedBiomes[x][z] = 0;
	//
	//		for (int y = 0; y < 256; y++)
	//		{
	//			if (y <= 4)
	//			{
	//				c.unsafeGet(x, y, z).setType(BlockTypes::stone);
	//			}
	//			else if(y <= 7)
	//			{
	//				c.unsafeGet(x, y, z).setType(BlockTypes::dirt);
	//			}
	//			else if (y == 8)
	//			{
	//				c.unsafeGet(x, y, z).setType(BlockTypes::grassBlock);
	//			}
	//			else
	//			{
	//				c.unsafeGet(x, y, z).setType(BlockTypes::air);
	//			}
	//
	//		}
	//	}
	//return;



	float interpolateValues[16 * 16] = {};
	float borderingFactor[16 * 16] = {};
	float vegetationMaster = 0;
	int currentBiomeHeight = wg.getRegionHeightAndBlendingsForChunk(c.x, c.z, interpolateValues, borderingFactor, vegetationMaster);

	float vegetationPower = linearRemap(vegetationMaster, 0, 1, 0.9, 1.8);

	auto interpolator = [&](int *ptr, float value)
	{
		if (value >= 5.f)
		{
			return (float)ptr[5];
		}

		float rez = ptr[int(value)];
		float rez2 = ptr[int(value)+1];

		float interp = value - int(value);

		//return rez;
		return glm::mix(rez, rez2, interp);
	};
	                   //water    plains   hills
	int startValues[] = {22, 45,  66,      72,     80, 140};
	int maxlevels[] =   {40, 64,  71,      120,     170, 250};
	int biomes[] = {BiomesManager::plains, BiomesManager::plains, 
		BiomesManager::plains, BiomesManager::forest,
		BiomesManager::snow, BiomesManager::snow};

	int valuesToAddToStart[] = {5, 5, 10,  20,  20,  20};
	int valuesToAddToMax[] = {5, 5, 5,  20,  10,  0};
	float peaksPower[] = {1,1, 0.5, 1, 1, 1};

	c.clear();
	
	int xPadd = c.x * 16;
	int zPadd = c.z * 16;

	float* continentalness
		= wg.continentalnessNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *peaksAndValies
		= wg.peaksValiesNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	//todo fix this bug in an older commit
	float *wierdness
		= wg.wierdnessNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *densityNoise
		= wg.stone3Dnoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	float *randomSand
		= wg.randomStonesNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	float *randomGravel
		= wg.randomStonesNoise->GetNoiseSet(xPadd, 300, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	float *randomClay
		= wg.randomStonesNoise->GetNoiseSet(xPadd, 600, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	float *spagettiNoise
		= wg.spagettiNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	const float SHIFT = 16;
	float *spagettiNoise2
		= wg.spagettiNoise->GetNoiseSet(xPadd + SHIFT + 6, 0 + SHIFT + 3, zPadd + SHIFT, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	float *randomStones = wg.randomStonesNoise->GetNoiseSet(xPadd, 0, zPadd, 1, 1, 1);
	*randomStones = std::pow(((*randomStones + 1.f) / 2.f)*0.5, 3.0);


	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		spagettiNoise[i] += 1;
		spagettiNoise[i] /= 2;
		spagettiNoise[i] = powf(spagettiNoise[i], wg.spagettiNoisePower);
		spagettiNoise[i] = wg.spagettiNoiseSplines.applySpline(spagettiNoise[i]);

		spagettiNoise2[i] += 1;
		spagettiNoise2[i] /= 2;
		spagettiNoise2[i] = powf(spagettiNoise2[i], wg.spagettiNoisePower);
		spagettiNoise2[i] = wg.spagettiNoiseSplines.applySpline(spagettiNoise2[i]);
	}
	
	float *vegetationNoise
		= wg.vegetationNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *vegetationNoise2
		= wg.vegetationNoise->GetNoiseSet(xPadd, 100, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *vegetationNoise3
		= wg.vegetationNoise->GetNoiseSet(xPadd, 200, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *vegetationNoise4
		= wg.vegetationNoise->GetNoiseSet(xPadd, 300, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *whiteNoise
		= wg.whiteNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1);

	float *whiteNoise2
		= wg.whiteNoise2->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1);

	float *whiteNoise3
		= wg.whiteNoise2->GetNoiseSet(xPadd, 100, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1);

	float *stonePatches =
		wg.stonePatchesNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, 1, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		stonePatches[i] += 1;
		stonePatches[i] /= 2;
		stonePatches[i] = powf(stonePatches[i], wg.stonePatchesPower);
		stonePatches[i] = wg.stonePatchesSpline.applySpline(stonePatches[i]);
	}

	//float *temperatureNoise
	//	= wg.temperatureNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	//for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	//{
	//	temperatureNoise[i] += 1;
	//	temperatureNoise[i] /= 2;
	//	temperatureNoise[i] = powf(temperatureNoise[i], wg.temperaturePower);
	//	temperatureNoise[i] = wg.temperatureSplines.applySpline(temperatureNoise[i]);
	//}

	float *riversNoise
		= wg.riversNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *hillsDropDownsNoise
		= wg.hillsDropsNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *roadNoise =
		wg.roadNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		riversNoise[i] += 1;
		riversNoise[i] /= 2;
		riversNoise[i] = powf(riversNoise[i], wg.riversPower);
		riversNoise[i] = wg.riversSplines.applySpline(riversNoise[i]);

		roadNoise[i] += 1;
		roadNoise[i] /= 2;
		roadNoise[i] = powf(roadNoise[i], wg.roadPower);
		roadNoise[i] = wg.roadSplines.applySpline(roadNoise[i]);

		hillsDropDownsNoise[i] += 1;
		hillsDropDownsNoise[i] /= 2;
		hillsDropDownsNoise[i] = powf(hillsDropDownsNoise[i], wg.hillsDropsPower);
		hillsDropDownsNoise[i] = wg.hillsDropsSpline.applySpline(hillsDropDownsNoise[i]);
	}


	for (int i = 0; i < (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1); i++)
	{
		whiteNoise[i] += 1;
		whiteNoise[i] /= 2;
	}

	for (int i = 0; i < (CHUNK_SIZE+1) * (CHUNK_SIZE+1); i++)
	{
		whiteNoise2[i] += 1;
		whiteNoise2[i] /= 2;
	}

	for (int i = 0; i < (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1); i++)
	{
		whiteNoise3[i] += 1;
		whiteNoise3[i] /= 2;
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		vegetationNoise[i] += 1;
		vegetationNoise[i] /= 2;
		vegetationNoise[i] = powf(vegetationNoise[i], wg.vegetationPower);
		vegetationNoise[i] = wg.vegetationSplines.applySpline(vegetationNoise[i]);
		vegetationNoise[i] = powf(vegetationNoise[i], vegetationPower);


		vegetationNoise2[i] += 1;
		vegetationNoise2[i] /= 2;
		vegetationNoise2[i] = powf(vegetationNoise2[i], wg.vegetationPower);
		vegetationNoise2[i] = wg.vegetationSplines.applySpline(vegetationNoise2[i]);
		vegetationNoise2[i] = powf(vegetationNoise[i], vegetationPower);

		vegetationNoise3[i] += 1;
		vegetationNoise3[i] /= 2;
		vegetationNoise3[i] = powf(vegetationNoise3[i], wg.vegetationPower);
		vegetationNoise3[i] = wg.vegetationSplines.applySpline(vegetationNoise3[i]);
		vegetationNoise3[i] = powf(vegetationNoise[i], vegetationPower);

		vegetationNoise4[i] += 1;
		vegetationNoise4[i] /= 2;
		vegetationNoise4[i] = powf(vegetationNoise4[i], wg.vegetationPower);
		vegetationNoise4[i] = wg.vegetationSplines.applySpline(vegetationNoise4[i]);
		vegetationNoise4[i] = powf(vegetationNoise[i], vegetationPower);

	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		continentalness[i] += 1;
		continentalness[i] /= 2;
		continentalness[i] = powf(continentalness[i], wg.continentalPower);
		continentalness[i] = wg.continentalSplines.applySpline(continentalness[i]);
	}

	

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		peaksAndValies[i] += 1;
		peaksAndValies[i] /= 2;
		peaksAndValies[i] = powf(peaksAndValies[i], wg.peaksValiesPower);

		float val = peaksAndValies[i];

		peaksAndValies[i] = wg.peaksValiesSplines.applySpline(peaksAndValies[i]);

		//continentalness[i] = lerp(continentalness[i], peaksAndValies[i], wg.peaksValiesContributionSplines.applySpline(val));

		//if (currentBiomeHeight % 2)
		//{
		//	continentalness[i] = 1.f - continentalness[i];
		//}
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		wierdness[i] += 1;
		wierdness[i] /= 2;
		wierdness[i] = powf(wierdness[i], wg.wierdnessPower);

		float val = wierdness[i];

	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		densityNoise[i] += 1;
		densityNoise[i] /= 2;
		densityNoise[i] = powf(densityNoise[i], wg.stone3Dpower);
		densityNoise[i] = wg.stone3DnoiseSplines.applySpline(densityNoise[i]);
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		randomSand[i] += 1;
		randomSand[i] /= 2;
		randomSand[i] = powf(randomSand[i], wg.randomSandPower);
		randomSand[i] = wg.randomSandSplines.applySpline(randomSand[i]);

		randomGravel[i] += 1;
		randomGravel[i] /= 2;
		randomGravel[i] = powf(randomGravel[i], wg.randomSandPower + 0.1);
		randomGravel[i] = wg.randomSandSplines.applySpline(randomGravel[i]);

		randomClay[i] += 1;
		randomClay[i] /= 2;
		randomClay[i] = powf(randomClay[i], wg.randomSandPower + 0.5);
		randomClay[i] = wg.randomSandSplines.applySpline(randomClay[i]);
	}

#pragma region gets

	auto getNoiseVal = [continentalness](int x, int y, int z)
	{
		return continentalness[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
	};

	//auto getTemperature = [temperatureNoise](int x, int z)
	//{
	//	return temperatureNoise[x * CHUNK_SIZE + z];
	//};

	auto getRivers = [riversNoise](int x, int z)
	{
		return riversNoise[x * CHUNK_SIZE + z];
	};

	auto getHillsDropDowns = [hillsDropDownsNoise](int x, int z)
	{
		return hillsDropDownsNoise[x * CHUNK_SIZE + z];
	};

	auto getRoads = [roadNoise](int x, int z)
	{
		return roadNoise[x * CHUNK_SIZE + z];
	};

	auto getPeaksAndValies = [peaksAndValies](int x, int z)
	{
		return peaksAndValies[x * CHUNK_SIZE + z];
	};
	
	auto getWhiteNoiseVal = [whiteNoise](int x, int z)
	{
		return whiteNoise[x * (CHUNK_SIZE + 1) + z];
	};

	auto getWhiteNoise2Val = [whiteNoise2](int x, int z)
	{
		return whiteNoise2[x * (CHUNK_SIZE + 1) + z];
	};

	auto getWhiteNoise3Val = [whiteNoise3](int x, int z)
	{
		return whiteNoise3[x * (CHUNK_SIZE + 1) + z];
	};

	auto getStonePatches = [stonePatches](int x, int z)
	{
		return stonePatches[x * CHUNK_SIZE + z];
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

	auto getVegetation2NoiseVal = [vegetationNoise2](int x, int z)
	{
		return vegetationNoise2[x * CHUNK_SIZE + z];
	};

	auto getVegetation3NoiseVal = [vegetationNoise3](int x, int z)
	{
		return vegetationNoise3[x * CHUNK_SIZE + z];
	};

	auto getVegetation4NoiseVal = [vegetationNoise4](int x, int z)
	{
		return vegetationNoise4[x * CHUNK_SIZE + z];
	};

	auto getDensityNoiseVal = [densityNoise](int x, int y, int z) //todo more cache friendly operation here please
	{
		return densityNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT) + y * CHUNK_SIZE + z];
	};

	auto getRandomSandVal = [randomSand](int x, int y, int z)
	{
		return randomSand[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getRandomGravelVal = [randomGravel](int x, int y, int z)
	{
		return randomGravel[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getRandomClayVal = [randomClay](int x, int y, int z)
	{
		return randomClay[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getSpagettiNoiseVal = [spagettiNoise](int x, int y, int z) //todo more cache friendly operation here please
	{
		return spagettiNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getSpagettiNoiseVal2 = [spagettiNoise2](int x, int y, int z) //todo more cache friendly operation here please
	{
		return spagettiNoise2[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	//auto getSpagettiNoiseVal3 = [spagettiNoise3](int x, int y, int z) //todo more cache friendly operation here please
	//{
	//	return spagettiNoise3[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	//};

	auto getWierdness = [&](int x, int z)
	{
		return wierdness[x * CHUNK_SIZE + z];
	};

#pragma endregion


	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{

			//this will be some random places where there is stone instead of grass or sand or whatever
			float stonePatchesVal = getStonePatches(x, z);

			float wierdnessTresshold = 0.4;
			float localWierdness = getWierdness(x, z);
			float peaks = getPeaksAndValies(x, z);
			float continentalness = getNoiseVal(x, 0, z);

			float currentInterpolatedValue = interpolateValues[z + x * CHUNK_SIZE];
			int startLevel = interpolator(startValues, currentInterpolatedValue);
			int maxMountainLevel = interpolator(maxlevels, currentInterpolatedValue);
			

		#pragma region roads
			bool placeRoad = 0;
			float roadValue = 0;

			//plains roads
			if (currentBiomeHeight == 2)
			{
				roadValue = getRoads(x, z);

				if (roadValue < 0.6)
				{
					peaks = glm::mix(0.5f, peaks, roadValue);

					if (roadValue < 0.5)
					{
						placeRoad = true;
					}
				};


				//maxMountainLevel = glm::mix(maxMountainLevel-1, maxMountainLevel, roadValue);


				//localWierdness = glm::mix(1.f, localWierdness, roadValue);
				//todo investigate wierdness
				//wierdnessTresshold = glm::mix(0.f, wierdnessTresshold, roadValue);
				//wierdnessTresshold = 0;

				//peaks = glm::mix(0.5f, peaks, roadValue);
			}
			else if (currentBiomeHeight == 3 && localWierdness < 0.5f && continentalness < 0.4f)
			{
				roadValue = getRoads(x, z);

				if (roadValue < 0.6)
				{
					peaks = glm::mix(0.5f, peaks, roadValue);

					if (roadValue < 0.5)
					{
						placeRoad = true;
					}
				};

				//wierdnessTresshold = glm::mix(0.f, wierdnessTresshold, roadValue);
				//wierdnessTresshold = 0;

				//maxMountainLevel = glm::mix(maxMountainLevel - 1, maxMountainLevel, roadValue);
				//startLevel = glm::mix(startLevel - 1, startLevel, roadValue);

				//localWierdness = glm::mix(1.f, localWierdness, roadValue);
				//todo investigate wierdness

				//peaks = glm::mix(0.5f, peaks, roadValue);

			}
		#pragma endregion

		#pragma region borderings

			bool sandShore = 0;
			if (
				//currentBiomeHeight == 2 
				currentInterpolatedValue <= 1.9f && currentInterpolatedValue > 1.1f
				&& borderingFactor[z + x * CHUNK_SIZE] > 0.1)
			{
				sandShore = 1;
			}

		#pragma endregion




			auto screenBlend = [](float a, float b)
			{
				return 1.f - (1.f - a) * (1.f - b);
			};

			//plains rivers
			if (currentBiomeHeight == 2)
			{
				float rivers = getRivers(x, z);

				startLevel = glm::mix(waterLevel - 5, startLevel, rivers);
				maxMountainLevel = glm::mix(waterLevel - 2, maxMountainLevel, rivers);
			}


			//hills drop downs
			if (currentBiomeHeight == 3)
			{
				float dropDown = getHillsDropDowns(x, z);

				//make sure we don't do this near edges...
				if (interpolateValues[z + x * CHUNK_SIZE] > 3.f)
				{
					float interpolator = interpolateValues[z + x * CHUNK_SIZE] - int(interpolateValues[z + x * CHUNK_SIZE]);
					dropDown = screenBlend(dropDown, interpolator);
				}

				if (dropDown < 0.9)
				{
					startLevel = glm::mix(66, startLevel, dropDown);
					maxMountainLevel = glm::mix(71, maxMountainLevel, dropDown);
				}
			}




			int valueToAddToStart = valuesToAddToStart[currentBiomeHeight];
			int valueToAddToEnd = valuesToAddToMax[currentBiomeHeight];
				
			peaks = std::powf(peaks, peaksPower[currentBiomeHeight]);
			startLevel += (peaks * valueToAddToStart) - 3;
			maxMountainLevel += (peaks * valueToAddToEnd) - 3;

			if (startLevel >= maxMountainLevel)
			{
				startLevel = maxMountainLevel;
				startLevel--;
				maxMountainLevel++;
			}


			int heightDiff = maxMountainLevel - startLevel;

			//float squishFactor = wg.densitySquishFactor + getWierdness(x, z) * 30 - 15;
			//squishFactor = std::max(squishFactor, 0.f);

			int biomeIndex = biomes[currentBiomeHeight];
			auto biome = biomesManager.biomes[biomeIndex];

			c.unsafeGetCachedBiome(x, z) = biomeIndex;

			constexpr int stoneNoiseStartLevel = 1;

			int height = int(startLevel + continentalness * heightDiff);

			float firstH = 1;
			for (int y = 0; y < 256; y++)
			{


				float density = 1;
				{
					//int heightOffset = height + wg.densityHeightoffset;
					//int difference = y - heightOffset;
					//float differenceMultiplier =
					//	glm::clamp(powf(std::abs(difference) / squishFactor, wg.densitySquishPower),
					//	1.f, 10.f);
					//density = getDensityNoiseVal(x, y, z);
					//if (difference > 0)
					//{
					//	density = powf(density, differenceMultiplier);
					//}
					//else if (difference < 0)
					//{
					//	density = powf(density, 1.f / (differenceMultiplier));
					//}

					density = getDensityNoiseVal(x, y, z);

					float heightNormalized = (y - startLevel) / (float)heightDiff;
					heightNormalized = glm::clamp(heightNormalized, 0.f, 1.f);
					float heightNormalizedRemapped = linearRemap(heightNormalized, 0, 1, 0.2, 7);

					heightNormalized = glm::mix(0.1f, heightNormalizedRemapped, localWierdness);
					//heightNormalized = 0.1f;


					density = std::powf(density, heightNormalized);
					density = glm::clamp(density, 0.f, 1.f);
						
					//density = linearRemap(density, 0, 1)
				}
				
			#pragma region other block patches
				BlockType block = BlockTypes::stone;

				float sandVal = getRandomSandVal(x, y, z);
				float gravelVal = getRandomGravelVal(x, y, z);
				float clayVal = getRandomClayVal(x, y, z);

				if (sandVal > 0.5 || gravelVal > 0.5 || clayVal > 0.5)
				{
					if (sandVal > gravelVal && sandVal > clayVal)
					{
						block = BlockTypes::sand;
					}
					else if (gravelVal > clayVal)
					{
						block = BlockTypes::gravel;
					}
					else
					{

						if (biome.isICy)
						{
							block = BlockTypes::ice;
						}
						else
						{
							block = BlockTypes::clay;
						}

					}

				}
			#pragma endregion


				if (y < stoneNoiseStartLevel)
				{
					c.unsafeGet(x, y, z).setType(block);
				}
				else
				{
					if (y < height)
					if (density > wierdnessTresshold)
					{
						firstH = y;
						c.unsafeGet(x, y, z).setType(block);
					}
					//else cave
				}

			}

			calculateBlockPass1(firstH, &c.unsafeGet(x, 0, z), biome, placeRoad, roadValue, 
				getWhiteNoiseVal(x,z), sandShore, stonePatchesVal > 0.5);



			for (int y = 1; y < firstH; y++)
			{
				auto density = getSpagettiNoiseVal(x, y, z);
				float density2 = getSpagettiNoiseVal2(x, y, z);
				//float density3 = getSpagettiNoiseVal3(x, y, z);
				density = screenBlend(density, density2);
				//density = screenBlend(density, density3);



				float bias = (y - 1) / (256 - 1.f - 1);
				bias = glm::clamp(bias, 0.f, 1.f);
				bias = 1.f - bias;
			
				bias = powf(bias, wg.spagettiNoiseBiasPower);
			
				if (density > wg.spagettiNoiseBias * bias)
				{
					//stone
				}
				else
				{
					if (c.unsafeGet(x, y, z).getType() != BlockTypes::water)
					{
						c.unsafeGet(x, y, z).setType(BlockTypes::air);
					}
				}
			
			}

			auto generateTreeFunction = [&](unsigned char treeType)
			{
				//generate tree
				StructureToGenerate str;
				if (treeType == Biome::treeNormal)
				{
					str.type = Structure_Tree;
				}else if (treeType == Biome::treeNormalTall)
				{
					str.type = Structure_Tree;
					str.addRandomTreeHeight = true;
					str.replaceLogWith = BlockTypes::woodLog;
				}
				else if (treeType == Biome::treeSpruceTallOakCenter)
				{
					str.type = Structure_SpruceSlim;
					str.addRandomTreeHeight = true;
					str.replaceLogWith = BlockTypes::woodLog;
					str.replaceLeavesWith = BlockTypes::leaves;
				}
				else if (treeType == Biome::treeSpruceTallOakCenterRed)
				{
					str.type = Structure_SpruceSlim;
					str.addRandomTreeHeight = true;
					str.replaceLogWith = BlockTypes::woodLog;
					str.replaceLeavesWith = BlockTypes::spruce_leaves_red;
				}
				else if (treeType == Biome::treeSpruceTallOakCenterYellow)
				{
					str.type = Structure_SpruceSlim;
					str.addRandomTreeHeight = true;
					str.replaceLogWith = BlockTypes::woodLog;
					str.replaceLeavesWith = BlockTypes::birch_leaves;
				}
				else if (treeType == Biome::treeJungle)
				{
					str.type = Structure_JungleTree;
				}
				else if (treeType == Biome::treePalm)
				{
					str.type = Structure_PalmTree;
				}
				else if (treeType == Biome::treeBirch)
				{
					str.type = Structure_BirchTree;
				}
				else if (treeType == Biome::treeSpruce)
				{
					str.type = Structure_Spruce;
				}
				else if (treeType == Biome::treeTallOak)
				{
					str.type = Structure_TallSlimTree;
					str.addRandomTreeHeight = true;
					str.replaceLogWith = BlockTypes::woodLog;
					str.replaceLeavesWith = BlockTypes::leaves;
				}
				else
				{
					assert(0);
				}

				str.pos = {x + xPadd, firstH, z + zPadd};
				str.replaceBlocks = false;
				str.randomNumber1 = getWhiteNoise3Val(x, z);
				str.randomNumber2 = getWhiteNoise3Val(x + 1, z);
				str.randomNumber3 = getWhiteNoise3Val(x + 1, z + 1);
				str.randomNumber4 = getWhiteNoise3Val(x, z + 1);

				generateStructures.push_back(str);
			};

			if (firstH < CHUNK_HEIGHT - 1)
			{
				auto b = c.unsafeGet(x, firstH, z).getType();

				bool generatedSomethingElse = 0;

				//random stones
				{

					float stonesChance = glm::mix(0.008, 0.04, stonePatchesVal);

					stonesChance *= randomStones[0];

					if (getWhiteNoiseChance(x, z, stonesChance))
					{
						generatedSomethingElse = true;

						StructureToGenerate str;
						str.type = Structure_SmallStone;

						str.pos = {x + xPadd, firstH + 1, z + zPadd};
						str.randomNumber1 = getWhiteNoise3Val(x, z);
						str.randomNumber2 = getWhiteNoise3Val(x + 1, z);
						str.randomNumber3 = getWhiteNoise3Val(x + 1, z + 1);
						str.randomNumber4 = getWhiteNoise3Val(x, z + 1);

						generateStructures.push_back(str);
					}
				}

				if(!generatedSomethingElse)
				for (int noiseIndex = 0; noiseIndex < biome.vegetationNoises.size(); noiseIndex++)
				{
					auto &noiseSettings = biome.vegetationNoises[noiseIndex];

					float noiseVal = 0;
					float noiseVal1 = 0;
					float noiseVal2 = 0;
					float noiseVal3 = 0;

					if (noiseIndex == 0)
					{
						noiseVal = getVegetationNoiseVal(x, z);
						noiseVal1 = getVegetationNoiseVal(x+1, z);
						noiseVal2 = getVegetationNoiseVal(x, z+1);
						noiseVal3 = getVegetationNoiseVal(x+1, z+1);
					}
					else if (noiseIndex == 1)
					{
						noiseVal = getVegetation2NoiseVal(x, z);
						noiseVal1 = getVegetation2NoiseVal(x+1, z);
						noiseVal2 = getVegetation2NoiseVal(x, z+1);
						noiseVal3 = getVegetation2NoiseVal(x+1, z+1);
					}
					else if(noiseIndex == 2)
					{
						noiseVal = getVegetation3NoiseVal(x, z);
						noiseVal1 = getVegetation3NoiseVal(x + 1, z);
						noiseVal2 = getVegetation3NoiseVal(x, z + 1);
						noiseVal3 = getVegetation3NoiseVal(x + 1, z + 1);
					}
					else if (noiseIndex == 3)
					{
						noiseVal = getVegetation4NoiseVal(x, z);
						noiseVal1 = getVegetation4NoiseVal(x + 1, z);
						noiseVal2 = getVegetation4NoiseVal(x, z + 1);
						noiseVal3 = getVegetation4NoiseVal(x + 1, z + 1);
					}

					bool generated = 0;

					//one distribution element, can be multiple things there tho
					for (auto &entry : noiseSettings.entry)
					{

						if (noiseVal >= entry.minTresshold && entry.maxTresshold >= noiseVal)
						{

							float chanceRemap = linearRemap(noiseVal, entry.minTresshold, entry.maxTresshold,
								entry.chanceRemap.x, entry.chanceRemap.y);
							//chanceRemap = noiseVal;

							if (getWhiteNoiseChance(x, z, chanceRemap))
							{

								//pick the block to place
								auto &growThing = entry.growThing;

								bool canGrow = 0;

								for (auto &growOn : growThing.growOn)
								{
									if (b == growOn)
									{
										canGrow = true;
									}
								}

								if (canGrow)
								{
									int count = growThing.elements.size();
									float noiseVal = getWhiteNoise2Val(x, z);

									int index = fromFloatNoiseValToIntegers(noiseVal, count);
									auto &growElement = growThing.elements[index];

									assert(growElement.block || growElement.treeType);
									assert(!(growElement.block != 0 && growElement.treeType != 0));

									if (growElement.block)
									{
										c.unsafeGet(x, firstH + 1, z).setType(growElement.block);
										generated = true;
									}
									else if (growElement.treeType)
									{

										//don't put trees too together...
										float noiseVal1 = getWhiteNoise2Val(x + 1, z);
										float noiseVal2 = getWhiteNoise2Val(x, z + 1);
										float noiseVal3 = getWhiteNoise2Val(x + 1, z + 1);

										if ( 0 &&
											noiseVal1 >= entry.minTresshold && entry.maxTresshold >= noiseVal1 ||
											noiseVal2 >= entry.minTresshold && entry.maxTresshold >= noiseVal2 ||
											noiseVal3 >= entry.minTresshold && entry.maxTresshold >= noiseVal3
											)
										{

											float chanceRemap1 = linearRemap(noiseVal1, entry.minTresshold, entry.maxTresshold,
												entry.chanceRemap.x, entry.chanceRemap.y);
											float chanceRemap2 = linearRemap(noiseVal2, entry.minTresshold, entry.maxTresshold,
												entry.chanceRemap.x, entry.chanceRemap.y);
											float chanceRemap3 = linearRemap(noiseVal3, entry.minTresshold, entry.maxTresshold,
												entry.chanceRemap.x, entry.chanceRemap.y);

											if (
												getWhiteNoiseChance(x + 1, z, chanceRemap1) ||
												getWhiteNoiseChance(x, z + 1, chanceRemap2) ||
												getWhiteNoiseChance(x + 1, z + 1, chanceRemap3)
												)
											{
												int index1 = fromFloatNoiseValToIntegers(noiseVal, count);
												int index2 = fromFloatNoiseValToIntegers(noiseVal, count);
												int index3 = fromFloatNoiseValToIntegers(noiseVal, count);

												if (index1 == index || index2 == index || index3 == index)
												{
													continue;
												}
											}
										}

										generateTreeFunction(growElement.treeType);
										generated = true;
										break;
									}
								}

							}

						}

						if (generated) { break; }
					}
					if (generated) { break; }


				}

			}



		}

	
	FastNoiseSIMD::FreeNoiseSet(continentalness);
	FastNoiseSIMD::FreeNoiseSet(peaksAndValies);
	FastNoiseSIMD::FreeNoiseSet(wierdness);
	FastNoiseSIMD::FreeNoiseSet(densityNoise);
	FastNoiseSIMD::FreeNoiseSet(randomSand);
	FastNoiseSIMD::FreeNoiseSet(randomGravel);
	FastNoiseSIMD::FreeNoiseSet(randomClay);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise2);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise3);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise4);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise2);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise3);
	FastNoiseSIMD::FreeNoiseSet(riversNoise);
	FastNoiseSIMD::FreeNoiseSet(hillsDropDownsNoise);
	FastNoiseSIMD::FreeNoiseSet(roadNoise);
	//FastNoiseSIMD::FreeNoiseSet(temperatureNoise);
	FastNoiseSIMD::FreeNoiseSet(spagettiNoise);
	FastNoiseSIMD::FreeNoiseSet(spagettiNoise2);
	FastNoiseSIMD::FreeNoiseSet(randomStones);
	FastNoiseSIMD::FreeNoiseSet(stonePatches);
	

}

