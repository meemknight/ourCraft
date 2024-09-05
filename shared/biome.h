#pragma once
#include <staticVector.h>
#include <blocks.h>
#include <glm/glm.hpp>
#include <vector>


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
	glm::vec2 chanceRemap = {0, 1};

	GrowingThing growThing;
};


//this are the settings for one vegetation noise...
//there can be multiple things stacked here
struct VegetationNoiseSettings
{

	StaticVector<VegetationSettings, 4> entry;


};


struct Biome
{

	enum
	{
		treeNone = 0,
		treeNormal,
		treeNormalTall,
		treeJungle,
		treePalm,
		treeBirch,
		treeSpruce,
	};

	const char *name = "";
	glm::vec3 color = {};

	BlockType surfaceBlock;
	BlockType secondaryBlock; //todo add height variation here


	StaticVector<VegetationNoiseSettings, 2> vegetationNoises;


	BlockType grassType;
	BlockType waterType;

};

struct BiomesManager
{

	std::vector<Biome> biomes;

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
		forest,
		desert,
		plains,
		oasis,
		jungls,
		dryLand,
		rocks,
		snow,
		taiga,

	};

};

