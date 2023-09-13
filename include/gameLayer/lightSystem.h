#pragma once
#include <glm/vec3.hpp>
#include <deque>
struct ChunkSystem;

struct LightSystem
{
	struct Light
	{
		glm::ivec3 pos = {};
		char intensity = 0; //0..15
	};

	std::deque<Light> sunLigtsToAdd;
	std::deque<Light> sunLigtsToRemove;

	void update(ChunkSystem &chunkSystem);
	
	void addSunLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char intensity);

	void removeSunLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char oldVal);

};