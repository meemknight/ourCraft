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
	std::vector<ServerTask> waitingTasks;
	ServerChunkStorer chunkCache;
	EntityData orphanEntities;
	unsigned int seed = 1;
};

static ThreadPool threadPool;
static std::vector<PerTickData> chunkRegionsData;
static float tickDeltaTime = 0;
static int tickDeltaTimeMs = 0;
static std::uint64_t currentTimer = 0;
static WorldSaver *worldSaver = 0;
static Profiler gameTickProfiler;

Profiler getServerTickProfilerCopy()
{
	return gameTickProfiler;
}


int regionsAverageCounter[50] = {};
int counterPosition = 0;

void workerThread(int index, ThreadPool &threadPool);

void closeThreadPool()
{
	threadPool.cleanup();

	memset(regionsAverageCounter, 0, sizeof(regionsAverageCounter));
	counterPosition = 0;
}

int tryTakeTask()
{

	for (int i = 0; i < threadPool.taskTaken.size(); i++)
	{
		if (!threadPool.taskTaken[i].exchange(true)) // If the flag was not set
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
#define ITERATE_TEST(X) iterateTest(*chunk.second->entityData.entityGetter<X>());


void splitUpdatesLogic(float tickDeltaTime, int tickDeltaTimeMs, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache, unsigned int seed, std::unordered_map<std::uint64_t, Client> &clients,
	WorldSaver &worldSaver, std::vector<ServerTask> &waitingTasks, Profiler &serverProfiler)
{


	if (1)
	{
		std::minstd_rand rng(seed);

		::tickDeltaTime = tickDeltaTime;
		::currentTimer = currentTimer;
		::tickDeltaTimeMs = tickDeltaTimeMs;
		::worldSaver = &worldSaver;

		PL::Profiler pl;

		pl.start();

		chunkRegionsData.clear();
		chunkRegionsData.reserve(10);

		serverProfiler.startSubProfile("Bfs for split update logic");
	#pragma region start bfs

		std::unordered_map<glm::ivec2, int> visited;

		for (auto i : chunkCache.savedChunks)
		{
			if (!i.second->otherData.shouldUnload
				//&& i.second->otherData.withinSimulationDistance
				)
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
								if (!foundChunk->second->otherData.shouldUnload
									//&& foundChunk->second->otherData.withinSimulationDistance
									)
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
		serverProfiler.endSubProfile("Bfs for split update logic");

		serverProfiler.startSubProfile("Prepare tasks");

		threadPool.taskTaken.resize(chunkRegionsData.size());
		for (auto &i : threadPool.taskTaken) { i = 0; }

		//todo fix lol
		//if (chunkRegionsData.size() > 1)
		//{
		//	std::cout << chunkRegionsData.size() << "!!!\n";
		//}

		//weak threads up
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

				threadPool.setThreadsNumber(averageInt - 1, workerThread);
				counterPosition = 0;
			}
			else
			{
				counterPosition++;
			}
		}
	#pragma endregion


		for (auto &c : chunkRegionsData)
		{
			c.seed = rng();
		}

		
	#pragma region set waiting tasks for each region

		std::unordered_map<std::uint64_t, int> playersRegions;

		//todo optimize this thing
		for (int i = 0; i < chunkRegionsData.size(); i++)
		{
			auto &cache = chunkRegionsData[i].chunkCache;

			for (auto &c : cache.savedChunks)
			{
				if (!c.second->entityData.players.empty())
				{
					for (auto &p : c.second->entityData.players)
					{
						playersRegions[p.first] = i;
					}
				}
			}
		}
			
		if (playersRegions.empty() && !waitingTasks.empty())
		{
			permaAssertComment(0, "No players in split update logic.cpp");
		}

		//TODO, some older tasks might happen outside chunk regions for the players, so we should check for that, 
		// and not allow them.
		for (auto &t : waitingTasks)
		{
				
			auto cid = t.cid;
			permaAssertComment(cid != 0, "Cid can't be 0 in split update logic.cpp");

			auto found = playersRegions.find(cid);

			permaAssertComment(found != playersRegions.end(), "invalid cid in split update logic.cpp");

			chunkRegionsData[found->second].waitingTasks.push_back(t);
		}
		waitingTasks.clear();

	#pragma endregion


		serverProfiler.endSubProfile("Prepare tasks");


		//start race
		threadPool.setThrerIsWork();

		auto rez = pl.end();
		//std::cout << threadPool.currentCounter << std::fixed
		//	<< " Duration ms: " << (rez.timeSeconds/1000.f) << '\n';

		serverProfiler.startSubProfile("Multi Threaded tick update");
		while (true)
		{
			int taskIndex = tryTakeTask();
			if (taskIndex < 0) { break; }

			//std::cout << "main took task " << taskIndex << '\n';

			Profiler *p = 0;
			if (taskIndex == 0) { p = &gameTickProfiler; }

			//tick
			doGameTick(tickDeltaTime, tickDeltaTimeMs, currentTimer,
				chunkRegionsData[taskIndex].chunkCache,
				chunkRegionsData[taskIndex].orphanEntities,
				chunkRegionsData[taskIndex].seed,
				chunkRegionsData[taskIndex].waitingTasks,
				worldSaver, p
				);
		}


		threadPool.waitForEveryoneToFinish();
		serverProfiler.endSubProfile("Multi Threaded tick update");


		chunkCache.entityChunkPositions.clear();

		for (auto &region : chunkRegionsData)
		{
			chunkCache.entityChunkPositions.merge(region.chunkCache.entityChunkPositions);
		}

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
					std::cout << "Moved!\n";
					auto member = memberSelector(chunk->entityData);
					(*member)[e.first] = e.second;
					chunkCache.entityChunkPositions[e.first] = pos;
				}
				else
				{
					//todo change and make in 2 steps
					std::cout << "OUT!\n";
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
		//deprecated single threaded version
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

#pragma region cache chunk positions checks debugging

	if(INTERNAL_BUILD == 1)
	for (auto &chunk : chunkCache.savedChunks)
	{

		auto &entityData = chunk.second->entityData;
		for (auto &e : entityData.players)
		{
			auto found = chunkCache.entityChunkPositions.find(e.first);

			if (found == chunkCache.entityChunkPositions.end())
			{
				permaAssertComment(0, "entityChunkPositions problem, player entity missing!");
			}
			else
			{
				auto pos = determineChunkThatIsEntityIn(e.second->getPosition());
				
				if (pos != found->second)
				{
					permaAssertComment(0, "entityChunkPositions problem, player desynk position!");
				}
			}
		}

		auto iterateTest = [&](auto &container)
		{
			for (auto &e : container)
			{
				auto found = chunkCache.entityChunkPositions.find(e.first);

				if (found == chunkCache.entityChunkPositions.end())
				{
					permaAssertComment(0, "entityChunkPositions problem, entity missing!");
				}
				else
				{
					auto pos = determineChunkThatIsEntityIn(e.second.getPosition());

					if (pos != found->second)
					{
						permaAssertComment(0, "entityChunkPositions problem, entity desynk position!");
					}
				}
			}

		};

		REPEAT_FOR_ALL_ENTITIES_NO_PLAYERS(ITERATE_TEST);



	}


#pragma endregion


}

void workerThread(int index, ThreadPool &threadPool)
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
				Profiler *p = 0;
				if (taskIndex == 0) { p = &gameTickProfiler; }

				//tick
				doGameTick(tickDeltaTime, tickDeltaTimeMs, currentTimer,
					chunkRegionsData[taskIndex].chunkCache,
					chunkRegionsData[taskIndex].orphanEntities,
					chunkRegionsData[taskIndex].seed,
					chunkRegionsData[taskIndex].waitingTasks,
					*worldSaver, p
					);
			}

			//done work
			threadPool.threIsWork[index] = false;
		}

		
	}

	//std::cout << "Closed thread " << index << "\n";
}

void ThreadPool::setThreadsNumber(int nr, void(*worker)(int, ThreadPool &))
{
	if (nr < 0) { nr = 0; }
	if (nr >= MAX_THREADS)
	{
		nr = MAX_THREADS - 1;
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
				threads[i] = std::move(std::thread(worker, i, std::ref(*this)));
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

void ThreadPool::cleanup()
{
	for (int i = 0; i < currentCounter; i++)
	{
		threIsWork[i] = 0;
	}
	setThreadsNumber(0, workerThread);
	taskTaken.clear();
}

