#pragma once
#include <blocks.h>
#include <glm/glm.hpp>
#include <vector>

struct Biome
{

	enum
	{
		treeNone = 0,
		treeNormal,
		treeJungle,
		treePalm,
		treeBirch,
	};

	const char *name = "";
	glm::vec3 color = {};

	BlockType surfaceBlock;
	BlockType secondaryBlock; //todo add height variation here

	int treeType = 0;

	float forestTresshold = 0; 
	float jusGrassTresshold = 0;

	glm::vec2 treeChanceRemap = {};
	glm::vec2 grassChanceForestRemap = {};
	glm::vec2 justGrassChanceRemap = {};

	BlockType growTreesOn;
	BlockType growGrassOn;

	BlockType grassType;
	BlockType waterType;

};

struct BiomesManager
{

	std::vector<Biome> biomes;

	bool loadAllBiomes();

	Biome *determineBiome(float t, float h);
	int determineBiomeIndex(float t, float h);

	struct BiomeRange
	{
		std::vector<int> ids;
		float tresshold;
	};

	struct Picker
	{

		std::vector<BiomeRange> temperature;
		std::vector<BiomeRange> humidity;

	};

	Picker picker;
};

