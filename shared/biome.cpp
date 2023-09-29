#include "biome.h"


bool BiomesManager::loadAllBiomes()
{
	*this = {};

	//forest 0
	{
		Biome plains;
		plains.name = "forest";
		plains.color = {0.2,0.7,0.3};
		plains.surfaceBlock = BlockTypes::grassBlock;
		plains.secondaryBlock = BlockTypes::dirt;

		plains.forestTresshold = 0.55;
		plains.jusGrassTresshold = 0.3;

		plains.treeChanceRemap = {0.01, 0.1};
		plains.grassChanceForestRemap = {0.2, 0.22};
		plains.justGrassChanceRemap = {0.12, 0.2};

		plains.growTreesOn = BlockTypes::grassBlock;
		plains.growGrassOn = BlockTypes::grassBlock;

		plains.grassType = BlockTypes::grass;
		plains.waterType = BlockTypes::water;

		biomes.push_back(plains);
	}

	//dry desert 1
	{
		Biome b;
		b.name = "dryDesert";
		b.color = glm::vec3{202, 169, 90}/255.f;
		b.surfaceBlock = BlockTypes::sand;
		b.secondaryBlock = BlockTypes::sand_stone;

		b.forestTresshold = 0.55;
		b.jusGrassTresshold = 0.3;

		b.treeChanceRemap = {0.01, 0.1};
		b.grassChanceForestRemap = {0.2, 0.22};
		b.justGrassChanceRemap = {0.12, 0.2};

		b.growTreesOn = 0;
		b.growGrassOn = 0;

		b.grassType = 0; //todo add dead bush here
		b.waterType = BlockTypes::sand;

		biomes.push_back(b);
	}

	//desert 2
	{
		Biome b;
		b.name = "desert";
		b.color = glm::vec3{212, 189, 99} / 255.f;
		b.surfaceBlock = BlockTypes::sand;
		b.secondaryBlock = BlockTypes::sand_stone;

		b.forestTresshold = 0.55;
		b.jusGrassTresshold = 0.3;

		b.treeChanceRemap = {0.01, 0.1};
		b.grassChanceForestRemap = {0.2, 0.22};
		b.justGrassChanceRemap = {0.12, 0.2};

		b.growTreesOn = 0;
		b.growGrassOn = 0;

		b.grassType = BlockTypes::grass; //todo add dead bush here
		b.waterType = BlockTypes::water;

		biomes.push_back(b);
	}

	//plains 3
	{
		Biome plains;
		plains.name = "plains";
		plains.color = {0.3,0.9,0.35};
		plains.surfaceBlock = BlockTypes::grassBlock;
		plains.secondaryBlock = BlockTypes::dirt;

		plains.forestTresshold = 0.75;
		plains.jusGrassTresshold = 0.2;

		plains.treeChanceRemap = {0.01, 0.05};
		plains.grassChanceForestRemap = {0.2, 0.22};
		plains.justGrassChanceRemap = {0.12, 0.2};

		plains.growTreesOn = BlockTypes::grassBlock;
		plains.growGrassOn = BlockTypes::grassBlock;

		plains.grassType = BlockTypes::grass;
		plains.waterType = BlockTypes::water;

		biomes.push_back(plains);
	}

	//oasis 4
	{
		Biome b;
		b.name = "oasis";
		b.color = glm::vec3{222, 102, 192} / 255.f;
		b.surfaceBlock = BlockTypes::grassBlock;
		b.secondaryBlock = BlockTypes::dirt;

		b.forestTresshold = 0.5;
		b.jusGrassTresshold = 0.2;

		b.treeChanceRemap = {0.01, 0.05};
		b.grassChanceForestRemap = {0.2, 0.22};
		b.justGrassChanceRemap = {0.12, 0.2};

		b.growTreesOn = BlockTypes::grassBlock;
		b.growGrassOn = BlockTypes::grassBlock;

		b.grassType = BlockTypes::grass;
		b.waterType = BlockTypes::water;

		biomes.push_back(b);
	}

	//jungle 5
	{
		Biome b;
		b.name = "jungle";
		b.color = {0.2,0.7,0.3};
		b.surfaceBlock = BlockTypes::grassBlock;
		b.secondaryBlock = BlockTypes::dirt;

		b.forestTresshold = 0.3;
		b.jusGrassTresshold = 0.1;

		b.treeChanceRemap = {0.01, 0.1};
		b.grassChanceForestRemap = {0.2, 0.22};
		b.justGrassChanceRemap = {0.12, 0.2};

		b.growTreesOn = BlockTypes::grassBlock;
		b.growGrassOn = BlockTypes::grassBlock;

		b.grassType = BlockTypes::grass;
		b.waterType = BlockTypes::water;

		biomes.push_back(b);
	}

	//dryLand 6
	{
		Biome b;
		b.name = "dryLand";
		b.color = glm::vec3{84, 51, 23} / 255.f;
		b.surfaceBlock = BlockTypes::dirt;
		b.secondaryBlock = BlockTypes::dirt;

		b.forestTresshold = 0.3;
		b.jusGrassTresshold = 0.1;

		b.treeChanceRemap = {0.01, 0.1};
		b.grassChanceForestRemap = {0.2, 0.22};
		b.justGrassChanceRemap = {0.12, 0.2};

		b.growTreesOn = 0;
		b.growGrassOn = 0;

		b.grassType = 0;
		b.waterType = BlockTypes::water;

		biomes.push_back(b);
	}

	//rocks 7
	{
		Biome b;
		b.name = "rocks";
		b.color = glm::vec3{89, 89, 89} / 255.f;
		b.surfaceBlock = BlockTypes::stone;
		b.secondaryBlock = BlockTypes::stone;

		b.forestTresshold = 0.3;
		b.jusGrassTresshold = 0.1;

		b.treeChanceRemap = {0.01, 0.1};
		b.grassChanceForestRemap = {0.2, 0.22};
		b.justGrassChanceRemap = {0.12, 0.2};

		b.growTreesOn = 0;
		b.growGrassOn = 0;

		b.grassType = 0;
		b.waterType = BlockTypes::ice;

		biomes.push_back(b);
	}

	//snow 8
	{
		Biome b;
		b.name = "snow";
		b.color = glm::vec3{199, 199, 199} / 255.f;
		b.surfaceBlock = BlockTypes::snow_dirt;
		b.secondaryBlock = BlockTypes::dirt;

		b.forestTresshold = 0.9;
		b.jusGrassTresshold = 0.3;

		b.treeChanceRemap = {0.02, 0.12};
		b.grassChanceForestRemap = {0.4, 0.6};
		b.justGrassChanceRemap = {0.12, 0.4};

		b.growTreesOn = 0;
		b.growGrassOn = BlockTypes::snow_dirt;

		b.grassType = 0;
		b.waterType = BlockTypes::ice;

		biomes.push_back(b);
	}

	//taiga 9
	{
		Biome b;
		b.name = "taiga";
		b.color = glm::vec3{184, 201, 159} / 255.f;
		b.surfaceBlock = BlockTypes::snow_dirt;
		b.secondaryBlock = BlockTypes::dirt;

		b.forestTresshold = 0.7;
		b.jusGrassTresshold = 0.3;

		b.treeChanceRemap = {0.02, 0.12};
		b.grassChanceForestRemap = {0.4, 0.6};
		b.justGrassChanceRemap = {0.12, 0.4};

		b.growTreesOn = BlockTypes::snow_dirt;
		b.growGrassOn = BlockTypes::snow_dirt;

		b.grassType = 0;
		b.waterType = BlockTypes::ice;

		biomes.push_back(b);
	}


	{
		BiomeRange cold;
		cold.tresshold = 0;
		cold.ids.push_back(7);
		cold.ids.push_back(8);
		cold.ids.push_back(9);
		picker.temperature.push_back(cold);

		BiomeRange medium;
		medium.tresshold = 0.29;
		medium.ids.push_back(6);
		medium.ids.push_back(3);
		medium.ids.push_back(0);
		picker.temperature.push_back(medium);

		BiomeRange warm;
		warm.tresshold = 0.59;
		warm.ids.push_back(1);
		warm.ids.push_back(2);
		warm.ids.push_back(5);
		picker.temperature.push_back(warm);

		BiomeRange veryWarm;
		warm.tresshold = 0.9;
		warm.ids.push_back(1);
		warm.ids.push_back(2);
		warm.ids.push_back(4);
		warm.ids.push_back(5);
		picker.temperature.push_back(warm);
	}

	{
		BiomeRange dry;
		dry.tresshold = 0;
		dry.ids.push_back(7);
		dry.ids.push_back(6);
		dry.ids.push_back(1);
		picker.humidity.push_back(dry);

		BiomeRange medium;
		medium.tresshold = 0.29;
		medium.ids.push_back(8);
		medium.ids.push_back(3);
		medium.ids.push_back(2);
		picker.humidity.push_back(medium);

		BiomeRange mediumOasis;
		mediumOasis.tresshold = 0.51;
		mediumOasis.ids.push_back(8);
		mediumOasis.ids.push_back(3);
		mediumOasis.ids.push_back(2);
		mediumOasis.ids.push_back(4);
		picker.humidity.push_back(mediumOasis);

		BiomeRange high;
		high.tresshold = 0.61;
		high.ids.push_back(5);
		high.ids.push_back(0);
		high.ids.push_back(9);
		picker.humidity.push_back(high);
	}


	return true;
}

Biome *BiomesManager::determineBiome(float t, float h)
{

	std::vector<int> biomes;
	biomes.reserve(5);

	size_t size = picker.temperature.size();
	for (int i = size - 1; i >= 0; i--)
	{
		if (t > picker.temperature[i].tresshold)
		{
			for (auto b : picker.temperature[i].ids)
			{
				biomes.push_back(b);
			}
			break;
		}
	}

	size = picker.humidity.size();
	for (int i = size - 1; i >= 0; i--)
	{
		if (h > picker.humidity[i].tresshold)
		{
			for (auto b : picker.humidity[i].ids)
			{
				for (auto id : biomes)
				{
					if (id == b)
					{
						return &this->biomes[id];
					}
				}
			}
		}
	}

	_CrtDbgBreak();

	return nullptr;
}
