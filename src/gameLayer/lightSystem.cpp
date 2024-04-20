#include <lightSystem.h>
#include <chunkSystem.h>
#include <iostream>
#include <chrono>
#include <unordered_set>

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

	const int maxUpperBoundPerElement = 1000;
	const int totalMaxUpperBound = 100'000;
	//const int maxUpperBound = 1000;


	//Timer sunLightsRemoveTimer;

	//sunLightsRemoveTimer.start();

	int upperBound = totalMaxUpperBound;
	
	static std::unordered_set<glm::ivec2> chunksToUpdate;
	chunksToUpdate.clear();
	chunksToUpdate.reserve(300);

	auto setChunkAndNeighboursFlagDirtyFromBlockPos = [&](int x, int z)
	{
		const int o = 5;
		glm::ivec2 offsets[o] = {{0,0}, {-1,0}, {1,0}, {0, -1}, {0, 1}};

		for (int i = 0; i < o; i++)
		{
			auto pos = glm::ivec2(x + offsets[i].x, z + offsets[i].y);

			pos.x = divideChunk(pos.x);
			pos.y = divideChunk(pos.y);

			chunksToUpdate.insert(pos);
		}
	};

	while(upperBound > 0)
	{
		int elementUpperBount = maxUpperBoundPerElement;
		while (!sunLigtsToRemove.empty() && (elementUpperBount--) > 0)
		{
			upperBound--;
			//sunLightsRemoveTimer.counter++; 
	
			//todo add this here
			//if(b->getSkyLight() != 0)continue;
	
			auto element = sunLigtsToRemove.front();
			sunLigtsToRemove.pop_front();
	
			if (element.intensity == -1) { break; }
	
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
					else if (b2Level >= currentLightLevel)
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
	
			//setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);
			chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);
	
		}
		//sunLightsRemoveTimer.end();
	
	
		//Timer sunLightsAddTimer;
		//sunLightsAddTimer.start();
	
		elementUpperBount = maxUpperBoundPerElement;
		while (!sunLigtsToAdd.empty() && (elementUpperBount--) > 0)
		{
			upperBound--;
			//sunLightsAddTimer.counter++;
	
			auto element = sunLigtsToAdd.front();
			sunLigtsToAdd.pop_front();
			if (element.intensity == -1) { break; }
	
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
					if (bottom)
						checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, bottom, currentLightLevel);
				}
				else
				{
					if (bottom)
						checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, bottom, currentLightLevel - 1);
				}

				if (back)
					checkNeighbout({element.pos.x - 1, element.pos.y, element.pos.z}, back, currentLightLevel - 1);

				if (front)
					checkNeighbout({element.pos.x + 1, element.pos.y, element.pos.z}, front, currentLightLevel - 1);

				if (top)
					checkNeighbout({element.pos.x, element.pos.y + 1, element.pos.z}, top, currentLightLevel - 1);

				if (right)
					checkNeighbout({element.pos.x, element.pos.y, element.pos.z + 1}, right, currentLightLevel - 1);

				if (left)
					checkNeighbout({element.pos.x, element.pos.y, element.pos.z - 1}, left, currentLightLevel - 1);

				//setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);
				chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);

			}
	
		}
	
		//sunLightsAddTimer.end();
	
		//...
	
		elementUpperBount = maxUpperBoundPerElement;
		while (!ligtsToRemove.empty() && (elementUpperBount--) > 0)
		{
				elementUpperBount--;

			auto element = ligtsToRemove.front();
			ligtsToRemove.pop_front();
			if (element.intensity == -1) { break; }

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

			//setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);
			chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);

		}
	
		elementUpperBount = maxUpperBoundPerElement;
		while (!ligtsToAdd.empty() && (elementUpperBount--) > 0)
		{
				elementUpperBount--;

			auto element = ligtsToAdd.front();
			ligtsToAdd.pop_front();
			if (element.intensity == -1) { break; }

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

				//setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);
				chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);

			}

		}

		if (sunLigtsToRemove.empty() && sunLigtsToAdd.empty() && ligtsToRemove.empty() &&
			ligtsToAdd.empty())
		{
			break;
		}
	};

	if (!sunLigtsToAdd.empty())
	{
		Light l;
		l.intensity = -1;
		sunLigtsToAdd.push_back(l);
	}

	if (!sunLigtsToRemove.empty())
	{
		Light l;
		l.intensity = -1;
		sunLigtsToRemove.push_back(l);
	}

	if (!ligtsToAdd.empty())
	{
		Light l;
		l.intensity = -1;
		ligtsToAdd.push_back(l);
	}

	if (!ligtsToRemove.empty())
	{
		Light l;
		l.intensity = -1;
		ligtsToRemove.push_back(l);
	}

	//for (auto &i : chunksToUpdate)
	//{
	//	//the positions are not in block space!
	//	auto c = chunkSystem.getChunkSafeFromBlockPos(i.x * CHUNK_SIZE, i.y * CHUNK_SIZE);
	//	if (c)
	//	{
	//		c->dirty = true;
	//	}
	//}

	//if (sunLightsAddTimer.counter > 1000)
	//{
	//	//std::cout << "Added: " << sunLightsAddTimer.counter << " in: " <<
	//	//	sunLightsAddTimer.getTimerInMiliseconds() << " ms,   " <<
	//	//	sunLightsAddTimer.getTimerInMiliseconds() / sunLightsAddTimer.counter << " ms per light\n";
	//}

}

