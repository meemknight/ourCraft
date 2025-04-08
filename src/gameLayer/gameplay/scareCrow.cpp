#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>
#include <gameplay/scareCrow.h>
#include <multyPlayer/serverChunkStorer.h>
#include <iostream>
#include <glm/gtx/quaternion.hpp>
#include <rendering/model.h>



void ScareCrow::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{

	updateForces(deltaTime, true);

	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, getColliderSize());

}

glm::vec3 ScareCrow::getColliderSize()
{
	return getMaxColliderSize();
}

glm::vec3 ScareCrow::getMaxColliderSize()
{
	return glm::vec3(0.8, 2.2, 0.8);
}

void ScareCrowClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	entityBuffered.update(deltaTime, chunkGetter);
}

void ScareCrowClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{



}

int ScareCrowClient::getTextureIndex()
{
	return ModelsManager::TexturesLoaded::scareCrowTexture;
}

void ScareCrowServer::appendDataToDisk(std::ofstream &f, std::uint64_t eId)
{
}

//todo temporary allocator
bool ScareCrowServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFindingSurvival,
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPositionSurvival,
	std::unordered_map < std::uint64_t, Client *> &allClients
)
{
	BasicEnemyBehaviourOtherSettings settings;
	settings.hearBonus = -1; //no hearing
	settings.searchDistance = 30;
	settings.runSpeed = 1.f;

	basicEnemyBehaviour.update(this, deltaTime, chunkGetter, serverChunkStorer, rng, yourEID, othersDeleted,
		pathFindingSurvival, playersPositionSurvival, getPosition(), allClients, settings);

	doCollisionWithOthers(getPosition(), entity.getMaxColliderSize(), entity.forces,
		serverChunkStorer, yourEID);

	entity.update(deltaTime, chunkGetter);


	return true;
}

WeaponStats ScareCrowServer::getWeaponStats()
{
	WeaponStats weaponStats;

	weaponStats.damage = 10;
	weaponStats.critDamage = 15;
	weaponStats.surprizeDamage = 15;
	weaponStats.critChance = 0.1;
	weaponStats.speed = 1.5;
	weaponStats.armourPenetration = 0;
	weaponStats.accuracy = 6; //accuracy is used for enemies to determine how corectly they hit
	weaponStats.range = 1.5;
	weaponStats.knockBack = 4;

	return weaponStats;
}

