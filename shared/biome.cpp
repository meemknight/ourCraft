#include "biome.h"
#include <utility>


int BiomesManager::pickBiomeIndex(float randomNumber)
{
	int pick = std::min(randomNumber, 0.99f) * biomeIndexes.size();
	return biomeIndexes[pick];
}

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


	//desert 0
	{
		Biome b;
		b.name = "desert";
		b.color = glm::vec3{212, 189, 99} / 255.f;
		b.surfaceBlock = BlockTypes::sand;
		b.secondaryBlock = BlockTypes::sand_stone;

		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand_stone, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::hardSandStone, BlockTypes::hardSandStone});
		b.blockVariations.push_back({BlockTypes::hardSandStone, BlockTypes::hardSandStone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::hardSandStone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::hardSandStone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::hardSandStone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::stone, BlockTypes::stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		b.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});

		b.grassType = BlockTypes::dead_bush;
		b.waterType = BlockTypes::water;
		b.waterTypeSecond = BlockTypes::water;
		

		b.swampBlock.block.push_back(BlockTypes::sand);
		b.swampBlock.block.push_back(BlockTypes::stone);
		b.swampBlock.block.push_back(BlockTypes::stone);
		b.swampBlock.block.push_back(BlockTypes::stone);


		biomes.push_back(b);
	}

	//plains 1
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


		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::sand, BlockTypes::sand_stone});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::stone, BlockTypes::stone});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});
		plains.blockVariations.push_back({BlockTypes::grassBlock, BlockTypes::dirt});


		plains.grassType = BlockTypes::grass;
		plains.waterType = BlockTypes::water;
		plains.waterTypeSecond = BlockTypes::water;

		plains.swampBlock.block.push_back(BlockTypes::dirt);
		plains.swampBlock.block.push_back(BlockTypes::dirt);
		plains.swampBlock.block.push_back(BlockTypes::dirt);
		plains.swampBlock.block.push_back(BlockTypes::dirt);
		plains.swampBlock.block.push_back(BlockTypes::clay);

		biomes.push_back(plains);
	}
	
	//snow 2
	{
		Biome b;
		b.name = "snow";
		b.color = glm::vec3{199, 199, 199} / 255.f;
		b.surfaceBlock = BlockTypes::snow_dirt;
		b.secondaryBlock = BlockTypes::dirt;

		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::stone, BlockTypes::stone});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});
		b.blockVariations.push_back({BlockTypes::snow_dirt, BlockTypes::dirt});

		//b.treeType = Biome::treeSpruce;
		//b.forestTresshold = 0.9;
		//b.jusGrassTresshold = 0.3;
		//b.treeChanceRemap = {0.01, 0.06};
		//b.grassChanceForestRemap = {0.4, 0.6};
		//b.justGrassChanceRemap = {0.12, 0.4};
		//b.growTreesOn = BlockTypes::snow_dirt;
		//b.growGrassOn = BlockTypes::snow_dirt;

		b.swampBlock.block.push_back(BlockTypes::dirt);
		b.swampBlock.block.push_back(BlockTypes::dirt);
		b.swampBlock.block.push_back(BlockTypes::stone);
		b.swampBlock.block.push_back(BlockTypes::stone);
		b.swampBlock.block.push_back(BlockTypes::stone);
		b.swampBlock.block.push_back(BlockTypes::cobblestone);

		b.isICy = true;
		b.grassType = 0;
		b.waterType = BlockTypes::ice;
		b.waterTypeSecond = BlockTypes::water;

		biomes.push_back(b);
	}

	//wasteLand 1
	{
		Biome b;
		b.name = "waste land";
		b.color = {0.3,0.2,0.2};
		b.surfaceBlock = BlockTypes::volcanicRock;
		b.secondaryBlock = BlockTypes::volcanicRock;

		b.blockVariations.push_back({BlockTypes::volcanicRock, BlockTypes::volcanicRock});
		b.blockVariations.push_back({BlockTypes::volcanicRock, BlockTypes::volcanicRock});
		b.blockVariations.push_back({BlockTypes::volcanicRock, BlockTypes::volcanicRock});

		//b.grassType = BlockTypes::grass;
		b.waterType = BlockTypes::water;


		b.swampBlock.block.push_back(BlockTypes::volcanicRock);
		b.swampBlock.block.push_back(BlockTypes::volcanicRock);
		b.swampBlock.block.push_back(BlockTypes::volcanicRock);
		b.swampBlock.block.push_back(BlockTypes::volcanicRock);
		b.swampBlock.block.push_back(BlockTypes::volcanicRock);
		b.swampBlock.block.push_back(BlockTypes::volcanicHotRock);


		biomes.push_back(b);
	}


	biomeIndexes.push_back(BiomesManager::plains);
	biomeIndexes.push_back(BiomesManager::plains);
	biomeIndexes.push_back(BiomesManager::plains);
	biomeIndexes.push_back(BiomesManager::plains);
	biomeIndexes.push_back(BiomesManager::desert);
	biomeIndexes.push_back(BiomesManager::plains);
	biomeIndexes.push_back(BiomesManager::snow);
	biomeIndexes.push_back(BiomesManager::plains);
	biomeIndexes.push_back(BiomesManager::wasteLand);
	biomeIndexes.push_back(BiomesManager::plains);





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
