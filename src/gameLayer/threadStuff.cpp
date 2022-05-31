#include "threadStuff.h"
#include <thread>
#include <mutex>
#include <queue>
#include "worldGenerator.h"
#include <thread>
#include <unordered_map>
#include <iostream>

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

std::vector<Task> tryForTasks()
{
	std::unique_lock<std::mutex> locker(taskMutex);
	if (tasks.empty())
	{
		locker.unlock();
		return {};
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
std::queue<Message> chunks;

void submitMessage(Message m)
{
	chunkMutex.lock();

	chunks.push(m);

	chunkMutex.unlock();
}

std::vector<Message> getMessages()
{
	std::vector<Message> retVector;

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

