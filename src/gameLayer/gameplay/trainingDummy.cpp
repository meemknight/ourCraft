#include "gameplay/trainingDummy.h"



void TrainingDummy::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{


}

glm::vec3 TrainingDummy::getColliderSize()
{
	return glm::vec3(0.8, 1.6, 0.8);
}

glm::vec3 TrainingDummy::getColliderOffset()
{
	return glm::vec3(0, 0.2, 0.0);
}


glm::vec3 TrainingDummy::getMaxColliderSize()
{
	return glm::vec3(1.5, 2, 1.5);
}

void TrainingDummyClient::update(float deltaTime, 
	decltype(chunkGetterSignature) *chunkGetter)
{

}

void TrainingDummyClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{

}

int TrainingDummyClient::getTextureIndex()
{
	return ModelsManager::TexturesLoaded::TrainingDummyTexture;
}

bool TrainingDummyServer::update(float deltaTime, 
	decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, 
	std::uint64_t yourEID, std::unordered_set<std::uint64_t> &othersDeleted, 
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding, 
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition, std::unordered_map<std::uint64_t, Client *> &allClients)
{
	return false;
}
