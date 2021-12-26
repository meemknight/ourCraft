#pragma once
#include <glm/vec3.hpp>
#include "chunkSystem.h"

struct Task
{
	enum Type
	{
		none = 0,
		generateChunk,
	};
	
	int type = 0;
	glm::ivec3 pos = {};


};


void submitTask(Task &t);
std::vector<Task> waitForTasks();

void submitChunk(Chunk *c);
std::vector<Chunk*> getChunks();


void serverFunction();
