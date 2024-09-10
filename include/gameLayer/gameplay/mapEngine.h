#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>
#include <gl2d/gl2d.h>
#include <unordered_map>

struct ProgramData;
struct ChunkSystem;

struct MapEngine
{
	
	void close();
	
	void update(ProgramData &programData, float deltaTime, glm::ivec2 chunkPos,
		ChunkSystem &chunkSystem);
	
	void open(ProgramData &programData, glm::ivec2 chunkPos, ChunkSystem &chunkSystem);

	glm::vec2 centerPos = {};
	gl2d::Camera camera;

	struct MapChunk
	{
		gl2d::Texture t;

		void cleanup();
	};

	std::unordered_map<glm::ivec2, MapChunk> mapChunks;

	glm::ivec2 mouseHovered = {};
	float mouseHoveredId = {-1};


};