void LightSystem::addSunLight(ChunkSystem &chunkSystem, glm::ivec3 pos, char intensity)
{
	if (pos.y == CHUNK_HEIGHT - 1)
	{
		intensity = 15;
	}

	auto b = chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);
	if (b)
	{
		b->setSkyLevel(intensity);
	}
	sunLigtsToAdd.push_back({pos, intensity});
	
}

void LightSystem::addSunLightAndPropagateDown(ChunkSystem &chunkSystem, glm::ivec3 pos, char intensity)
{
	if (pos.y == CHUNK_HEIGHT - 1)
	{
		intensity = 15;
	}

	auto b = chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);
	if (b)
	{
		b->setSkyLevel(intensity);
	}
	sunLigtsToAdd.push_back({pos, intensity});

	if (intensity == 15)
	{
		pos.y--;
		while (pos.y > 0)
		{
			auto b = chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);
			if (b && !b->isOpaque())
			{
				b->setSkyLevel(intensity);
			}
			else
			{
				break;
			}
	
			sunLigtsToAdd.push_back({pos, intensity});
	
			pos.y--;
		}
	}

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

void LightSystem::setSunlightForAnEntireChunk(Chunk &chunk, ChunkSystem &chunkSystem)
{

	int xStart = chunk.data.x * CHUNK_SIZE;
	int ZStart = chunk.data.z * CHUNK_SIZE;
	
	//auto checkNeighbour = [&](glm::ivec3 pos, unsigned char light, unsigned char decrease = 0)
	//{
	//	auto b = chunk.safeGet(pos.x, pos.y, pos.z);
	//	if (b)
	//	{
	//		unsigned char sky = b->getSkyLight();
	//		if (decrease || sky < 15) { sky--; }
	//
	//		return std::max(light, sky);
	//	}
	//	return light;
	//};
	//
	//for (int x = 0; x < CHUNK_SIZE; x++)
	//	for (int z = 0; z < CHUNK_SIZE; z++)
	//	{
	//		auto &b = chunk.unsafeGet(x, CHUNK_HEIGHT-1, z);
	//		if (!b.isOpaque())
	//		{
	//			b.setSkyLevel(15);
	//		}
	//		else
	//		{
	//			b.setSkyLevel(0);
	//		}
	//	}
	//
	//for (int y = CHUNK_HEIGHT - 2; y > 0; y--)
	//{
	//	for (int x = 0; x < CHUNK_SIZE; x++)
	//		for (int z = 0; z < CHUNK_SIZE; z++)
	//		{
	//
	//			auto &b = chunk.unsafeGet(x, CHUNK_HEIGHT - 1, z);
	//			if (!b.isOpaque())
	//			{
	//				int xPos = x + xStart;
	//				int zPos = z + ZStart;
	//
	//				glm::ivec3 pos = {x,y,z};
	//
	//				unsigned char light = checkNeighbour(pos + glm::ivec3(0, 1, 0), 0, 0);
	//				light = checkNeighbour(pos + glm::ivec3(1, 0, 0), light, 1);
	//				light = checkNeighbour(pos + glm::ivec3(0, 0, 1), light, 1);
	//				light = checkNeighbour(pos + glm::ivec3(-1, 0, 0), light, 1);
	//				light = checkNeighbour(pos + glm::ivec3(0, 0, -1), light, 1);
	//
	//				light = std::max(light, unsigned char(0));
	//				b.setSkyLevel(light);
	//			}
	//			else
	//			{
	//				b.setSkyLevel(0);
	//			}
	//
	//
	//		}
	//
	//}

	for (int xPos = xStart; xPos < xStart + CHUNK_SIZE; xPos++)
		for (int zPos = ZStart; zPos < ZStart + CHUNK_SIZE; zPos++)
		{
			if (!chunk.unsafeGet(xPos - xStart, CHUNK_HEIGHT - 1, zPos - ZStart).isOpaque())
			{
				addSunLight(chunkSystem, {xPos, CHUNK_HEIGHT - 1, zPos}, 15);
				//addSunLightAndPropagateDown(chunkSystem, {xPos, CHUNK_HEIGHT - 1, zPos}, 15);
			}
		}


}
