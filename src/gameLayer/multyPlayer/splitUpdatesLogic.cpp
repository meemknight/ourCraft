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


void splitUpdatesLogic(float tickDeltaTime, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache)
{

	PL::Profiler pl;

	pl.start();

	static std::vector<PerTickData> chunkRegionsData;
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
						visited.insert({el, currentIndex});

						auto foundChunk = chunkCache.savedChunks.find(el);

						if (foundChunk != chunkCache.savedChunks.end())
						{
							if(!foundChunk->second->otherData.shouldUnload)
							{
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

	std::cout << chunkRegionsData.size() << std::fixed 
		<< " Duration ms: " << (rez.timeSeconds/1000.f) << '\n';

	for (auto &region : chunkRegionsData)
	{


		doGameTick(tickDeltaTime, currentTimer,
			region.chunkCache, region.orphanEntities);

	}




	//todo save orphan entities to disk...



}