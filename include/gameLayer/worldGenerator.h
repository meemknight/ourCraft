#pragma once
#include "blocks.h"
#include <glm/vec2.hpp>
#include <FastNoiseSIMD.h>
#include <worldGeneratorSettings.h>
#include <structure.h>
#include <vector>
#include <chunk.h>
#include <biome.h>

enum
{
	Structure_None = 0,
	Structure_Tree,
	Structure_JungleTree,
	Structure_PalmTree,
	Structure_TreeHouse,
	Structure_Pyramid,
	Structure_Igloo,
	Structure_BirchTree,
	Structure_Spruce,
	Structure_SpruceSlim,
	Structure_SmallStone,
	Structure_TallSlimTree,
};

//used to report back
struct StructureToGenerate
{
	glm::ivec3 pos = {};
	int type = 0;
	float randomNumber1 = 0;
	float randomNumber2 = 0;
	float randomNumber3 = 0;
	float randomNumber4 = 0;
	bool replaceBlocks = 0;
	bool addRandomTreeHeight = 0;
	BlockType replaceLeavesWith = 0;
	BlockType replaceLogWith = 0;
};

void generateChunk(Chunk &c, WorldGenerator &wg, StructuresManager &structuresManager, BiomesManager &biomesManager
	,std::vector<StructureToGenerate> &generateStructures);

void generateChunk(ChunkData &c, WorldGenerator &wg, StructuresManager &structuresManager, BiomesManager &biomesManager
	,std::vector<StructureToGenerate> &generateStructures);

