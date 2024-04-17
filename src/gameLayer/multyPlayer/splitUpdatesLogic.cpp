#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <multyPlayer/splitUpdatesLogic.h>
#include <multyPlayer/tick.h>
#include <queue>
#include <platform/platformTools.h>
#include <profilerLib.h>
#include <iostream>

struct PerTickData
{
	ServerChunkStorer chunkCache;
	EntityData orphanEntities;
};

static ThreadPool threadPool;
static std::vector<PerTickData> chunkRegionsData;
static std::deque<std::atomic<bool>> taskTaken;
static float tickDeltaTime = 0;
static std::uint64_t currentTimer = 0;

void closeThreadPool()
{
	for (int i = 0; i < threadPool.currentCounter; i++)
	{
		threadPool.threIsWork[i] = 0;
	}
	threadPool.setThreadsNumber(0);
}

int tryTakeTask()
{

	for (int i = 0; i < taskTaken.size(); i++)
	{
		if (!taskTaken[i].exchange(true)) // If the flag was not set
		{
			return i;
		}
	}
	return -1;
}



void splitUpdatesLogic(float tickDeltaTime, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache)
{
	::tickDeltaTime = tickDeltaTime;
	::currentTimer = currentTimer;

	PL::Profiler pl;

	pl.start();

	chunkRegionsData.clear();
	chunkRegionsData.reserve(10);

#pragma region start bfs

	std::unordered_map<glm::ivec2, int> visited;

	for (auto i : chunkCache.savedChunks)
	{
		if (!i.second->otherData.shouldUnload)
		{

			auto find = visited.find(i.first);

			if (find == visited.end())
			{
				chunkRegionsData.push_back({});
				
				int currentIndex = chunkRegionsData.size() - 1;

				visited.insert({i.first, currentIndex});

				chunkRegionsData[currentIndex].chunkCache.
					savedChunks.insert(i);

				std::deque<glm::ivec2> toLook;
				toLook.push_back({i.first + glm::ivec2(1,0)});
				toLook.push_back({i.first + glm::ivec2(0,1)});
				toLook.push_back({i.first + glm::ivec2(-1,0)});
				toLook.push_back({i.first + glm::ivec2(0,-1)});
				toLook.push_back({i.first + glm::ivec2(1,1)});
				toLook.push_back({i.first + glm::ivec2(-1,1)});
				toLook.push_back({i.first + glm::ivec2(1,-1)});
				toLook.push_back({i.first + glm::ivec2(-1,-1)});

				//start BFS
				while(!toLook.empty())
				{
					auto el = toLook.back();
					toLook.pop_back();

					auto found = visited.find(el);

					if (found != visited.end())
					{
						permaAssertComment(found->second == currentIndex, "error in the split chunks algorithm");
					}
					else
					{
						auto foundChunk = chunkCache.savedChunks.find(el);
						if (foundChunk != chunkCache.savedChunks.end())
						{
							if(!foundChunk->second->otherData.shouldUnload)
							{
								visited.insert({el, currentIndex});

								chunkRegionsData[currentIndex].chunkCache.
									savedChunks.insert(*foundChunk);

								toLook.push_back({el + glm::ivec2(1,0)});
								toLook.push_back({el + glm::ivec2(0,1)});
								toLook.push_back({el + glm::ivec2(-1,0)});
								toLook.push_back({el + glm::ivec2(0,-1)});
								toLook.push_back({el + glm::ivec2(1,1)});
								toLook.push_back({el + glm::ivec2(-1,1)});
								toLook.push_back({el + glm::ivec2(1,-1)});
								toLook.push_back({el + glm::ivec2(-1,-1)});
							}

						}

					}


				}

			}

		}


	}

#pragma endregion

	auto rez = pl.end();

	//std::cout << chunkRegionsData.size() << std::fixed 
	//	<< " Duration ms: " << (rez.timeSeconds/1000.f) << '\n';

	taskTaken.resize(chunkRegionsData.size());
	for (auto &i : taskTaken) { i = 0; }

	//todo better way of asigning a number of threads over time
	threadPool.setThreadsNumber(chunkRegionsData.size()-1);

	//start race
	threadPool.setThrerIsWork();

	while (true)
	{
		int taskIndex = tryTakeTask();
		if (taskIndex < 0) { break; }
		
		std::cout << "main took task " << taskIndex << '\n';

		//tick
		doGameTick(tickDeltaTime, currentTimer,
			chunkRegionsData[taskIndex].chunkCache,
			chunkRegionsData[taskIndex].orphanEntities);
	}


	threadPool.waitForEveryoneToFinish();

	//todo save orphan entities to disk...


}


void workerThread(int index)
{
	std::cout << "Weaked up thread " << index << "\n";
		 
	while (threadPool.running[index])
	{
		//wait for work...
		if (threadPool.threIsWork[index])
		{
			while (true)
			{
				int taskIndex = tryTakeTask();
				if (taskIndex < 0) { break; }
				
				std::cout << index << " took task " << taskIndex << '\n';

				//tick
				doGameTick(tickDeltaTime, currentTimer,
					chunkRegionsData[taskIndex].chunkCache,
					chunkRegionsData[taskIndex].orphanEntities);
			}

			//done work
			threadPool.threIsWork[index] = false;
		}

		
	}

	std::cout << "Closed thread " << index << "\n";
}

void ThreadPool::setThreadsNumber(int nr)
{
	if (nr < 0) { nr = 0; }
	if (nr > MAX_THREADS)
	{
		nr = MAX_THREADS;
	}

	if (nr != currentCounter)
	{

		if (nr > currentCounter)
		{
			//wake threads
			for (int i = currentCounter; i < nr; i++)
			{
				running[i] = 1;
			}

			for (int i = currentCounter; i < nr; i++)
			{
				threads[i] = std::move(std::thread(workerThread, i));
			}
		}
		else
		{
			for (int i = nr; i < currentCounter; i++)
			{
				running[i] = 0;
			}

			for (int i = nr; i < currentCounter; i++)
			{
				threads[i].join();
			}
		}

		currentCounter = nr;

	}

}

void ThreadPool::setThrerIsWork()
{
	for (int i = 0; i < currentCounter; i++)
	{
		threIsWork[i] = 1;
	}
}

void ThreadPool::waitForEveryoneToFinish()
{
	for (int i = 0; i < currentCounter; i++)
	{
		while (threIsWork[i] != 0) {};
	}
}

