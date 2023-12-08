#include <lightSystem.h>
#include <chunkSystem.h>
#include <iostream>
#include <chrono>

struct Timer
{
	std::chrono::steady_clock::time_point begin;
	std::chrono::steady_clock::time_point endp;

	int counter = 0;

	void start()
	{
		begin = std::chrono::steady_clock::now();
	}

	void end()
	{
		endp = std::chrono::steady_clock::now();
	}

	float getTimerInMiliseconds()
	{
		return 
			std::chrono::duration_cast<std::chrono::nanoseconds> (endp - begin).count() / 1'000'000.f;
	}
};

void LightSystem::update(ChunkSystem &chunkSystem)
{
	if (dontUpdateLightSystem)
	{
		sunLigtsToRemove.clear();
		sunLigtsToAdd.clear();
		ligtsToRemove.clear();
		ligtsToAdd.clear();
	}

	const int maxUpperBound = 100000;
	
	

	Timer sunLightsRemoveTimer;

	sunLightsRemoveTimer.start();

	int upperBound = maxUpperBound;
	while (!sunLigtsToRemove.empty() && (upperBound--) > 0)
	{
		sunLightsRemoveTimer.counter++; 

		//todo add this here
		//if(b->getSkyLight() != 0)continue;

		auto element = sunLigtsToRemove.front();
		sunLigtsToRemove.pop_front();
		
		char currentLightLevel = element.intensity;

		auto checkNeighbout = [&](glm::ivec3 p, bool down = 0)
		{
			auto b2 = chunkSystem.getBlockSafe(p.x, p.y, p.z);

			if (b2 && !b2->isOpaque())
			{
				int b2Level = b2->getSkyLight();

				if ((down && currentLightLevel == 15) || (b2Level != 0 && b2Level < currentLightLevel))
				{
					//remove
					LightSystem::Light l;
					l.intensity = b2Level;
					l.pos = p;
					sunLigtsToRemove.push_back(l);
					b2->setSkyLevel(0);
				}
				else if(b2Level >= currentLightLevel)
				{
					//add
					LightSystem::Light l;
					l.intensity = b2Level;
					l.pos = p;
					sunLigtsToAdd.push_back(l);
					//b2->setSkyLevel(b2Level); //it already is
				}
			}
		};

		checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, true);

		checkNeighbout({element.pos.x - 1, element.pos.y, element.pos.z});
		checkNeighbout({element.pos.x + 1, element.pos.y, element.pos.z});

		checkNeighbout({element.pos.x, element.pos.y + 1, element.pos.z});

		checkNeighbout({element.pos.x, element.pos.y, element.pos.z + 1});
		checkNeighbout({element.pos.x, element.pos.y, element.pos.z - 1});

		chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);

	}
	sunLightsRemoveTimer.end();


	Timer sunLightsAddTimer;
	sunLightsAddTimer.start();

	upperBound = maxUpperBound;
	while (!sunLigtsToAdd.empty() && (upperBound--) > 0)
	{
		sunLightsAddTimer.counter++;

		auto element = sunLigtsToAdd.front();
		sunLigtsToAdd.pop_front();

		Block *b, *front, *back, *top, *bottom, *left, *right;

		chunkSystem.getBlockSafeWithNeigbhoursStopIfCenterFails(element.pos.x, element.pos.y, element.pos.z,
			b, front, back, top, bottom, left, right);

		//auto b = chunkSystem.getBlockSafe(element.pos.x, element.pos.y, element.pos.z);
		if (!b || b->getSkyLight() != element.intensity)
		{
			continue;
		}
		
		{
			//c->dontDrawYet = false; 

			//char currentLightLevel = b->getSkyLight();
			char currentLightLevel = element.intensity;

			auto checkNeighbout = [&](glm::ivec3 p, Block *b2, char newLightLevel)
			{
				if (!b2->isOpaque() && b2->getSkyLight() < newLightLevel)
				{
					LightSystem::Light l;
					l.intensity = newLightLevel;
					l.pos = p;
					sunLigtsToAdd.push_back(l);
					b2->setSkyLevel(newLightLevel);
				}
			};

			if (currentLightLevel == 15)
			{
				if(bottom)
					checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, bottom, currentLightLevel);
			}
			else
			{
				if (bottom)
					checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, bottom, currentLightLevel - 1);
			}

			if (back)
				checkNeighbout({element.pos.x - 1, element.pos.y, element.pos.z}, back, currentLightLevel - 1);
			
			if(front)
				checkNeighbout({element.pos.x + 1, element.pos.y, element.pos.z}, front, currentLightLevel - 1);
			
			if(top)
			checkNeighbout({element.pos.x, element.pos.y + 1, element.pos.z}, top, currentLightLevel - 1);

			if(right)
				checkNeighbout({element.pos.x, element.pos.y, element.pos.z + 1}, right, currentLightLevel - 1);
			
			if(left)
				checkNeighbout({element.pos.x, element.pos.y, element.pos.z - 1}, left, currentLightLevel - 1);

			chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);

		}


	}

	sunLightsAddTimer.end();

	//...

	upperBound = maxUpperBound;
	while (!ligtsToRemove.empty() && (upperBound--) > 0)
	{
		auto element = ligtsToRemove.front();
		ligtsToRemove.pop_front();

		char currentLightLevel = element.intensity;

		auto checkNeighbout = [&](glm::ivec3 p)
		{
			auto b2 = chunkSystem.getBlockSafe(p.x, p.y, p.z);

			if (b2 && !b2->isOpaque())
			{
				int b2Level = b2->getLight();

				if (b2Level != 0 && b2Level < currentLightLevel)
				{
					//remove
					LightSystem::Light l;
					l.intensity = b2Level;
					l.pos = p;
					ligtsToRemove.push_back(l);
					b2->setLightLevel(0);
				}
				else if (b2Level >= currentLightLevel)
				{
					//add
					LightSystem::Light l;
					l.intensity = b2Level;
					l.pos = p;
					ligtsToAdd.push_back(l);
					//b2->setlightLevel(b2Level); //it already is
				}
			}
		};

		checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z});

		checkNeighbout({element.pos.x - 1, element.pos.y, element.pos.z});
		checkNeighbout({element.pos.x + 1, element.pos.y, element.pos.z});

		checkNeighbout({element.pos.x, element.pos.y + 1, element.pos.z});

		checkNeighbout({element.pos.x, element.pos.y, element.pos.z + 1});
		checkNeighbout({element.pos.x, element.pos.y, element.pos.z - 1});

		chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);
	}

	upperBound = maxUpperBound;
	while (!ligtsToAdd.empty() && (upperBound--) > 0)
	{
		auto element = ligtsToAdd.front();
		ligtsToAdd.pop_front();

		auto b = chunkSystem.getBlockSafe(element.pos.x, element.pos.y, element.pos.z);
		if (!b || b->getLight() != element.intensity)
		{
			continue;
		}

		//Chunk *c = 0;
		//todo optimize and remove this
		//auto b = chunkSystem.getBlockSafeAndChunk(element.pos.x, element.pos.y, element.pos.z, c);
		//if (b)
		{
			//c->dontDrawYet = false; 

			//char currentLightLevel = b->getSkyLight();
			char currentLightLevel = element.intensity;

			auto checkNeighbout = [&](glm::ivec3 p, char newLightLevel)
			{
				auto b2 = chunkSystem.getBlockSafe(p.x, p.y, p.z);

				if (b2)
				{
					if (!b2->isOpaque() && b2->getLight() < newLightLevel)
					{
						LightSystem::Light l;
						l.intensity = newLightLevel;
						l.pos = p;
						ligtsToAdd.push_back(l);
						b2->setLightLevel(newLightLevel);
					}
				}
			};

			checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, currentLightLevel - 1);

			checkNeighbout({element.pos.x - 1, element.pos.y, element.pos.z}, currentLightLevel - 1);
			checkNeighbout({element.pos.x + 1, element.pos.y, element.pos.z}, currentLightLevel - 1);
			checkNeighbout({element.pos.x, element.pos.y + 1, element.pos.z}, currentLightLevel - 1);

			checkNeighbout({element.pos.x, element.pos.y, element.pos.z + 1}, currentLightLevel - 1);
			checkNeighbout({element.pos.x, element.pos.y, element.pos.z - 1}, currentLightLevel - 1);

			chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);

		}

	}


	if (sunLightsAddTimer.counter > 1000)
	{
		//std::cout << "Added: " << sunLightsAddTimer.counter << " in: " <<
		//	sunLightsAddTimer.getTimerInMiliseconds() << " ms,   " <<
		//	sunLightsAddTimer.getTimerInMiliseconds() / sunLightsAddTimer.counter << " ms per light\n";
	}

}

void LightSystem::addSunLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char intensity)
{
	auto b = chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);
	if (b)
	{
		b->setSkyLevel(intensity);
	}
	sunLigtsToAdd.push_back({pos, intensity});
}

void LightSystem::removeSunLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char oldVal)
{
	auto b = chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);
	if (b)
	{
		b->setSkyLevel(0);
	}

	sunLigtsToRemove.push_back({pos, oldVal});
}

void LightSystem::addLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char intensity)
{
	chunkSystem.shouldUpdateLights = true;
	auto b = chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);
	if (b)
	{
		b->setLightLevel(intensity);
	}
	ligtsToAdd.push_back({pos, intensity});
}

void LightSystem::removeLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char oldVal)
{
	chunkSystem.shouldUpdateLights = true;
	auto b = chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);
	if (b)
	{
		b->setLightLevel(0);
	}

	ligtsToRemove.push_back({pos, oldVal});
}
