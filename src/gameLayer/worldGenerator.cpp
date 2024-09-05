#include <platformTools.h>
#include "worldGenerator.h"
#include "FastNoiseSIMD.h"
#include "FastNoise/FastNoise.h"
#include <cmath>
#include <math.h>
#include <algorithm>

const int waterLevel = 65;

void calculateBlockPass1(int height, Block *startPos, Biome &biome, bool road)
{

	int y = height;

	//find grass
	for (; y >= waterLevel; y--)
	{
		if (startPos[y].getType() != BlockTypes::air)
		{
			startPos[y].setType(biome.surfaceBlock);

			for (y--; y > height - 4; y--)
			{
				if (startPos[y].getType() != BlockTypes::air)
				{
					startPos[y].setType(biome.secondaryBlock);
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


	float interpolateValues[16 * 16] = {};
	int currentBiomeHeight = wg.getRegionHeightAndBlendingsForChunk(c.x, c.z, interpolateValues);

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
		BiomesManager::plains, BiomesManager::forest, BiomesManager::snow, BiomesManager::snow};

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

	float *spagettiNoise
		= wg.spagettiNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	const float SHIFT = 16;
	float *spagettiNoise2
		= wg.spagettiNoise->GetNoiseSet(xPadd + SHIFT + 6, 0 + SHIFT + 3, zPadd + SHIFT, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	//float *spagettiNoise3
	//	= wg.spagettiNoise->GetNoiseSet(xPadd - SHIFT + 6, 0 - SHIFT + 6, zPadd - SHIFT, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);


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
		= wg.vegetationNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

	float *whiteNoise
		= wg.whiteNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1);

	float *whiteNoise2
		= wg.whiteNoise2->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1);

	float *temperatureNoise
		= wg.temperatureNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		temperatureNoise[i] += 1;
		temperatureNoise[i] /= 2;
		temperatureNoise[i] = powf(temperatureNoise[i], wg.temperaturePower);
		temperatureNoise[i] = wg.temperatureSplines.applySpline(temperatureNoise[i]);
	}

	float *riversNoise
		= wg.riversNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);

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

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		vegetationNoise[i] += 1;
		vegetationNoise[i] /= 2;
		vegetationNoise[i] = powf(vegetationNoise[i], wg.vegetationPower);
		vegetationNoise[i] = wg.vegetationSplines.applySpline(vegetationNoise[i]);

		vegetationNoise2[i] += 1;
		vegetationNoise2[i] /= 2;
		vegetationNoise2[i] = powf(vegetationNoise2[i], wg.vegetationPower);
		vegetationNoise2[i] = wg.vegetationSplines.applySpline(vegetationNoise2[i]);
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

#pragma region gets

	auto getNoiseVal = [continentalness](int x, int y, int z)
	{
		return continentalness[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
	};

	auto getTemperature = [temperatureNoise](int x, int z)
	{
		return temperatureNoise[x * CHUNK_SIZE + z];
	};

	auto getRivers = [riversNoise](int x, int z)
	{
		return riversNoise[x * CHUNK_SIZE + z];
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

	auto getDensityNoiseVal = [densityNoise](int x, int y, int z) //todo more cache friendly operation here please
	{
		return densityNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT) + y * CHUNK_SIZE + z];
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

			float localWierdness = getWierdness(x, z);
			float peaks = getPeaksAndValies(x, z);

			int startLevel = interpolator(startValues, interpolateValues[z + x * CHUNK_SIZE]);
			int maxMountainLevel = interpolator(maxlevels, interpolateValues[z + x * CHUNK_SIZE]);
			

			bool placeRoad = 0;
			//plains roads
			if (currentBiomeHeight == 2)
			{
				float road = getRoads(x, z);

				if (road < 0.5)
				{
					placeRoad = true;
				}

				maxMountainLevel = glm::mix(maxMountainLevel-2, maxMountainLevel, road);


				localWierdness = glm::mix(1.f, localWierdness, road);
				//todo investigate wierdness
				peaks = glm::mix(0.5f, peaks, road);
			}
			else if (currentBiomeHeight == 3)
			{
				float road = getRoads(x, z);

				if (road < 0.5)
				{
					placeRoad = true;
				}

				maxMountainLevel = glm::mix(maxMountainLevel - 3, maxMountainLevel, road);
				startLevel = glm::mix(startLevel - 1, startLevel, road);

				localWierdness = glm::mix(1.f, localWierdness, road);
				//todo investigate wierdness
				peaks = glm::mix(0.5f, peaks, road);

			}


			//plains rivers
			if (currentBiomeHeight == 2)
			{
				float rivers = getRivers(x, z);

				startLevel = glm::mix(waterLevel - 5, startLevel, rivers);
				maxMountainLevel = glm::mix(waterLevel - 2, maxMountainLevel, rivers);
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

			//todo change
			if (placeRoad)
			{
				biome.surfaceBlock = BlockTypes::coarseDirt;
			}

			c.unsafeGetCachedBiome(x, z) = biomeIndex;

			constexpr int stoneNoiseStartLevel = 1;

			float heightPercentage = getNoiseVal(x, 0, z);
			int height = int(startLevel + heightPercentage * heightDiff);

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

				if (y < stoneNoiseStartLevel)
				{
					c.unsafeGet(x, y, z).setType(BlockTypes::stone);
				}
				else
				{
					if (y < height)
					if (density > 0.4)
					{
						firstH = y;
						c.unsafeGet(x, y, z).setType(BlockTypes::stone);
					}
					//else cave
				}

			}

			auto screenBlend = [](float a, float b)
			{
				return 1.f - (1.f - a) * (1.f - b);
			};

			calculateBlockPass1(firstH, &c.unsafeGet(x, 0, z), biome, placeRoad);

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
				else
				{
					assert(0);
				}

				str.pos = {x + xPadd, firstH, z + zPadd};
				str.replaceBlocks = false;
				str.randomNumber1 = getWhiteNoise2Val(x, z);
				str.randomNumber2 = getWhiteNoise2Val(x + 1, z);
				str.randomNumber3 = getWhiteNoise2Val(x + 1, z + 1);
				str.randomNumber4 = getWhiteNoise2Val(x, z + 1);

				generateStructures.push_back(str);
			};

			if (firstH < CHUNK_HEIGHT - 1)
			{
				auto b = c.unsafeGet(x, firstH, z).getType();

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
					else
					{
						permaAssert(0);
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
					//if (generated) { break; }


				}

			}



		}
			

	
	FastNoiseSIMD::FreeNoiseSet(continentalness);
	FastNoiseSIMD::FreeNoiseSet(peaksAndValies);
	FastNoiseSIMD::FreeNoiseSet(wierdness);
	FastNoiseSIMD::FreeNoiseSet(densityNoise);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise2);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise2);
	FastNoiseSIMD::FreeNoiseSet(riversNoise);
	FastNoiseSIMD::FreeNoiseSet(roadNoise);
	FastNoiseSIMD::FreeNoiseSet(temperatureNoise);
	FastNoiseSIMD::FreeNoiseSet(spagettiNoise);
	FastNoiseSIMD::FreeNoiseSet(spagettiNoise2);
	//FastNoiseSIMD::FreeNoiseSet(spagettiNoise3);
	

}

