#include <lightSystem.h>
#include <chunkSystem.h>
#include <iostream>


void LightSystem::update(ChunkSystem &chunkSystem)
{
	if (dontUpdateLightSystem)
	{
		sunLigtsToRemove.clear();
		sunLigtsToAdd.clear();
	}

	while (!sunLigtsToRemove.empty())
	{
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

	while (!sunLigtsToAdd.empty())
	{
		auto element = sunLigtsToAdd.front();
		sunLigtsToAdd.pop_front();

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
					if (!b2->isOpaque() && b2->getSkyLight() < newLightLevel)
					{
						LightSystem::Light l;
						l.intensity = newLightLevel;
						l.pos = p;
						sunLigtsToAdd.push_back(l);
						b2->setSkyLevel(newLightLevel);
					}
				}
			};

			if (currentLightLevel == 15)
			{
				checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, currentLightLevel);
			}
			else
			{
				checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, currentLightLevel - 1);
			}

			checkNeighbout({element.pos.x - 1, element.pos.y, element.pos.z}, currentLightLevel - 1);
			checkNeighbout({element.pos.x + 1, element.pos.y, element.pos.z}, currentLightLevel - 1);
			checkNeighbout({element.pos.x, element.pos.y + 1, element.pos.z}, currentLightLevel - 1);

			checkNeighbout({element.pos.x, element.pos.y, element.pos.z + 1}, currentLightLevel - 1);
			checkNeighbout({element.pos.x, element.pos.y, element.pos.z - 1}, currentLightLevel - 1);

			chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);

		}

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
