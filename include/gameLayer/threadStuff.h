#pragma once
#include <glm/vec3.hpp>
#include "chunkSystem.h"

struct Task
{
	enum Type
	{
		none = 0,
		placeBlock,
		generateChunk,
	};

	glm::ivec3 pos = {};
	int type = 0;
	uint16_t blockType = 0;

};

//struct Message
//{
//	enum Type
//	{
//		none = 0,
//		recievedChunk,
//		placeBlock,
//	};
//	
//	int type = 0;
//	Chunk *chunk = nullptr;
//	int blockType;
//	glm::ivec3 pos;
//};

//todo move out of thread stuff
void submitTaskClient(Task& t);
void submitTaskClient(std::vector<Task> &t);


void serverFunction();
