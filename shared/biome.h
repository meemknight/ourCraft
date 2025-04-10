#pragma once
#include <staticVector.h>
#include <blocks.h>
#include <glm/glm.hpp>
#include <vector>


struct BlockVariation
{
	StaticVector<BlockTypes, 16> block;
	BlockTypes getRandomBLock(float f)
	{
		if (f >= 0.99) { f = 0.99; }
		return block[int(f * block.size())];
	}
};

struct GrowElement
{
	//only one or the other!
	BlockTypes block = BlockTypes::air;
	unsigned char treeType = 0;
};

struct GrowingThing
{
	StaticVector<GrowElement, 10> elements;
	StaticVector<BlockTypes, 5> growOn;
};


struct VegetationSettings
{
	float minTresshold = 0;
	float maxTresshold = 1;
	glm::vec2 chanceRemap = {0, 0.95};

	GrowingThing growThing;
};


//this are the settings for one vegetation noise...
//there can be multiple things stacked here
struct VegetationNoiseSettings
{

	StaticVector<VegetationSettings, 4> entry;


};

struct BlockSurfaceAndSecondary
{
	BlockType surfaceBlock;
	BlockType secondaryBlock;
};


struct Biome
{

	enum
	{
		treeNone = 0,
		treeNormal,
		treeNormalTall,
		treeJungle,
		treeSpruceTallOakCenter,
		treeSpruceTallOakCenterRed,
		treeSpruceTallOakCenterYellow,
		treePalm,
		treeBirch,
		treeBirchTall,
		treeRedBirchTall,
		treeRedBirch,
		treeSpruce,
		treeTallOak,
		abandonedHouse,
	};

	const char *name = "";
	glm::vec3 color = {};
	bool isICy = 0;

	BlockType surfaceBlock;
	BlockType secondaryBlock; //todo add height variation here

	StaticVector< BlockSurfaceAndSecondary, 40> blockVariations;

	//StaticVector<VegetationNoiseSettings, 4> vegetationNoises;

	//BlockType grassType;
	//BlockType waterType;
	//BlockType waterTypeSecond;
	
	BlockVariation swampBlock;
};

struct BiomesManager
{

	std::vector<Biome> biomes;

	std::vector<int> biomeIndexes;

	std::vector<VegetationNoiseSettings> greenBiomesTrees;

	std::vector<VegetationNoiseSettings> greenBiomesGrass;

	int pickBiomeIndex(float randomNumber);

	bool loadAllBiomes();

	Biome *determineBiome(float t, float h);
	int determineBiomeIndex(float t, int h);

	struct BiomeRange
	{
		std::vector<int> ids;
		float tresshold;
	};

	enum
	{

		desert,
		plains,
		snow,
		wasteLand,
		hayLand,

	};

};

