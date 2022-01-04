#pragma once
#include <glm/vec3.hpp>
#include "chunkSystem.h"

struct Task
{
	enum Type
	{
		none = 0,
		generateChunk,
		placeBlock
	};

	glm::ivec3 pos = {};
	int type = 0;
	int blockType = 0;

};

struct Message
{
	enum Type
	{
		none = 0,
		recievedChunk,
		placeBlock,
	};
	
	int type = 0;
	Chunk *chunk = nullptr;
	int blockType;
	glm::ivec3 pos;
};


void submitTask(Task& t);
void submitTask(std::vector<Task> &t);
std::vector<Task> waitForTasks();
std::vector<Task> tryForTasks();

void submitMessage(Message m);
std::vector<Message> getMessages();


void serverFunction();
