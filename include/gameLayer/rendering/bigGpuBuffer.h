#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glad/glad.h>
#include <list>

struct BigGpuBuffer
{
	
	void create(size_t chunks);

	GLuint opaqueGeometryBuffer = 0;
	GLuint opaqueGeometryIndex = 0;
	GLuint vao = 0;

	size_t arenaSize = 0;

	void cleanup();

	struct GpuEntry
	{
		size_t beg = 0;
		size_t size = 0;
	};

	std::list<GpuEntry> entriesList;
	std::unordered_map<glm::ivec2, std::list<GpuEntry>::iterator> entriesMap;

	void addChunk(glm::ivec2 chunkPos, std::vector<int> &data);

	void removeChunk(glm::ivec2 chunkPos);

	void writeData(std::vector<int> &data, size_t pos);

	GpuEntry getEntry(glm::ivec2 chunkPos);
};

void setupVertexAttributes();
