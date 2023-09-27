#pragma once
#include "blocks.h"
#include <glm/vec2.hpp>
#include <FastNoiseSIMD.h>
#include <worldGeneratorSettings.h>



void generateChunk(int seed, Chunk &c, WorldGenerator &wg);
void generateChunk(int seed, ChunkData &c, WorldGenerator &wg);

