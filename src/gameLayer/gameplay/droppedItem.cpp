#include <gameplay/droppedItem.h>
#include <multyPlayer/serverChunkStorer.h>



void DroppedItem::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{

	updateForces(deltaTime, true);

	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, {0.4,0.4,0.4});


}

bool DroppedItemServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted)
{

	stayTimer -= deltaTime;
	
	if (stayTimer < 0)
	{
		return 0;
	}

	glm::ivec2 chunkPosition = determineChunkThatIsEntityIn(getPosition());

	for (auto offset : *getChunkNeighboursOffsets())
	{
		glm::ivec2 pos = chunkPosition + offset;
		auto c = serverChunkStorer.getChunkOrGetNull(pos.x, pos.y);
		if (c)
		{
			for (auto &p : c->entityData.droppedItems)
			{
				if (p.first != yourEID)
				{
					if (glm::distance(getPosition(), p.second.getPosition()) < 1.f)
					{
						//merge the 2 items
						othersDeleted.insert(p.first);
						c->entityData.droppedItems.erase(p.first);
						break;
					};

				}
			}
		}
	}





	entity.update(deltaTime, chunkGetter);

	return true;
}

void DroppedItemServer::appendDataToDisk(std::ofstream &f, std::uint64_t eId)
{
	//basicEntitySave(f, Markers::droppedItem, eId, &entity, sizeof(entity));
}

bool DroppedItemServer::loadFromDisk(std::ifstream &f)
{
	return readData(f, &entity, sizeof(entity));
}

void DroppedItemClient::update(float deltaTime, 
	decltype(chunkGetterSignature) *chunkGetter)
{
	entity.update(deltaTime, chunkGetter);
}

void DroppedItemClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{
}
