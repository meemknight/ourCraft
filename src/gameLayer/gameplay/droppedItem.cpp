#include <gameplay/droppedItem.h>
#include <multyPlayer/serverChunkStorer.h>
#include <multyPlayer/enetServerFunction.h>


void DroppedItem::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{


	//THIS IS SHARED CODE!!!!
	PhysicalSettings ps;
	ps.gravityModifier = 0.5f;
	ps.sideFriction = 1.f;
	updateForces(deltaTime, true, ps);
	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, getMaxColliderSize(), ps);

}

glm::vec3 DroppedItem::getMaxColliderSize()
{
	return {0.4,0.4,0.4};
}

glm::vec3 DroppedItem::getColliderSize()
{
	return getMaxColliderSize();
}

glm::vec3 DroppedItemServer::getMaxColliderSize()
{
	return {0.4,0.4,0.4};
}

glm::vec3 DroppedItemServer::getColliderSize()
{
	return getMaxColliderSize();
}

DroppedItem DroppedItemServer::getDataToSend()
{
	DroppedItem ret;
	ret.type = item.type;
	ret.count = item.counter;

	ret.forces = entity.forces;
	ret.lastPosition = entity.lastPosition;
	ret.position = entity.position;

	return ret;
}

bool DroppedItemServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition
)
{
	//todo use persistent timer
	stayTimer -= deltaTime;
	
	if (dontPickTimer>0)
	dontPickTimer -= deltaTime;

	
	
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
					if (glm::distance(getPosition(), p.second.getPosition()) < 1.f && item.type == p.second.item.type)
					{

						const int stackSize = p.second.item.getStackSize();
						if (item.counter + p.second.item.counter < stackSize)
						{
							//merge the 2 items
							item.counter += p.second.item.counter;
							othersDeleted.insert(p.first);
							c->entityData.droppedItems.erase(p.first);

							stayTimer = std::max(stayTimer, p.second.stayTimer);

							break;
						}
						else if(item.counter < stackSize && p.second.item.counter < stackSize)
						{
							//steal sum
							p.second.item.counter = (item.counter + p.second.item.counter) - stackSize;
							item.counter = stackSize;
						}

						
					};

				}
			}
		}
	}


	if(dontPickTimer<=0)
	for (auto &p : playersPosition)
	{
		if (glm::distance(getPosition(), p.second) < 1.f)
		{

			auto client = getClientNotLocked(p.first);

			if (client)
			{

				//pickupped this item
				int pickupped = client->playerData.inventory.tryPickupItem(item);
				if (pickupped)
				{
					sendPlayerInventoryAndIncrementRevision(*client);

					item.counter -= pickupped;
					if(item.counter <= 0)
					{
						return 0;
					}
				}

			}

		}
	}

	//
	//auto client = getClient(0);
	//sendPlayerInventory(client);

	doCollisionWithOthers(getPosition(), getMaxColliderSize(), entity.forces,
		serverChunkStorer, yourEID);
	
	//THIS IS SHARED CODE!!!!
	PhysicalSettings ps;
	ps.gravityModifier = 0.5f;
	ps.sideFriction = 1.f;
	entity.updateForces(deltaTime, true, ps);
	entity.resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, getMaxColliderSize(), ps);


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
