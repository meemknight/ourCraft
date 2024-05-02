#include <gameplay/droppedItem.h>
#include <multyPlayer/serverChunkStorer.h>



void DroppedItem::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{

	updateForces(deltaTime, true);

	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, {0.4,0.4,0.4});


}

void DroppedItemServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng)
{
	entity.update(deltaTime, chunkGetter);
}

void DroppedItemServer::appendDataToDisk(std::ofstream &f, std::uint64_t eId)
{
	basicEntitySave(f, Markers::droppedItem, eId, &entity, sizeof(entity));
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
