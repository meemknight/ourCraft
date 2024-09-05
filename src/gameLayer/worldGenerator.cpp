#include <platformTools.h>
#include "worldGenerator.h"
#include "FastNoiseSIMD.h"
#include "FastNoise/FastNoise.h"
#include <cmath>
#include <math.h>
#include <algorithm>

int waterLevel = 65;

void calculateBlockPass1(int height, Block *startPos, Biome &biome)
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
			startPos[y].setType(biome.waterType);
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

	int startValues[] = {22, 45, 66, 80, 90, 150};
	int maxlevels[] = {40, 64, 70, 90, 150, 250};
	int biomes[] = {BiomesManager::plains, BiomesManager::plains, 
		BiomesManager::plains, BiomesManager::forest, BiomesManager::snow, BiomesManager::snow};



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

	//float *densityNoise
	//	= wg.stone3Dnoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	float *spagettiNoise
		= wg.spagettiNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		spagettiNoise[i] += 1;
		spagettiNoise[i] /= 2;
		spagettiNoise[i] = powf(spagettiNoise[i], wg.spagettiNoisePower);
		spagettiNoise[i] = wg.spagettiNoiseSplines.applySpline(spagettiNoise[i]);
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

	float *humidityNoise
		= wg.humidityNoise->GetNoiseSet(xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		humidityNoise[i] += 1;
		humidityNoise[i] /= 2;
		humidityNoise[i] = powf(humidityNoise[i], wg.humidityPower);
		humidityNoise[i] = wg.humiditySplines.applySpline(humidityNoise[i]);
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

		continentalness[i] = lerp(continentalness[i], peaksAndValies[i], wg.peaksValiesContributionSplines.applySpline(val));

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

	//for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	//{
	//	densityNoise[i] += 1;
	//	densityNoise[i] /= 2;
	//	densityNoise[i] = powf(densityNoise[i], wg.stone3Dpower);
	//	densityNoise[i] = wg.stone3DnoiseSplines.applySpline(densityNoise[i]);
	//}

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

	//auto getDensityNoiseVal = [densityNoise](int x, int y, int z) //todo more cache friendly operation here please
	//{
	//	return densityNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT) + y * CHUNK_SIZE + z];
	//};

	auto getSpagettiNoiseVal = [spagettiNoise](int x, int y, int z) //todo more cache friendly operation here please
	{
		return spagettiNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getWierdness = [&](int x, int z)
	{
		return wierdness[x * CHUNK_SIZE + z];
	};

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{

			int startLevel = interpolator(startValues, interpolateValues[z + x * CHUNK_SIZE]);
			int maxMountainLevel = interpolator(maxlevels, interpolateValues[z + x * CHUNK_SIZE]);
			int heightDiff = maxMountainLevel - startLevel;

			float squishFactor = wg.densitySquishFactor + getWierdness(x, z) * 30 - 15;
			squishFactor = std::max(squishFactor, 0.f);

			//auto biomeIndex = biomesManager.determineBiomeIndex(getTemperature(x, z), getHumidity(x, z));
			int biomeIndex = biomes[currentBiomeHeight];
			auto &biome = biomesManager.biomes[biomeIndex];

			c.unsafeGetCachedBiome(x, z) = biomeIndex;

			constexpr int stoneNoiseStartLevel = 1;

			float heightPercentage = getNoiseVal(x, 0, z);
			int height = int(startLevel + heightPercentage * heightDiff);

			float firstH = 1;
			for (int y = 0; y < 256; y++)
			{

				//auto density = getDensityNoiseVal(x, y, z);

				int heightOffset = height + wg.densityHeightoffset;
				int difference = y - heightOffset;
				float differenceMultiplier =
					glm::clamp(powf(std::abs(difference) / squishFactor, wg.densitySquishPower),
					1.f, 10.f);

				//if (difference > 0)
				//{
				//	density = powf(density, differenceMultiplier);
				//}
				//else if (difference < 0)
				//{
				//	density = powf(density, 1.f / (differenceMultiplier));
				//}

				if (y < stoneNoiseStartLevel)
				{
					c.unsafeGet(x, y, z).setType(BlockTypes::stone);
				}
				else
				{
					//if (density > 0.5)
					if (y < height)
					{
						firstH = y;
						c.unsafeGet(x, y, z).setType(BlockTypes::stone);
					}
					//else cave
				}

			}


			calculateBlockPass1(firstH, &c.unsafeGet(x, 0, z), biome);

			for (int y = 1; y < firstH; y++)
			{
				auto density = getSpagettiNoiseVal(x, y, z);
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
	//FastNoiseSIMD::FreeNoiseSet(densityNoise);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise);
	FastNoiseSIMD::FreeNoiseSet(vegetationNoise2);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise);
	FastNoiseSIMD::FreeNoiseSet(whiteNoise2);
	FastNoiseSIMD::FreeNoiseSet(humidityNoise);
	FastNoiseSIMD::FreeNoiseSet(temperatureNoise);
	FastNoiseSIMD::FreeNoiseSet(spagettiNoise);



}

