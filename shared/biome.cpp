#include "biome.h"

bool BiomesManager::loadAllBiomes()
{
	*this = {};


	GrowElement grassGrowElement;
	grassGrowElement.block = BlockTypes::grass;

	GrowElement roseGrowElement;
	roseGrowElement.block = BlockTypes::rose;

	GrowElement basictreeElement;
	basictreeElement.treeType = Biome::treeNormal;

	GrowElement tallForestTrees;
	tallForestTrees.treeType = Biome::treeNormalTall;

	GrowElement tallSlimSpruceOakCenterTrees;
	tallSlimSpruceOakCenterTrees.treeType = Biome::treeSpruceTallOakCenter;

	GrowElement normalSpruceTree;
	normalSpruceTree.treeType = Biome::treeSpruce;

	GrowElement birchTree;
	birchTree.treeType = Biome::treeBirch;

	GrowElement tallBirchTree;
	tallBirchTree.treeType = Biome::treeBirchTall;

	GrowElement redBirchTree;
	redBirchTree.treeType = Biome::treeRedBirch;

	GrowElement redBirchTreeTall;
	redBirchTreeTall.treeType = Biome::treeRedBirchTall;

	GrowElement tallOakElement;
	tallOakElement.treeType = Biome::treeTallOak;

	StaticVector<BlockTypes, 5> growOnNormalPlants = {BlockTypes::grassBlock, BlockTypes::snow_dirt};


	GrowingThing growGrass;
	growGrass.elements.push_back(grassGrowElement);
	growGrass.growOn = growOnNormalPlants;

	GrowingThing growTree;
	growTree.elements.push_back(basictreeElement);
	growTree.growOn = growOnNormalPlants;

	GrowingThing grawTallTrees;
	grawTallTrees.elements.push_back(tallForestTrees);
	grawTallTrees.elements.push_back(tallForestTrees);
	grawTallTrees.elements.push_back(basictreeElement);
	grawTallTrees.growOn = growOnNormalPlants;

	GrowingThing grawTallSpruces;
	grawTallSpruces.elements.push_back(tallSlimSpruceOakCenterTrees);
	grawTallSpruces.elements.push_back(tallSlimSpruceOakCenterTrees);
	grawTallSpruces.elements.push_back(tallSlimSpruceOakCenterTrees);
	grawTallSpruces.elements.push_back(tallSlimSpruceOakCenterTrees);
	grawTallSpruces.elements.push_back(tallSlimSpruceOakCenterTrees);
	grawTallSpruces.elements.push_back(tallSlimSpruceOakCenterTrees);
	grawTallSpruces.elements.push_back(tallSlimSpruceOakCenterTrees);
	grawTallSpruces.elements.push_back(tallSlimSpruceOakCenterTrees);
	grawTallSpruces.elements.push_back(normalSpruceTree);
	grawTallSpruces.growOn = growOnNormalPlants;


	GrowingThing growThickSpruce;
	growThickSpruce.elements.push_back(normalSpruceTree);
	growThickSpruce.growOn = growOnNormalPlants;


	GrowingThing growBirch;
	growBirch.elements.push_back(birchTree);
	growBirch.growOn = growOnNormalPlants;

	GrowingThing growTallOak;
	growTallOak.elements.push_back(tallOakElement);
	growTallOak.growOn = growOnNormalPlants;

	GrowingThing growRose;
	growRose.elements.push_back(roseGrowElement);
	growRose.elements.push_back(roseGrowElement);
	growRose.elements.push_back(grassGrowElement);
	growRose.elements.push_back(grassGrowElement);
	growRose.elements.push_back(grassGrowElement);
	growRose.growOn = growOnNormalPlants;

	GrowingThing growTreeOrBirch;
	growTreeOrBirch.elements.push_back(basictreeElement);
	growTreeOrBirch.elements.push_back(basictreeElement);
	growTreeOrBirch.elements.push_back(birchTree);
	growTreeOrBirch.growOn = growOnNormalPlants;

	GrowingThing growBirchForest;
	growBirchForest.elements.push_back(birchTree);
	growBirchForest.elements.push_back(birchTree);
	growBirchForest.elements.push_back(tallBirchTree);
	growBirchForest.growOn = growOnNormalPlants;

	GrowingThing growRedBirchForest;
	growRedBirchForest.elements.push_back(redBirchTree);
	growRedBirchForest.elements.push_back(redBirchTree);
	growRedBirchForest.elements.push_back(redBirchTreeTall);
	growRedBirchForest.growOn = growOnNormalPlants;

	VegetationSettings growFullGrassVegetation;
	growFullGrassVegetation.growThing = growGrass;
	growFullGrassVegetation.chanceRemap = {0.1, 0.5};

	VegetationSettings growNormalTree;
	growNormalTree.growThing = growTree;
	growNormalTree.minTresshold = 0.5;
	growNormalTree.chanceRemap = {0.01, 0.08};

	VegetationSettings growBirchVegetation;
	growBirchVegetation.growThing = growBirchForest;
	growBirchVegetation.minTresshold = 0.5;
	growBirchVegetation.chanceRemap = {0.012, 0.09};

	VegetationSettings growRedBirchVegetation;
	growRedBirchVegetation.growThing = growRedBirchForest;
	growRedBirchVegetation.minTresshold = 0.5;
	growRedBirchVegetation.chanceRemap = {0.015, 0.05};

	VegetationSettings growRandomTreesVegetation;
	growRandomTreesVegetation.growThing = growTreeOrBirch;
	growRandomTreesVegetation.minTresshold = 0.65;
	growRandomTreesVegetation.chanceRemap = {0.01, 0.04};

	VegetationSettings growRosePatchesVegetation;
	growRosePatchesVegetation.growThing = growRose;
	growRosePatchesVegetation.minTresshold = 0.9;
	growRosePatchesVegetation.chanceRemap = {0.1, 0.5};

	VegetationSettings tallSpruceForestVegetation;
	tallSpruceForestVegetation.growThing = grawTallSpruces;
	tallSpruceForestVegetation.minTresshold = 0.50;
	tallSpruceForestVegetation.chanceRemap = {0.01, 0.07};

	VegetationSettings ThickSpruce;
	ThickSpruce.growThing = growThickSpruce;
	ThickSpruce.minTresshold = 0.50;
	ThickSpruce.chanceRemap = {0.01, 0.04};
	
	VegetationSettings tallForestVegetation;
	tallForestVegetation.growThing = growTallOak;
	tallForestVegetation.minTresshold = 0.5;
	tallForestVegetation.chanceRemap = {0.03, 0.10};

	auto pushTreeVegetation = [&](VegetationSettings &v)
	{
		VegetationNoiseSettings noiseSettings;
		noiseSettings.entry.push_back(v);
		greenBiomesTrees.push_back(noiseSettings);
	};
	

	pushTreeVegetation(growNormalTree);
	pushTreeVegetation(growRandomTreesVegetation);
	pushTreeVegetation(growBirchVegetation);
	pushTreeVegetation(tallForestVegetation);
	pushTreeVegetation(tallSpruceForestVegetation);
	pushTreeVegetation(growRedBirchVegetation);
	pushTreeVegetation(ThickSpruce);
	

	{
		VegetationNoiseSettings noiseSettings;
		noiseSettings.entry.push_back(growFullGrassVegetation);
		greenBiomesGrass.push_back(noiseSettings);
	}


	//forest 0
	{
		Biome plains;
		plains.name = "forest";
		plains.color = {0.2,0.7,0.3};
		plains.surfaceBlock = BlockTypes::grassBlock;
		plains.secondaryBlock = BlockTypes::dirt;


		{
			VegetationNoiseSettings noiseSettings;
			noiseSettings.entry.push_back(growRandomTreesVegetation);
			plains.vegetationNoises.push_back(noiseSettings);
		};

		{
			VegetationNoiseSettings noiseSettings;
			noiseSettings.entry.push_back(ThickSpruce);
			plains.vegetationNoises.push_back(noiseSettings);
		};

		{
			VegetationNoiseSettings noiseSettings;
			noiseSettings.entry.push_back(tallForestVegetation);
			plains.vegetationNoises.push_back(noiseSettings);
		};

		{
			VegetationNoiseSettings noiseSettings;
			noiseSettings.entry.push_back(tallSpruceForestVegetation);
			noiseSettings.entry.push_back(growFullGrassVegetation);
			plains.vegetationNoises.push_back(noiseSettings);
		};

	

		plains.grassType = BlockTypes::grass;
		plains.waterType = BlockTypes::water;

		biomes.push_back(plains);
	}

	//desert 1
	{
		Biome b;
		b.name = "desert";
		b.color = glm::vec3{212, 189, 99} / 255.f;
		b.surfaceBlock = BlockTypes::sand;
		b.secondaryBlock = BlockTypes::sand_stone;

		//b.forestTresshold = 0.55;
		//b.jusGrassTresshold = 0.3;
		//b.treeChanceRemap = {0.01, 0.1};
		//b.grassChanceForestRemap = {0.01, 0.02};
		//b.justGrassChanceRemap = {0.001, 0.01};
		//b.growTreesOn = 0;
		//b.growGrassOn = BlockTypes::sand;

		b.grassType = BlockTypes::dead_bush;
		b.waterType = BlockTypes::water;

		biomes.push_back(b);
	}

	//plains 2
	{
		Biome plains;
		plains.name = "plains";
		plains.color = {0.3,0.9,0.35};
		plains.surfaceBlock = BlockTypes::grassBlock;
		plains.secondaryBlock = BlockTypes::dirt;

		VegetationNoiseSettings noiseSettings;
		noiseSettings.entry.push_back(growRandomTreesVegetation);
		noiseSettings.entry.push_back(growFullGrassVegetation);


		VegetationNoiseSettings noiseSettings2;
		noiseSettings2.entry.push_back(growRosePatchesVegetation);
		
		
		plains.vegetationNoises.push_back(noiseSettings);
		plains.vegetationNoises.push_back(noiseSettings2);



		//plains.treeType = Biome::treeNormal;
		////plains.forestTresshold = 0.96;
		//plains.forestTresshold = 1;
		//plains.jusGrassTresshold = 0.05;
		//plains.jusTreeTresshold = 0.6;
		//plains.treeChanceRemap = {0.01, 0.05};
		//plains.grassChanceForestRemap = {0.2, 0.22};
		//plains.justGrassChanceRemap = {0.12, 0.2};
		//plains.justTreeChanceRemap = {0.0, 0.001};
		//plains.growTreesOn = BlockTypes::grassBlock;
		//plains.growGrassOn = BlockTypes::grassBlock;

		plains.grassType = BlockTypes::grass;
		plains.waterType = BlockTypes::water;

		biomes.push_back(plains);
	}

	//oasis 3
	{
		Biome b;
		b.name = "oasis";
		b.color = glm::vec3{222, 102, 192} / 255.f;
		b.surfaceBlock = BlockTypes::grassBlock;
		b.secondaryBlock = BlockTypes::dirt;


		//b.treeType = Biome::treePalm;
		//b.forestTresshold = 0.3;
		//b.jusGrassTresshold = 0.1;
		//b.treeChanceRemap = {0.001, 0.01};
		//b.grassChanceForestRemap = {0.01, 0.03};
		//b.justGrassChanceRemap = {0.01, 0.02};
		//b.growTreesOn = BlockTypes::grassBlock;
		//b.growGrassOn = BlockTypes::grassBlock;


		b.grassType = BlockTypes::cactus_bud;
		b.waterType = BlockTypes::water;

		biomes.push_back(b);
	}

	//jungle 4
	{
		Biome b;
		b.name = "jungle";
		b.color = {0.2,0.7,0.3};
		b.surfaceBlock = BlockTypes::grassBlock;
		b.secondaryBlock = BlockTypes::dirt;


		//b.treeType = Biome::treeJungle;
		//b.forestTresshold = 0.3;
		//b.jusGrassTresshold = 0.1;
		//b.treeChanceRemap = {0.006, 0.07};
		//b.grassChanceForestRemap = {0.2, 0.22};
		//b.justGrassChanceRemap = {0.12, 0.2};
		//b.growTreesOn = BlockTypes::grassBlock;
		//b.growGrassOn = BlockTypes::grassBlock;


		b.grassType = BlockTypes::grass;
		b.waterType = BlockTypes::water;

		biomes.push_back(b);
	}

	//dryLand 5
	{
		Biome b;
		b.name = "dryLand";
		b.color = glm::vec3{84, 51, 23} / 255.f;
		b.surfaceBlock = BlockTypes::dirt;
		b.secondaryBlock = BlockTypes::dirt;

		//b.forestTresshold = 0.3;
		//b.jusGrassTresshold = 0.1;
		//b.treeChanceRemap = {0.01, 0.1};
		//b.grassChanceForestRemap = {0.2, 0.22};
		//b.justGrassChanceRemap = {0.12, 0.2};
		//b.growTreesOn = 0;
		//b.growGrassOn = 0;

		b.grassType = 0;
		b.waterType = BlockTypes::water;

		biomes.push_back(b);
	}

	//rocks 6
	{
		Biome b;
		b.name = "rocks";
		b.color = glm::vec3{89, 89, 89} / 255.f;
		b.surfaceBlock = BlockTypes::stone;
		b.secondaryBlock = BlockTypes::stone;

		//b.forestTresshold = 0.3;
		//b.jusGrassTresshold = 0.1;
		//b.treeChanceRemap = {0.01, 0.1};
		//b.grassChanceForestRemap = {0.2, 0.22};
		//b.justGrassChanceRemap = {0.12, 0.2};
		//b.growTreesOn = 0;
		//b.growGrassOn = 0;

		b.grassType = 0;
		b.waterType = BlockTypes::ice;

		biomes.push_back(b);
	}

	//snow 7
	{
		Biome b;
		b.name = "snow";
		b.color = glm::vec3{199, 199, 199} / 255.f;
		b.surfaceBlock = BlockTypes::snow_dirt;
		b.secondaryBlock = BlockTypes::dirt;

		//b.treeType = Biome::treeSpruce;
		//b.forestTresshold = 0.9;
		//b.jusGrassTresshold = 0.3;
		//b.treeChanceRemap = {0.01, 0.06};
		//b.grassChanceForestRemap = {0.4, 0.6};
		//b.justGrassChanceRemap = {0.12, 0.4};
		//b.growTreesOn = BlockTypes::snow_dirt;
		//b.growGrassOn = BlockTypes::snow_dirt;

		b.isICy = true;
		b.grassType = 0;
		b.waterType = BlockTypes::ice;

		biomes.push_back(b);
	}

	//taiga 8
	{
		Biome b;
		b.name = "taiga";
		b.color = glm::vec3{184, 201, 159} / 255.f;
		b.surfaceBlock = BlockTypes::snow_dirt;
		b.secondaryBlock = BlockTypes::dirt;

		//b.treeType = Biome::treeSpruce;
		//b.forestTresshold = 0.7;
		//b.jusGrassTresshold = 0.3;
		//b.treeChanceRemap = {0.02, 0.12};
		//b.grassChanceForestRemap = {0.4, 0.6};
		//b.justGrassChanceRemap = {0.12, 0.4};
		//b.growTreesOn = BlockTypes::snow_dirt;
		//b.growGrassOn = BlockTypes::snow_dirt;

		b.grassType = 0;
		b.waterType = BlockTypes::ice;

		biomes.push_back(b);
	}









	return true;
}

Biome *BiomesManager::determineBiome(float t, float h)
{
	return &this->biomes[determineBiomeIndex(t, h)];
}

int BiomesManager::determineBiomeIndex(float t, int h)
{
	









	return 0;
}
