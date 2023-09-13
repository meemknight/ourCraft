#include <lightSystem.h>
#include <chunkSystem.h>


void LightSystem::update(ChunkSystem &chunkSystem)
{
	return;
	while (!sunLigtsToAdd.empty())
	{
		auto element = sunLigtsToAdd.front();
		sunLigtsToAdd.pop_front();

		auto b = chunkSystem.getBlockSafe(element.pos.x, element.pos.y, element.pos.z);
		if (b)
		{
			char currentLightLevel = b->getSkyLight();
			if (element.intensity > currentLightLevel)
			{
				b->setSkyLevel(element.intensity);
				currentLightLevel = element.intensity;
			}
			else
			{
				continue;
			}

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

			//checkNeighbout({element.pos.x - 1, element.pos.y, element.pos.z}, currentLightLevel - 1);
			//checkNeighbout({element.pos.x + 1, element.pos.y, element.pos.z}, currentLightLevel - 1);
			//checkNeighbout({element.pos.x, element.pos.y + 1, element.pos.z}, currentLightLevel - 1);
			checkNeighbout({element.pos.x, element.pos.y - 1, element.pos.z}, currentLightLevel);
			//checkNeighbout({element.pos.x, element.pos.y, element.pos.z + 1}, currentLightLevel - 1);
			//checkNeighbout({element.pos.x, element.pos.y, element.pos.z - 1}, currentLightLevel - 1);

			chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(element.pos.x, element.pos.z);


		}

	}


}
