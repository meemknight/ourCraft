#include <platformTools.h>
#include "worldGenerator.h"
#include "FastNoiseSIMD.h"
#include "FastNoise/FastNoise.h"
#include <cmath>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <profilerLib/include/profilerLib.h>
#include <gl2d/gl2d.h>

const int waterLevel = 65;

alignas(32) static float iceNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
float getIceNoise(int x, int z)
{
	return iceNoise[x * CHUNK_SIZE + z];
};


//riverValue is from 0 to 1 where 1 means river
void calculateBlockPass1(int height, Block *startPos, Biome &biome, bool road, float roadValue,
	float randomNumber, bool sandShore, bool stonePatch, float riverValue,
	float currentInterpolatedValue, float randomBlockVariation, 
	float swampValue, int biomeHeight, float stoneSpikes, float canionValue, 
	int valueWithoutCanionDropDown, int x, int z, float lakeValue)
{

	
	BlockType surfaceBlock = biome.surfaceBlock;
	BlockType secondBlock = biome.secondaryBlock;
	float canionBridgeHeight = 89;

	bool placeCanionBridge = canionValue > 0.2 && roadValue < 0.3 && road
		;//&& valueWithoutCanionDropDown >= canionBridgeHeight-2;


	if (biome.blockVariations.size())
	{
		
		float f = randomBlockVariation;
		if (f >= 0.99) { f = 0.99; }
		auto &b = biome.blockVariations[int(f * biome.blockVariations.size())];
		
		surfaceBlock = b.surfaceBlock;
		secondBlock = b.secondaryBlock;

	}


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

	if (riverValue > 0.20)
	{
		BlockVariation roadShape;
		roadShape.block.push_back(BlockTypes::cobblestone);
		roadShape.block.push_back(BlockTypes::stone);
		roadShape.block.push_back(BlockTypes::gravel);
		roadShape.block.push_back(BlockTypes::coarseDirt);
		roadShape.block.push_back((BlockTypes)surfaceBlock);

		if (riverValue < 0.5)
		{
			roadShape.block.push_back((BlockTypes)surfaceBlock);
		}

		if (riverValue < 0.4)
		{
			roadShape.block.push_back((BlockTypes)surfaceBlock);
		}

		if (riverValue < 0.35)
		{
			roadShape.block.push_back((BlockTypes)surfaceBlock);
		}

		if (riverValue < 0.30)
		{
			roadShape.block.push_back((BlockTypes)surfaceBlock);
			roadShape.block.push_back((BlockTypes)surfaceBlock);
		}

		if (riverValue < 0.25)
		{
			roadShape.block.push_back((BlockTypes)surfaceBlock);
			roadShape.block.push_back((BlockTypes)surfaceBlock);
		}

		surfaceBlock = roadShape.getRandomBLock(randomNumber);
	}

#pragma region road
	if(road && !placeCanionBridge && canionValue < 0.2)
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

	canionBridgeHeight = glm::mix(canionBridgeHeight, canionBridgeHeight - 3.f, canionValue);
	if (placeCanionBridge && (startPos[(int)canionBridgeHeight].getType() == BlockTypes::air))
	{
		startPos[(int)canionBridgeHeight].setType(BlockTypes::wooden_slab);
		if (canionBridgeHeight - (int)canionBridgeHeight > 0.5)
		{
			startPos[(int)canionBridgeHeight].setTopPartForSlabs(true);
		}
	}

	int y = height;

	//find grass
	for (; y >= waterLevel; y--)
	{
		//if (startPos[y].getType() != BlockTypes::air)

	
		if (startPos[y].getType() == BlockTypes::stone)
		{

			if (stoneSpikes < 0.32)
			{
				startPos[y].setType(surfaceBlock);
			}

			for (y--; y > height - 4; y--) //todo randomness here
			{
				if (startPos[y].getType() != BlockTypes::air)
				{
					if (stoneSpikes < 0.32)
					{
						startPos[y].setType(secondBlock);
					}
				}
			}

			break;
		}
	}

	BlockType waterType = BlockTypes::water;
	BlockType secondaryWaterBlock = BlockTypes::water;

	if (biome.isICy)
	{
		if ( 
			getIceNoise(x, z) > 0.5 ||
			(riverValue > 0.01 && riverValue < 0.95) ||
			(lakeValue > 0.01 && lakeValue < 0.90)
			)
		{
			waterType = BlockTypes::ice;
		}

	}

	//if(biome.isICy && islake)

	for (y = waterLevel+2; y >= 20; y--)
	{

		if (
			y <= waterLevel &&
			startPos[y].getType() == BlockTypes::stone && 
			(startPos[y+1].getType() == waterType ||
			startPos[y + 1].getType() == biome.secondaryBlock
			))
		{
			//water block

			//add swamp blocks
			if (swampValue >= 0.5 && biomeHeight == 2)
			{

				startPos[y].setType(biome.swampBlock.getRandomBLock(randomNumber));
			}else
			if (riverValue > 0.2)
			{
				BlockVariation riverShape;

				if (riverValue > 0.9)
				{
					riverShape.block.push_back(BlockTypes::stone);
				}

				riverShape.block.push_back(BlockTypes::stone);
				riverShape.block.push_back(BlockTypes::gravel);
				riverShape.block.push_back(BlockTypes::gravel);
				riverShape.block.push_back(BlockTypes::cobblestone);
				riverShape.block.push_back(BlockTypes::cobblestone);

				startPos[y].setType(riverShape.getRandomBLock(randomNumber));
			}
			else
			{
				//under water block
				BlockVariation underWaterLayerBlock;
				underWaterLayerBlock.block.push_back(BlockTypes::stone);
				underWaterLayerBlock.block.push_back(BlockTypes::stone);
				underWaterLayerBlock.block.push_back(BlockTypes::stone);
				underWaterLayerBlock.block.push_back(BlockTypes::gravel);
				underWaterLayerBlock.block.push_back(BlockTypes::gravel);
				underWaterLayerBlock.block.push_back(BlockTypes::cobblestone);

				if (currentInterpolatedValue > 1.1 && currentInterpolatedValue <= 2.f)
				{
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
					underWaterLayerBlock.block.push_back(BlockTypes::sand);
				}

				startPos[y].setType(underWaterLayerBlock.getRandomBLock(randomNumber));

			}

		}else
		if (startPos[y].getType() == BlockTypes::air)
		{
			if(road && y == waterLevel+ 1 + int(1.2f * riverValue)
				&& startPos[y-1].getType() == BlockTypes::air && biomeHeight == 2
				)
			{
				if (int(1.2f * riverValue))
				{
					startPos[y].setType(BlockTypes::wooden_slab);
				}
				else
				{
					startPos[y].setType(BlockTypes::wooden_plank);
				}

			}
			else
			{
				if (y == waterLevel)
				{
					startPos[y].setType(waterType);
				}
				else if(y < waterLevel)
				{
					startPos[y].setType(secondaryWaterBlock);
				}
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


uint32_t hash(uint32_t x, uint32_t y)
{
	x ^= y * 0x51d7348d;
	x ^= x >> 15;
	x *= 0x85ebca6b;
	x ^= x >> 13;
	x *= 0xc2b2ae35;
	x ^= x >> 16;
	return x;
}

constexpr uint32_t wangHash(uint32_t x)
{
	x = (x ^ 61) ^ (x >> 16);
	x *= 9;
	x ^= x >> 4;
	x *= 0x27d4eb2d;
	x ^= x >> 15;
	return x;
}

constexpr uint32_t hash(int32_t x, int32_t y, int32_t z)
{
	uint32_t ux = static_cast<uint32_t>(x) * 73856093u;
	uint32_t uy = static_cast<uint32_t>(y) * 19349663u;
	uint32_t uz = static_cast<uint32_t>(z) * 83492791u;

	return wangHash(ux ^ uy ^ uz);
}

float hashNormalized(uint32_t h)
{
	return (h & 0xFFFFFF) / float(0x1000000);;
}

// Returns true for some (x, y), ensuring no adjacent (x, y) is also true
// Ensures no "true" values within `radius`, checking only in positive directions
bool generateFeature(int x, int y, int seedHash, float threshold = 0.1f, int radius = 2)
{
	uint32_t h = hash(x, y, 0);
	float value = hashNormalized(h);// Normalize to [0,1]

	if (value >= threshold) return false; // Not selected

	// Check only in positive directions to avoid redundant checks
	for (int dx = 0; dx <= radius; ++dx)
	{
		for (int dy = 0; dy <= radius; ++dy)
		{
			if (dx == 0 && dy == 0) continue; // Skip self
			if (dx * dx + dy * dy > radius * radius) continue; // Ensure inside circle

			uint32_t neighborHash = hash(x + dx, y + dy, seedHash);
			float neighborValue = (neighborHash & 0xFFFFFF) / float(0x1000000);

			if (neighborValue < threshold) return false; // Conflict, discard
		}
	}

	return true; // Passed checks
}

void generateChunk(ChunkData& c, WorldGenerator &wg, StructuresManager &structuresManager, BiomesManager &biomesManager,
	std::vector<StructureToGenerate> &generateStructures)
{
	//PL::Profiler profiler;
	//profiler.start();

	if (wg.isSuperFlat)
	{
		//super flat
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				for (int y = 0; y < 256; y++)
				{
					if (y <= 4)
					{
						c.unsafeGet(x, y, z).setType(BlockTypes::stone);
					}
					else if(y <= 7)
					{
						c.unsafeGet(x, y, z).setType(BlockTypes::dirt);
					}
					else if (y == 8)
					{
						c.unsafeGet(x, y, z).setType(BlockTypes::grassBlock);
					}
					else
					{
						c.unsafeGet(x, y, z).setType(BlockTypes::air);
					}
		
				}
			}
		return;
	}


	float interpolateValues[16 * 16] = {};
	float borderingFactor[16 * 16] = {};
	float tightBorders[16 * 16] = {};
	float vegetationMaster = 0;
	float xCellValue = 0;
	float zCellValue = 0;
	float biomeType = 0;
	int currentBiomeHeight = wg.getRegionHeightAndBlendingsForChunk(c.x, c.z,
		interpolateValues, borderingFactor, vegetationMaster, tightBorders, xCellValue, zCellValue,
		biomeType);

	auto &biome = biomesManager.biomes[biomesManager.pickBiomeIndex(biomeType)];
	//auto &biome = biomesManager.biomes[BiomesManager::snow];

	c.vegetation = vegetationMaster;
	c.regionCenterX = xCellValue;
	c.regionCenterZ = zCellValue;

	//vegetationMaster = 1.f;
	float vegetationPower = linearRemap(vegetationMaster, 0, 1, 1.2, 0.4);

	auto interpolator = [&](int *ptr, float value)
	{
		if (value >= 5.f)
		{
			return (float)ptr[5];
		}
		
		if (currentBiomeHeight == value)
		{
			return (float)ptr[currentBiomeHeight];
		}else
		//if (currentBiomeHeight < value)
		{
			float rez = ptr[int(value)];
			float rez2 = ptr[int(value) + 1];

			float interp = value - int(value);

			//return rez;
			return glm::mix(rez, rez2, interp);
		}
		//else
		//{
		//	float rez = ptr[int(value)];
		//	float rez2 = ptr[int(value) + 1];
		//
		//	float interp = value - int(value);
		//
		//	//return rez;
		//	return glm::mix(rez, rez2, interp);
		//}
		
	};
				   
	//water    plains   hills
	//int startValues[] = {22, 45,  66,      72,     80, 140};
	//int maxlevels[] =   {40, 64,  71,      120,     170, 250};

	int startValues[] = {22, 48,  66,      70,     74, 100};
	int maxlevels[] =   {40, 68,  71,      120,     170, 240};
	int biomes[] = {BiomesManager::plains, BiomesManager::plains, 
		BiomesManager::plains, BiomesManager::plains,
		BiomesManager::snow, BiomesManager::snow};

	int valuesToAddToStart[] = {5, 5, 10,  20,  20,  20};
	int valuesToAddToMax[] = {5, 5, 5,  20,  10,  0};
	float peaksPower[] = {1,1, 0.5, 1, 1, 1};

	int hillsValuesToAddToStart[] = {5, 5, 10,  10,  10,  4};
	int hillsValuesToAddToMax[] = {5, 20, 45,  45,  30,  4};

	c.clear();
	
	int xPadd = c.x * 16;
	int zPadd = c.z * 16;

	c.vegetation = vegetationMaster;

#pragma region noises
	alignas(32) static float continentalness[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.continentalnessNoise->FillNoiseSet(continentalness, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		continentalness[i] += 1;
		continentalness[i] /= 2;
		continentalness[i] = powf(continentalness[i], wg.continentalPower);
		continentalness[i] = wg.continentalSplines.applySpline(continentalness[i]);
	}

	alignas(32) static float continentalness2[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.continentalness2Noise->FillNoiseSet(continentalness2, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		continentalness2[i] += 1;
		continentalness2[i] /= 2;
		continentalness2[i] = powf(continentalness2[i], wg.continental2Power);
		continentalness2[i] = wg.continental2Splines.applySpline(continentalness2[i]);
	}

	alignas(32) static float continentalnessPick[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.continentalness2Noise->FillNoiseSet(continentalnessPick, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		continentalnessPick[i] += 1;
		continentalnessPick[i] /= 2;
		continentalnessPick[i] = powf(continentalnessPick[i], wg.continentalnessPickPower);
		continentalnessPick[i] = wg.continentalnessPickSplines.applySpline(continentalnessPick[i]);
	}

	alignas(32) static float continentalnessFinal[CHUNK_SIZE * CHUNK_SIZE] = {};
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{

		continentalnessFinal[i] = glm::mix(continentalness[i], continentalness2[i],
			continentalnessPick[i]);

		//if (continentalnessPick[i] > 0.5)
		//{
		//	continentalnessFinal[i] = continentalness2[i];
		//}
		//else
		//{
		//	continentalnessFinal[i] = continentalness[i];
		//}
	}

	alignas(32) static float peaksAndValies[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.peaksValiesNoise->FillNoiseSet(peaksAndValies, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		peaksAndValies[i] += 1;
		peaksAndValies[i] /= 2;
		peaksAndValies[i] = powf(peaksAndValies[i], wg.peaksValiesPower);
		peaksAndValies[i] = wg.peaksValiesSplines.applySpline(peaksAndValies[i]);
	}


	alignas(32) static float wierdness[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.wierdnessNoise->FillNoiseSet(wierdness, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		wierdness[i] += 1;
		wierdness[i] /= 2;
		wierdness[i] = powf(wierdness[i], wg.wierdnessPower);
		wierdness[i] = wg.wierdnessSplines.applySpline(wierdness[i]);
	}


	alignas(32) static float densityNoise[CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE] = {};
	wg.stone3Dnoise->FillNoiseSet(densityNoise, xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		densityNoise[i] += 1;
		densityNoise[i] /= 2;
		densityNoise[i] = powf(densityNoise[i], wg.stone3Dpower);
		densityNoise[i] = wg.stone3DnoiseSplines.applySpline(densityNoise[i]);
	}


	alignas(32) static float randomSand[CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE] = {};
	wg.randomStonesNoise->FillNoiseSet(randomSand, xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		randomSand[i] += 1;
		randomSand[i] /= 2;
		randomSand[i] = powf(randomSand[i], wg.randomSandPower);
		randomSand[i] = wg.randomSandSplines.applySpline(randomSand[i]);
	}


	alignas(32) static float randomGravel[CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE] = {};
	wg.randomStonesNoise->FillNoiseSet(randomGravel, xPadd, 300, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		randomGravel[i] += 1;
		randomGravel[i] /= 2;
		randomGravel[i] = powf(randomGravel[i], wg.randomSandPower + 0.1);
		randomGravel[i] = wg.randomSandSplines.applySpline(randomGravel[i]);
	}
	

	alignas(32) static float randomClay[CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE] = {};
	wg.randomStonesNoise->FillNoiseSet(randomClay, xPadd, 600, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{	
		randomClay[i] += 1;
		randomClay[i] /= 2;
		randomClay[i] = powf(randomClay[i], wg.randomSandPower + 0.5);
		randomClay[i] = wg.randomSandSplines.applySpline(randomClay[i]);
	}


	alignas(32) static float spagettiNoise[CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE] = {};
	wg.spagettiNoise->FillNoiseSet(spagettiNoise, xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		spagettiNoise[i] += 1;
		spagettiNoise[i] /= 2;
		spagettiNoise[i] = powf(spagettiNoise[i], wg.spagettiNoisePower);
		spagettiNoise[i] = wg.spagettiNoiseSplines.applySpline(spagettiNoise[i]);
	}


	const float SHIFT = 16;
	alignas(32) static float spagettiNoise2[CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE] = {};
	wg.spagettiNoise->FillNoiseSet(spagettiNoise2, xPadd + SHIFT + 6, 0 + SHIFT + 3, zPadd + SHIFT,
		CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		spagettiNoise2[i] += 1;
		spagettiNoise2[i] /= 2;
		spagettiNoise2[i] = powf(spagettiNoise2[i], wg.spagettiNoisePower);
		spagettiNoise2[i] = wg.spagettiNoiseSplines.applySpline(spagettiNoise2[i]);
	}

	alignas(32) static float randomStones[1] = {};
	wg.randomStonesNoise->FillNoiseSet(randomStones, xPadd, 0, zPadd, 1, 1, 1);
	*randomStones = std::pow(((*randomStones + 1.f) / 2.f)*0.5, 3.0);

	
	alignas(32) static float cavesNoise[CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE] = {};
	wg.cavesNoise->FillNoiseSet(cavesNoise, xPadd, 0, zPadd, CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; i++)
	{
		cavesNoise[i] += 1;
		cavesNoise[i] /= 2;
		cavesNoise[i] = powf(cavesNoise[i], wg.cavesPower);
		cavesNoise[i] = wg.cavesSpline.applySpline(cavesNoise[i]);
	}

	alignas(32) static float lakesNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.lakesNoise->FillNoiseSet(lakesNoise, xPadd, 0, zPadd, CHUNK_SIZE, 1, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		lakesNoise[i] += 1;
		lakesNoise[i] /= 2;
		lakesNoise[i] = powf(lakesNoise[i], wg.lakesPower);
		lakesNoise[i] = wg.lakesSplines.applySpline(lakesNoise[i]);
	}

	alignas(32) static float alternativeBlocksNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.alternativePatchesOfBlocks->FillNoiseSet(alternativeBlocksNoise, xPadd, 0, zPadd, CHUNK_SIZE, 1, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		alternativeBlocksNoise[i] += 1;
		alternativeBlocksNoise[i] /= 2;
	}

	alignas(32) static float whiteNoise[(CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)] = {};
	wg.whiteNoise->FillNoiseSet(whiteNoise, xPadd, 0, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1);
	for (int i = 0; i < (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1); i++)
	{
		whiteNoise[i] += 1;
		whiteNoise[i] /= 2;
	}

	alignas(32) static float whiteNoise2[(CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)] = {};
	wg.whiteNoise2->FillNoiseSet(whiteNoise2, xPadd, 0, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1);
	for (int i = 0; i < (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1); i++)
	{
		whiteNoise2[i] += 1;
		whiteNoise2[i] /= 2;
	}

	alignas(32) static float whiteNoise3[(CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)] = {};
	wg.whiteNoise2->FillNoiseSet(whiteNoise3, xPadd, 100, zPadd, CHUNK_SIZE + 1, (1), CHUNK_SIZE + 1);
	for (int i = 0; i < (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1); i++)
	{
		whiteNoise3[i] += 1;
		whiteNoise3[i] /= 2;
	}


	alignas(32) static float stonePatches[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.stonePatchesNoise->FillNoiseSet(stonePatches, xPadd, 0, zPadd, CHUNK_SIZE, 1, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		stonePatches[i] += 1;
		stonePatches[i] /= 2;
		stonePatches[i] = powf(stonePatches[i], wg.stonePatchesPower);
		stonePatches[i] = wg.stonePatchesSpline.applySpline(stonePatches[i]);
	}


	alignas(32) static float riversNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.riversNoise->FillNoiseSet(riversNoise, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		riversNoise[i] += 1;
		riversNoise[i] /= 2;
		riversNoise[i] = powf(riversNoise[i], wg.riversPower);
		riversNoise[i] = wg.riversSplines.applySpline(riversNoise[i]);
	}


	alignas(32) static float hillsDropDownsNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.hillsDropsNoise->FillNoiseSet(hillsDropDownsNoise, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		hillsDropDownsNoise[i] += 1;
		hillsDropDownsNoise[i] /= 2;
		hillsDropDownsNoise[i] = powf(hillsDropDownsNoise[i], wg.hillsDropsPower);
		hillsDropDownsNoise[i] = wg.hillsDropsSpline.applySpline(hillsDropDownsNoise[i]);
	}


	alignas(32) static float roadNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.roadNoise->FillNoiseSet(roadNoise, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		roadNoise[i] += 1;
		roadNoise[i] /= 2;
		roadNoise[i] = powf(roadNoise[i], wg.roadPower);
		roadNoise[i] = wg.roadSplines.applySpline(roadNoise[i]);
	}


	alignas(32) static float treeAmountNoise1[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.treesAmountNoise->FillNoiseSet(treeAmountNoise1, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		treeAmountNoise1[i] += 1;
		treeAmountNoise1[i] /= 2;
		treeAmountNoise1[i] = powf(treeAmountNoise1[i], wg.treesAmountPower);
		treeAmountNoise1[i] = wg.treesAmountSpline.applySpline(treeAmountNoise1[i]);
	}

	alignas(32) static float treeAmountNoise2[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.treesAmountNoise->FillNoiseSet(treeAmountNoise2, xPadd + 10000, 1000, zPadd + 10000, CHUNK_SIZE, (1), CHUNK_SIZE, 2);

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		treeAmountNoise2[i] += 1;
		treeAmountNoise2[i] /= 2;
		treeAmountNoise2[i] = powf(treeAmountNoise2[i], wg.treesAmountPower);
		treeAmountNoise2[i] = wg.treesAmountSpline.applySpline(treeAmountNoise2[i]);
	}

	alignas(32) static float treeTypeNoise1[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.treesTypeNoise->FillNoiseSet(treeTypeNoise1, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		treeTypeNoise1[i] += 1;
		treeTypeNoise1[i] /= 2;
		treeTypeNoise1[i] = powf(treeTypeNoise1[i], wg.treesTypePower);
		treeTypeNoise1[i] = wg.treesTypeSpline.applySpline(treeTypeNoise1[i]);
	}


	alignas(32) static float treeTypeNoise2[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.treesTypeNoise->FillNoiseSet(treeTypeNoise2, xPadd + 10000, 1000, zPadd + 10000, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		treeTypeNoise2[i] += 1;
		treeTypeNoise2[i] /= 2;
		treeTypeNoise2[i] = powf(treeTypeNoise2[i], wg.treesTypePower);
		treeTypeNoise2[i] = wg.treesTypeSpline.applySpline(treeTypeNoise2[i]);
	}

	alignas(32) static float randomHills[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.randomHillsNoise->FillNoiseSet(randomHills, xPadd, 0, zPadd, CHUNK_SIZE, 1, CHUNK_SIZE);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		randomHills[i] += 1;
		randomHills[i] /= 2;
		randomHills[i] = powf(randomHills[i], wg.randomHillsPower);
		randomHills[i] = wg.randomHillsSplines.applySpline(randomHills[i]);
	}

	alignas(32) static float swampNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.swampNoise->FillNoiseSet(swampNoise, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		swampNoise[i] += 1;
		swampNoise[i] /= 2;
		swampNoise[i] = powf(swampNoise[i], wg.swampPower);
		swampNoise[i] = wg.swampSplines.applySpline(swampNoise[i]);
	}

	alignas(32) static float swampMaskNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.swampMask->FillNoiseSet(swampMaskNoise, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		swampMaskNoise[i] += 1;
		swampMaskNoise[i] /= 2;
		swampMaskNoise[i] = powf(swampMaskNoise[i], wg.swampMaskPower);
		swampMaskNoise[i] = wg.swampMaskSplines.applySpline(swampMaskNoise[i]);
	}

	alignas(32) static float stoneSpikesNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.stoneSpikesNoise->FillNoiseSet(stoneSpikesNoise, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		stoneSpikesNoise[i] += 1;
		stoneSpikesNoise[i] /= 2;
		stoneSpikesNoise[i] = powf(stoneSpikesNoise[i], wg.stoneSpikesPower);
		stoneSpikesNoise[i] = wg.stoneSpikesSplines.applySpline(stoneSpikesNoise[i]);
	}

	alignas(32) static float stoneSpikesMaskNoise[CHUNK_SIZE * CHUNK_SIZE] = {};
	wg.stoneSpikesMask->FillNoiseSet(stoneSpikesMaskNoise, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		stoneSpikesMaskNoise[i] += 1;
		stoneSpikesMaskNoise[i] /= 2;
		stoneSpikesMaskNoise[i] = powf(stoneSpikesMaskNoise[i], wg.stoneSpikesMaskPower);
		stoneSpikesMaskNoise[i] = wg.stoneSpikesMaskSplines.applySpline(stoneSpikesMaskNoise[i]);
	}

	if (biome.isICy)
	{
		wg.iceNoise->FillNoiseSet(iceNoise, xPadd, 0, zPadd, CHUNK_SIZE, (1), CHUNK_SIZE, 1);
		for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
		{
			iceNoise[i] += 1;
			iceNoise[i] /= 2;
			iceNoise[i] = powf(iceNoise[i], wg.iceNoisePower);
			iceNoise[i] = wg.iceNoiseSplines.applySpline(iceNoise[i]);
		}
	}

#pragma endregion


#pragma region gets

	auto getNoiseVal = [](int x, int y, int z)
	{
		//return continentalness[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
		//return continentalness2[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
		return continentalnessFinal[x * CHUNK_SIZE * (1) + y * CHUNK_SIZE + z];
	};

	auto getRivers = [](int x, int z)
	{
		return riversNoise[x * CHUNK_SIZE + z];
	};

	auto getHillsDropDowns = [](int x, int z)
	{
		return hillsDropDownsNoise[x * CHUNK_SIZE + z];
	};

	auto getTreeAmount1 = [](int x, int z)
	{
		return treeAmountNoise1[x * CHUNK_SIZE + z];
	};
	
	auto getTreeAmount2 = [](int x, int z)
	{
		return treeAmountNoise2[x * CHUNK_SIZE + z];
	};

	auto getTreeType1 = [](int x, int z)
	{
		return treeTypeNoise1[x * CHUNK_SIZE + z];
	};

	auto getTreeType2 = [](int x, int z)
	{
		return treeTypeNoise2[x * CHUNK_SIZE + z];
	};

	auto getRoads = [](int x, int z)
	{
		return roadNoise[x * CHUNK_SIZE + z];
	};

	auto getPeaksAndValies = [](int x, int z)
	{
		return peaksAndValies[x * CHUNK_SIZE + z];
	};
	
	auto getWhiteNoiseVal = [](int x, int z)
	{
		return whiteNoise[x * (CHUNK_SIZE + 1) + z];
	};

	auto getLakesNoiseVal = [](int x, int z)
	{
		return lakesNoise[x * (CHUNK_SIZE) + z];
	};

	auto getAlternativeNoiseVal = [](int x, int z)
	{
		return alternativeBlocksNoise[x * (CHUNK_SIZE)+z];
	};

	auto getRandomHillsVal = [](int x, int z)
	{
		return randomHills[x * (CHUNK_SIZE)+z];
	};

	auto getWhiteNoise2Val = [](int x, int z)
	{
		return whiteNoise2[x * (CHUNK_SIZE + 1) + z];
	};

	auto getWhiteNoise3Val = [](int x, int z)
	{
		return whiteNoise3[x * (CHUNK_SIZE + 1) + z];
	};

	auto getStonePatches = [](int x, int z)
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

	auto getDensityNoiseVal = [](int x, int y, int z) //todo more cache friendly operation here please
	{
		return densityNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT) + y * CHUNK_SIZE + z];
	};

	auto getRandomSandVal = [](int x, int y, int z)
	{
		return randomSand[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getRandomGravelVal = [](int x, int y, int z)
	{
		return randomGravel[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getRandomClayVal = [](int x, int y, int z)
	{
		return randomClay[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getSpagettiNoiseVal = [](int x, int y, int z) //todo more cache friendly operation here please
	{
		return spagettiNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getSpagettiNoiseVal2 = [](int x, int y, int z) //todo more cache friendly operation here please
	{
		return spagettiNoise2[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};

	auto getCavesNoiseVal = [](int x, int y, int z)
	{
		return cavesNoise[x * CHUNK_SIZE * (CHUNK_HEIGHT)+y * CHUNK_SIZE + z];
	};


	auto getWierdness = [](int x, int z)
	{
		return wierdness[x * CHUNK_SIZE + z];
	};

	auto getSwampNoise = [](int x, int z)
	{
		return swampNoise[x * CHUNK_SIZE + z];
	};

	auto getSwampMaskNoise = [](int x, int z)
	{
		return swampMaskNoise[x * CHUNK_SIZE + z];
	};

	auto getStoneSpikesNoise = [](int x, int z)
	{
		return stoneSpikesNoise[x * CHUNK_SIZE + z];
	};

	auto getStoneSpikesMaskNoise = [](int x, int z)
	{
		return stoneSpikesMaskNoise[x * CHUNK_SIZE + z];
	};

	auto getIntFromFloat = [](float f, int maxExclusive)
	{
		if (f >= 0.99) { f = 0.99; }
		return int(f * maxExclusive);
	};

	auto screenBlend = [](float a, float b)
	{
		return 1.f - (1.f - a) * (1.f - b);
	};

#pragma endregion


#pragma region could generate medium structures

	auto spawnProbability = [](float structuresOnceEveryChunks)
	{
		return 1.0 / (structuresOnceEveryChunks * structuresOnceEveryChunks);
	};

	unsigned int seedHash = wg.continentalnessNoise->GetSeed();
	bool couldGenerateMediumStructures = generateFeature(c.x, c.z, seedHash++, spawnProbability(14), 4);


#pragma endregion

	glm::ivec2 lowestPointInChunk = {};
	int lowestLevelInChunk = 300;

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{

			//this will be some random places where there is stone instead of grass or sand or whatever
			float stonePatchesVal = getStonePatches(x, z);

			float wierdnessTresshold = 0.4;
			float localWierdness = getWierdness(x, z);
			//localWierdness = 0;

			float peaks = getPeaksAndValies(x, z);
			float hills = getRandomHillsVal(x, z);
			float continentalness = getNoiseVal(x, 0, z);

			float currentInterpolatedValue = interpolateValues[z + x * CHUNK_SIZE];
			
			float treeAmountVal1 = getTreeAmount1(x, z);
			float treeAmountVal2 = getTreeAmount2(x, z);

			float treeType1 = getTreeType1(x, z);
			float treeType2 = getTreeType2(x, z);

			float lakeNoiseVal = getLakesNoiseVal(x, z); //this one is 1 for lake 0 for no lake
			float localBorderingFactor = borderingFactor[z + x * CHUNK_SIZE];

			float localSwampVal = getSwampNoise(x, z) * getSwampMaskNoise(x, z);
			float localStoneSpikes = getStoneSpikesNoise(x, z) * getStoneSpikesMaskNoise(x, z);

			if (tightBorders[z + x * CHUNK_SIZE])
			{
				c.setBorderFlag(x, z);
			}

			constexpr int stoneNoiseStartLevel = 1;



			auto getHeightForBiome = [=](int currentHeightLevel, bool &placeRoad, float
				&roadValue, float &outLakeNoiseVal, float &underGroundRivers,
				float &riverChanceValue, int &heightDiff, int &startLevel, int maxMountainLevel
				, float &outCanionValue, int &valueWithoudCanionDropDown
				)
			{

				outCanionValue = 0;
				float newPeaks = peaks;
				outLakeNoiseVal = lakeNoiseVal;
				valueWithoudCanionDropDown = 0;

			#pragma region roads

				//plains roads
				if (currentHeightLevel == 2)
				{
					roadValue = getRoads(x, z);

					if (roadValue < 0.6)
					{
						newPeaks = glm::mix(0.5f, newPeaks, roadValue);

						if (roadValue < 0.5)
						{
							placeRoad = true;
						}
					};


					//newMaxMountainLevel = glm::mix(newMaxMountainLevel-1, newMaxMountainLevel, roadValue);


					//localWierdness = glm::mix(1.f, localWierdness, roadValue);
					//wierdnessTresshold = glm::mix(0.f, wierdnessTresshold, roadValue);
					//wierdnessTresshold = 0;

					//peaks = glm::mix(0.5f, peaks, roadValue);
				}
				else if (currentHeightLevel == 3
					//&& localWierdness < 0.9f
					&& continentalness < 0.42f)
				{
					roadValue = getRoads(x, z);

					if (roadValue < 0.6)
					{
						newPeaks = glm::mix(0.5f, newPeaks, roadValue);

						if (roadValue < 0.5)
						{
							placeRoad = true;
						}
					};

					//wierdnessTresshold = glm::mix(0.f, wierdnessTresshold, roadValue);
					//wierdnessTresshold = 0;

					//newMaxMountainLevel = glm::mix(newMaxMountainLevel - 1, newMaxMountainLevel, roadValue);
					//startLevel = glm::mix(startLevel - 1, startLevel, roadValue);

					//localWierdness = glm::mix(1.f, localWierdness, roadValue);
					//todo investigate wierdness

					//peaks = glm::mix(0.5f, peaks, roadValue);

				}
			#pragma endregion


			#pragma region lakes
				if (outLakeNoiseVal < 0.1)
				{
					outLakeNoiseVal = 0;
				}

				if (currentHeightLevel != 2)
				{
					outLakeNoiseVal = 0;
				}
				else
				{
					outLakeNoiseVal *= (1.f - localBorderingFactor);

					//if (outLakeNoiseVal > 0.1)
					//{
					//	newStartLevel = glm::mix(newStartLevel, waterLevel - 5, outLakeNoiseVal);
					//	newMaxMountainLevel = glm::mix(newMaxMountainLevel, waterLevel - 2, outLakeNoiseVal);
					//}
				}
			#pragma endregion


			#pragma region rivers
				underGroundRivers = 0;
				riverChanceValue = 0;

				int newStartLevel = startLevel;
				int newMaxMountainLevel = maxMountainLevel;

				//plains rivers
				if (currentHeightLevel == 2)
				{
					float rivers = getRivers(x, z);
					rivers *= (1 - lakeNoiseVal);

					riverChanceValue = 1 - rivers;

					newStartLevel = glm::mix(waterLevel - 5, newStartLevel, rivers);
					newMaxMountainLevel = glm::mix(waterLevel - 2, newMaxMountainLevel, rivers);
				}
				else if (currentHeightLevel >= 3)
				{
					float rivers = getRivers(x, z);
					underGroundRivers = 1 - rivers;
					riverChanceValue = underGroundRivers;

					if (currentHeightLevel == 3)
					{
						newStartLevel = glm::mix(newStartLevel, waterLevel + 4, (1.f - rivers) * localBorderingFactor);
						newMaxMountainLevel = glm::mix(newMaxMountainLevel, waterLevel + 18, (1.f - rivers) * localBorderingFactor);
					}


				}
			#pragma endregion



			#pragma region hills drop downs
				//hills drop downs

				if (currentHeightLevel == 3)
				{
					float dropDown = getHillsDropDowns(x, z);

					//decrease contribution in plains
					if (currentHeightLevel == 2)
					{
						dropDown /= 2.f;
					}

					//make sure we don't do this near edges...
					if (interpolateValues[z + x * CHUNK_SIZE] > 3.f)
					{
						float interpolator = interpolateValues[z + x * CHUNK_SIZE] - int(interpolateValues[z + x * CHUNK_SIZE]);
						dropDown = screenBlend(dropDown, interpolator);
					}
					//else
					//if(interpolateValues[z + x * CHUNK_SIZE] < 2.8f)
					//{
					//	float interpolator = interpolateValues[z + x * CHUNK_SIZE] - int(interpolateValues[z + x * CHUNK_SIZE]);
					//	dropDown = dropDown * interpolator;
					//}

					if (dropDown < 0.9)
					{
						newStartLevel = glm::mix(waterLevel + 1, newStartLevel, dropDown);
						newMaxMountainLevel = glm::mix(waterLevel + 6, newMaxMountainLevel, dropDown);
					}

					outCanionValue = 1.f - dropDown;

					{
						if (newMaxMountainLevel <= newStartLevel) 
						{
							valueWithoudCanionDropDown = newMaxMountainLevel; 
						}
						else
						{

							heightDiff = newMaxMountainLevel - newStartLevel;
							int height = int(newStartLevel + continentalness * heightDiff);
							valueWithoudCanionDropDown = height;
						}
					}

				}

			#pragma endregion


			#pragma region swamp
				if (currentHeightLevel == 2)
				{
					if (localSwampVal > 0.05)
					{

						//remap to 0 -> 1
						//float swamp = glm::clamp((localSwampVal - 0.5f) * 2.f, 0.f, 1.f);
						float swamp = localSwampVal;

						newStartLevel = glm::mix(newStartLevel, waterLevel - 2, swamp);
						newMaxMountainLevel = glm::mix(newMaxMountainLevel, waterLevel, swamp);
					}

				}
			#pragma endregion

			#pragma region stone spikes

				//if (currentHeightLevel == 2)
				{
					if (localStoneSpikes > 0.2)
					{
						//remap to 0 -> 1
						//float swamp = glm::clamp((localSwampVal - 0.5f) * 2.f, 0.f, 1.f);

						newStartLevel = glm::mix(newStartLevel, newStartLevel + 25, localStoneSpikes);
						newMaxMountainLevel = glm::mix(newMaxMountainLevel, newMaxMountainLevel + 40, localStoneSpikes);
					}

				}

			#pragma endregion


			#pragma region determine height

				int hillsValueToAddToStart = hillsValuesToAddToStart[currentHeightLevel];
				int hillsValueToAddToEnd = hillsValuesToAddToMax[currentHeightLevel];

				int valueToAddToStart = valuesToAddToStart[currentHeightLevel];
				int valueToAddToEnd = valuesToAddToMax[currentHeightLevel];

				newPeaks = std::powf(newPeaks, peaksPower[currentHeightLevel]);
				newStartLevel += (newPeaks * valueToAddToStart) - 3;
				newMaxMountainLevel += (newPeaks * valueToAddToEnd) - 3;

				newStartLevel += (hills * hillsValueToAddToStart);
				newMaxMountainLevel += (hills * hillsValueToAddToEnd);

				if (newStartLevel >= newMaxMountainLevel)
				{
					newStartLevel = newMaxMountainLevel;
					newStartLevel--;
					newMaxMountainLevel++;
				}

				if (newMaxMountainLevel <= newStartLevel) { newStartLevel = newMaxMountainLevel - 1; }
				heightDiff = newMaxMountainLevel - newStartLevel;

				//c.unsafeGetCachedBiome(x, z) = biomeIndex;


				int height = int(newStartLevel + continentalness * heightDiff);

				if (valueWithoudCanionDropDown == 0)
				{
					valueWithoudCanionDropDown = height;
				}

			#pragma endregion

				return height;

			};

			//biomesManager.biomes.size()



			bool placeRoad = 0;
			float roadValue = 0;
			float newLakeNoiseVal = lakeNoiseVal;
			float underGroundRivers = 0;
			float riverChanceValue = 0;
			float canionValue = 0;
			int heightDiff = 0;
			int valueWithoutCanionDropDown = 0;

			int startLevel = interpolator(startValues, currentInterpolatedValue);
			int maxMountainLevel = interpolator(maxlevels, currentInterpolatedValue);

			//int startLevel = startValues[currentBiomeHeight];
			//int maxMountainLevel = maxlevels[currentBiomeHeight];

			int newStartLevel = startLevel;

			int height = getHeightForBiome(currentBiomeHeight, placeRoad, roadValue, newLakeNoiseVal,
				underGroundRivers, riverChanceValue, heightDiff, 
				newStartLevel, maxMountainLevel, canionValue, valueWithoutCanionDropDown);

			//if (currentInterpolatedValue != (float)((int)(currentInterpolatedValue))
			//	&& currentInterpolatedValue < 5.f
			//	)
			//{
			//
			//	int newBiomeHeight = currentBiomeHeight + 1;
			//
			//	if (currentBiomeHeight > currentInterpolatedValue)
			//	{
			//		newBiomeHeight = currentBiomeHeight - 1;
			//	}
			//
			//	bool placeRoad = 0;
			//	float roadValue = 0;
			//	float newLakeNoiseVal = lakeNoiseVal;
			//	float underGroundRivers = 0;
			//	float riverChanceValue = 0;
			//	int heightDiff = 0;
			//	int startLevel = startValues[newBiomeHeight];
			//	int maxMountainLevel = maxlevels[newBiomeHeight];
			//	int newStartLevel = startLevel;
			//
			//	int height2 = getHeightForBiome(newBiomeHeight, placeRoad, roadValue, newLakeNoiseVal,
			//		underGroundRivers, riverChanceValue, heightDiff, newStartLevel, maxMountainLevel);
			//
			//	if (currentBiomeHeight > currentInterpolatedValue)
			//	{
			//		height = glm::mix((float)height2, (float)height, currentInterpolatedValue - (int)currentInterpolatedValue);
			//	}
			//	else
			//	{
			//		height = glm::mix((float)height, (float)height2, currentInterpolatedValue - (int)currentInterpolatedValue);
			//	}
			//
			//}
			
			//if (localBorderingFactor > 0.95)
			//{
			//	c.setBorderFlag(x, z);
			//	//std::cout << "YESSS ";
			//}
		

		#pragma region borderings

			bool sandShore = 0;
			if (
				//currentBiomeHeight == 2 
				currentInterpolatedValue <= 1.9f && currentInterpolatedValue > 1.1f
				&& localBorderingFactor > 0.1)
			{
				sandShore = 1;
			}

		#pragma endregion

	


			

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

					float heightNormalized = (y - newStartLevel) / (float)heightDiff;
					//float heightNormalized = y / 256.f;
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

			if (underGroundRivers > 0.2)
			{
				int water = waterLevel;
				int min = water - 4 * underGroundRivers;
				int max = water + 8 * underGroundRivers;

				for (int y = min; y < max; y++)
				{
					c.unsafeGet(x, y, z).setType(BlockTypes::air);
				}

				
			}

			float riverValueForPlacingRocks = screenBlend(riverChanceValue, newLakeNoiseVal);
			if (currentBiomeHeight != 2 && currentBiomeHeight != 1)
			{
				riverValueForPlacingRocks = 0;
			}

			calculateBlockPass1(firstH, &c.unsafeGet(x, 0, z), biome, placeRoad, roadValue, 
				getWhiteNoiseVal(x,z), sandShore, stonePatchesVal > 0.5, 
				riverValueForPlacingRocks, currentInterpolatedValue, 
				getAlternativeNoiseVal(x, z), localSwampVal, currentBiomeHeight, 
				localStoneSpikes, canionValue, valueWithoutCanionDropDown, x, z, 
				lakeNoiseVal);

			//all caves
			for (int y = 2; y < firstH; y++)
			{
				
				auto caveDensity = getCavesNoiseVal(x, y, z);
				//auto caveDensityBellow = getCavesNoiseVal(x, y - 1, z);
				//auto caveDensityBellow2 = getCavesNoiseVal(x, y-2, z);
				//|| caveDensityBellow < 0.5 || caveDensityBellow2 < 0.5

				if (caveDensity < 0.5 )
				{
					//cave
					if (c.unsafeGet(x, y, z).getType() != BlockTypes::water)
					{
						c.unsafeGet(x, y, z).setType(BlockTypes::air);
					}
				}
				else
				{
					auto density = getSpagettiNoiseVal(x, y, z);
					float density2 = getSpagettiNoiseVal2(x, y, z);
					density = screenBlend(density, density2);

					float height = y / (CHUNK_HEIGHT-1);
					//height = height * height;
					float heightRemapped = linearRemap(height, 0, 1, 0.25, 0.1);


					//bias = powf(bias, wg.spagettiNoiseBiasPower);

					if (density > 0.75 - heightRemapped)
					{
						//stone
					}
					else
					{
						//spagetti cave
						if (c.unsafeGet(x, y, z).getType() != BlockTypes::water)
						{
							c.unsafeGet(x, y, z).setType(BlockTypes::air);
						}
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
				else if (treeType == Biome::treeBirchTall)
				{	
					str.type = Structure_BirchTree;
					str.addRandomTreeHeight = true;
					str.replaceLogWith = BlockTypes::birch_log;
					str.replaceLeavesWith = BlockTypes::birch_leaves;
				}
				else if (treeType == Biome::treeRedBirchTall)
				{
					str.type = Structure_BirchTree;
					str.addRandomTreeHeight = true;
					str.replaceLogWith = BlockTypes::birch_log;
					str.replaceLeavesWith = BlockTypes::spruce_leaves_red;
				}
				else if (treeType == Biome::treeRedBirch)
				{
					str.type = Structure_BirchTree;
					//str.addRandomTreeHeight = true;
					//str.replaceLogWith = BlockTypes::birch_log;
					str.replaceLeavesWith = BlockTypes::spruce_leaves_red;
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

					//more stones near rivers
					if (riverChanceValue > 0.2 && currentBiomeHeight == 2)
					{
						stonesChance += 0.002;
					}

					//if()

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

				auto generateOneFeature = [&](float treeAmount, 
					VegetationNoiseSettings &veg)
				{
					treeAmount = std::powf(treeAmount, vegetationPower);
					float noiseVal = treeAmount;

					//one distribution element, can be multiple things there tho
					for (auto &entry : veg.entry)
					{
						if (noiseVal >= entry.minTresshold && entry.maxTresshold >= noiseVal)
						{

							float chanceRemap = linearRemap(noiseVal, entry.minTresshold, entry.maxTresshold,
								entry.chanceRemap.x, entry.chanceRemap.y);

							//float chanceRemap = linearRemap(noiseVal, 0, 1,
							//	entry.chanceRemap.x, entry.chanceRemap.y);

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
										generatedSomethingElse = true;
									}
									else if (growElement.treeType)
									{

										//don't put trees too together...
										float noiseVal1 = getWhiteNoise2Val(x + 1, z);
										float noiseVal2 = getWhiteNoise2Val(x, z + 1);
										float noiseVal3 = getWhiteNoise2Val(x + 1, z + 1);

										if (0 &&
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
										generatedSomethingElse = true;
										break;
									}
								}

							}

						}

						if (generatedSomethingElse) { break; }
					}
				};

				if (couldGenerateMediumStructures)
				{
					
					//we don't do anything yet
					
				}
				else
				{
					//generate small structures
					if (!generatedSomethingElse)
					{
						int type = getIntFromFloat(treeType1, biomesManager.greenBiomesTrees.size());
						generateOneFeature(treeAmountVal1, biomesManager.greenBiomesTrees[type]);
					}

					if (!generatedSomethingElse)
					{
						int type = getIntFromFloat(treeType2, biomesManager.greenBiomesTrees.size());
						generateOneFeature(treeAmountVal2, biomesManager.greenBiomesTrees[type]);
					}

					if (!generatedSomethingElse)
					{
						//todo
						generateOneFeature(0.6, biomesManager.greenBiomesGrass[0]);
					}
				}

				

				/*
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
				*/

			}

			if (firstH < lowestLevelInChunk)
			{
				lowestLevelInChunk = firstH;
				lowestPointInChunk = glm::ivec2(x, z);
			}

		}


	#pragma region big structures

		#pragma region templates

		auto smallAbandonedHouse = [&]()
		{
			StructureToGenerate str;
			str.type = Structure_AbandonedHouse;

			str.pos = {lowestPointInChunk.x + xPadd, lowestLevelInChunk, lowestPointInChunk.y + zPadd};
			str.randomNumber1 = getWhiteNoise3Val(lowestPointInChunk.x, lowestPointInChunk.y);
			str.randomNumber2 = getWhiteNoise3Val(lowestPointInChunk.x + 1, lowestPointInChunk.y);
			str.randomNumber3 = getWhiteNoise3Val(lowestPointInChunk.x + 1, lowestPointInChunk.y + 1);
			str.randomNumber4 = getWhiteNoise3Val(lowestPointInChunk.x, lowestPointInChunk.y + 1);
			str.setDefaultSmallBuildingSettings();

			generateStructures.push_back(str);
		};
		
		#pragma endregion



		if (couldGenerateMediumStructures)
		{

			float generateStructureChance = hashNormalized(hash(c.x, c.z, seedHash++));

			//not inside oceans
			if (currentBiomeHeight > 1)
			{

				//if(generateStructureChance)

				smallAbandonedHouse();




			}

			
		}

	#pragma endregion


	//profiler.end();
	//std::cout << "Time ms: " << profiler.rezult.timeSeconds * 1000 << "\n";


}

void WorldGenerator::generateChunkPreview(gl2d::Texture &t, glm::ivec2 size, glm::ivec2 pos)
{

	struct Color
	{
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		unsigned char a = 255;
	};

	static std::vector<Color> chunkPreviewData;
	chunkPreviewData.clear();

	if (size.x <= 0) { return; }
	if (size.y <= 0) { return; }
	if (size.x > 4'000) { size.x = 4000; }
	if (size.y > 4'000) { size.y = 4000; }

	chunkPreviewData.resize(size.x * size.y);
	static float values[256];
	static float borderingFactor[16 * 16];
	float vegetationMaster = 0;
	static float tightBorders[16 * 16];
	float xValuue = 0;
	float zValue = 0;
	float biomeTypeRandomValue = 0;

	for (int y = 0; y < size.y; y+= CHUNK_SIZE)
		for (int x = 0; x < size.x; x+= CHUNK_SIZE)
		{

			int height = getRegionHeightAndBlendingsForChunk(divideChunk(x), divideChunk(y), values,
				borderingFactor, vegetationMaster, tightBorders, xValuue, zValue, biomeTypeRandomValue);

			glm::vec4 color = {};

			if (height == 0 || height == 1)
			{
				color = Colors_Blue;
			}else if (height == 4)
			{
				color = Colors_Gray;
			}
			else if (height == 5)
			{
				color = Colors_White;
			}
			else
			{
				color = Colors_Green;
			}

			for (int j = y; j < std::min(size.y, y + CHUNK_SIZE); j++)
				for (int i = x; i < std::min(size.x, x + CHUNK_SIZE); i++)
				{
					auto c = color;

					int indexX = i - x;
					int indexY = j - y;

					if (tightBorders[indexY + indexX * CHUNK_SIZE] != 0)
					{
						c = Colors_Red;
					}

					Color cfinal{c.r * 255,c.g * 255,c.b * 255,c.a * 255};
					chunkPreviewData[i + j * size.x] = cfinal;
				}



		}

	t.createFromBuffer((char*)chunkPreviewData.data(), size.x, size.y, true, false);

	//t.create1PxSquare();


}
