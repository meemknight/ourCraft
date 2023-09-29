#pragma once
#include "blocks.h"
#include <glm/vec2.hpp>
#include <FastNoiseSIMD.h>
#include <worldGeneratorSettings.h>
#include <structure.h>
#include <vector>

enum
{
	Structure_None = 0,
	Structure_Tree,
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

};

void generateChunk(Chunk &c, WorldGenerator &wg, StructuresManager &structuresManager, std::vector<StructureToGenerate> &generateStructures);
void generateChunk(ChunkData &c, WorldGenerator &wg, StructuresManager &structuresManager, std::vector<StructureToGenerate> &generateStructures);

