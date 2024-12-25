#define GLM_ENABLE_EXPERIMENTAL
#include <multyPlayer/enetServerFunction.h>
#include <glm/gtx/hash.hpp>
#include <multyPlayer/splitUpdatesLogic.h>
#include <multyPlayer/tick.h>
#include <queue>
#include <platform/platformTools.h>
#include <profilerLib.h>
#include <iostream>

struct Client;

struct PerTickData
{
	ServerChunkStorer chunkCache;
	EntityData orphanEntities;
	unsigned int seed = 1;
};

static ThreadPool threadPool;
static std::vector<PerTickData> chunkRegionsData;
static std::deque<std::atomic<bool>> taskTaken;
static float tickDeltaTime = 0;
static int tickDeltaTimeMs = 0;
static std::uint64_t currentTimer = 0;

int regionsAverageCounter[50] = {};
int counterPosition = 0;

void closeThreadPool()
{
	for (int i = 0; i < threadPool.currentCounter; i++)
	{
		threadPool.threIsWork[i] = 0;
	}
	threadPool.setThreadsNumber(0);

	memset(regionsAverageCounter, 0, sizeof(regionsAverageCounter));
	counterPosition = 0;
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

int getThredPoolSize()
{
	return threadPool.currentCounter;
}

#define ENTITY_SET_IN_CHUNKS(X) resetEntitiesInTheirNewChunk(*c.orphanEntities.entityGetter<X>(), [](auto &entityData) { return entityData.entityGetter<X>(); });


void splitUpdatesLogic(float tickDeltaTime, int tickDeltaTimeMs, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache, unsigned int seed, std::unordered_map<std::uint64_t, Client> &clients,
	WorldSaver &worldSaver)
{



	if (1)
	{
		std::minstd_rand rng(seed);

		::tickDeltaTime = tickDeltaTime;
		::currentTimer = currentTimer;
		::tickDeltaTimeMs = tickDeltaTimeMs;

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
					while (!toLook.empty())
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
								if (!foundChunk->second->otherData.shouldUnload)
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


		taskTaken.resize(chunkRegionsData.size());
		for (auto &i : taskTaken) { i = 0; }

		for (auto &c : chunkRegionsData)
		{
			c.seed = rng();
		}

		//todo fix lol
		//if (chunkRegionsData.size() > 1)
		//{
		//	std::cout << chunkRegionsData.size() << "!!!\n";
		//}

	#pragma region set threads count
		{

			regionsAverageCounter[counterPosition] = chunkRegionsData.size();

			if (counterPosition == 49)
			{
				float average = 0;
				for (int i = 0; i < 50; i++)
				{
					average += regionsAverageCounter[i];
				}

				average /= 50;

				int averageInt = std::roundf(average);

				threadPool.setThreadsNumber(averageInt - 1);
				counterPosition = 0;
			}
			else
			{
				counterPosition++;
			}
		}
	#pragma endregion


		//start race
		threadPool.setThrerIsWork();

		auto rez = pl.end();
		//std::cout << threadPool.currentCounter << std::fixed
		//	<< " Duration ms: " << (rez.timeSeconds/1000.f) << '\n';

		while (true)
		{
			int taskIndex = tryTakeTask();
			if (taskIndex < 0) { break; }

			//std::cout << "main took task " << taskIndex << '\n';

			//tick
			doGameTick(tickDeltaTime, tickDeltaTimeMs, currentTimer,
				chunkRegionsData[taskIndex].chunkCache,
				chunkRegionsData[taskIndex].orphanEntities,
				chunkRegionsData[taskIndex].seed
				);
		}


		threadPool.waitForEveryoneToFinish();

		//todo save orphan entities to disk...

		auto resetEntitiesInTheirNewChunk = [&](auto &container, auto memberSelector)
		{

			for (auto it = container.begin();
				it != container.end();)
			{
				auto &e = *it;

				auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
				auto chunk = chunkCache.getChunkOrGetNull(pos.x, pos.y);

				if (chunk)
				{
					auto member = memberSelector(chunk->entityData);
					member->insert({e.first, e.second});
				}
				else
				{
					std::cout << "Saved entity!\n";
					//todo change and make in 2 steps
					worldSaver.appendEntitiesForChunk(pos);
					//todo save entity to disk here!.
				}

				it++;

			}
		};


		for (auto &c : chunkRegionsData)
		{
			//save this entities to disk or other chunks...

			//resetEntitiesInTheirNewChunk(*c.orphanEntities.entityGetter<1>(),
			//	[](auto &entityData) { return entityData.entityGetter<1>(); });

			REPEAT_FOR_ALL_ENTITIES_NO_PLAYERS(ENTITY_SET_IN_CHUNKS);
		}

		chunkRegionsData.clear();
	}
	else
	{
		//deprecated
		// 
		//ServerChunkStorer copy;
		//EntityData orphans;
		//
		//for (auto &i : chunkCache.savedChunks)
		//{
		//
		//	if (i.second && !i.second->otherData.shouldUnload)
		//	{
		//		copy.savedChunks.insert(i);
		//	}
		//
		//}
		//
		//doGameTick(tickDeltaTime, currentTimer,
		//	copy,
		//	orphans,
		//	seed);

	}


	//not needed anymore
	//for (auto &c : chunkCache.savedChunks)
	//{
	//	for (auto &p : c.second->entityData.players)
	//	{
	//		clients[p.first].playerData = p.second;
	//	}
	//}
	


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
				
				//std::cout << index << " took task " << taskIndex << '\n';

				//tick
				doGameTick(tickDeltaTime, tickDeltaTimeMs, currentTimer,
					chunkRegionsData[taskIndex].chunkCache,
					chunkRegionsData[taskIndex].orphanEntities,
					chunkRegionsData[taskIndex].seed);
			}

			//done work
			threadPool.threIsWork[index] = false;
		}

		
	}

	//std::cout << "Closed thread " << index << "\n";
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

