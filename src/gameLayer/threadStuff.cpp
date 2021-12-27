#include "threadStuff.h"
#include <thread>
#include <mutex>
#include <queue>
#include "worldGenerator.h"
#include <thread>
#include <unordered_map>

std::mutex taskMutex;
std::condition_variable taskCondition;
std::queue<Task> tasks;

void submitTask(Task& t)
{

	std::unique_lock<std::mutex> locker(taskMutex);
	tasks.push(t);
	locker.unlock();
	taskCondition.notify_one();

}

void submitTask(std::vector<Task>& t)
{
	std::unique_lock<std::mutex> locker(taskMutex);
	for (auto& i : t)
	{
		tasks.push(i);
	}
	locker.unlock();
	taskCondition.notify_one();
}

std::vector<Task> waitForTasks()
{
	std::unique_lock<std::mutex> locker(taskMutex);
	if (tasks.empty())
	{
		taskCondition.wait(locker);
	}

	auto size = tasks.size();
	std::vector<Task> retVector;
	retVector.reserve(size);

	for (int i = 0; i < size; i++)
	{
		retVector.push_back(tasks.front());
		tasks.pop();
	}

	locker.unlock();

	return retVector;
}

std::mutex chunkMutex;
std::queue<Chunk*> chunks;

void submitChunk(Chunk* c)
{
	chunkMutex.lock();

	chunks.push(c);

	chunkMutex.unlock();
}

std::vector<Chunk*> getChunks()
{
	std::vector<Chunk*> retVector;

	chunkMutex.lock();

	auto size = chunks.size();
	for (int i = 0; i < size; i++)
	{
		retVector.push_back(chunks.front());
		chunks.pop();
	}

	chunkMutex.unlock();

	return retVector;
}


void chunkCreatorFunction()
{
	while (true)
	{
		auto tasks = waitForTasks();

		for (auto& i : tasks)
		{

			if (i.type == Task::generateChunk)
			{
				Chunk* c = new Chunk;
				c->x = i.pos.x;
				c->z = i.pos.z;

				generateChunk(1234, *c);
				//std::this_thread::sleep_for(std::chrono::milliseconds(1));
				submitChunk(c);

			}

		}
	}
}


void serverFunction()
{

	chunkCreatorFunction();

	//std::thread chunkCreator1(chunkCreatorFunction);
	//chunkCreator1.detach();
	//std::thread chunkCreator2(chunkCreatorFunction);
	//chunkCreator2.detach();
	//std::thread chunkCreator3(chunkCreatorFunction);
	//chunkCreator3.detach();

	//while (true)
	//{
	//	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//}


}
