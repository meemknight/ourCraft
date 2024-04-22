#pragma once
#include <glm/vec3.hpp>
#include <deque>

bool constexpr dontUpdateLightSystem = 1;

struct ChunkSystem;
struct Chunk;

struct LightSystem
{
	struct Light
	{
		glm::ivec3 pos = {};
		char intensity = 0; //0..15
	};

	std::deque<Light> sunLigtsToAdd;
	std::deque<Light> sunLigtsToRemove;

	std::deque<Light> ligtsToAdd;
	std::deque<Light> ligtsToRemove;

	void update(ChunkSystem &chunkSystem);
	
	void addSunLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char intensity);
	void addSunLightAndPropagateDown(ChunkSystem &chunkSystem, glm::ivec3 pos, char intensity);
	void removeSunLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char oldVal);

	void addLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char intensity);
	void removeLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char oldVal);

	void setSunlightForAnEntireChunk(Chunk &chunk, ChunkSystem &chunkSystem);
};