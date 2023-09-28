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
	int type = 0;
	glm::ivec3 pos = {};
	bool replaceBlocks = 0;
};

void generateChunk(Chunk &c, WorldGenerator &wg, StructuresManager &structuresManager, std::vector<StructureToGenerate> &generateStructures);
void generateChunk(ChunkData &c, WorldGenerator &wg, StructuresManager &structuresManager, std::vector<StructureToGenerate> &generateStructures);

