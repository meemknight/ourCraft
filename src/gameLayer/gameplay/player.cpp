#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "gameplay/player.h"
#include <iostream>
#include <rendering/model.h>

void Player::flyFPS(glm::vec3 direction, glm::vec3 lookDirection)
{
	lookDirection.y = 0;
	float l = glm::length(lookDirection);

	if (!l) { return; }

	lookDirection /= l;

	//forward
	float forward = -direction.z;
	float leftRight = direction.x;
	float upDown = direction.y;

	glm::vec3 move = {};

	move += glm::vec3(0, 1, 0) * upDown;
	move += glm::normalize(glm::cross(lookDirection, glm::vec3(0, 1, 0))) * leftRight;
	move += lookDirection * forward;

	//applyImpulse(this->forces, move);
	this->position += move;
}


void Player::moveFPS(glm::vec3 direction, glm::vec3 lookDirection, float deltaTime)
{
	lookDirection.y = 0;
	lookDirection = glm::normalize(lookDirection);

	//forward
	float forward = -direction.z;
	float leftRight = direction.x;

	glm::vec3 move = {};

	move += glm::normalize(glm::cross(lookDirection, glm::vec3(0, 1, 0))) * leftRight;
	move += lookDirection * forward;

	this->moveDynamic({move.x, move.z}, deltaTime);
}

glm::vec3 Player::getColliderSize()
{
	return getMaxColliderSize();
}

void Player::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	updateForces(deltaTime, !fly);

	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, getColliderSize());
}

glm::vec3 Player::getMaxColliderSize()
{
	return glm::vec3(0.8, 1.8, 0.8);
}



//todo move update here
void PlayerClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	entityBuffered.update(deltaTime, chunkGetter);
}

void PlayerClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{

	//skinningMatrix[0] = skinningMatrix[0] * glm::toMat4(
	//	glm::quatLookAt(glm::normalize(entityBuffered.lookDirectionAnimation), glm::vec3(0, 1, 0)));

}

int PlayerClient::getTextureIndex()
{
	return ModelsManager::TexturesLoaded::SteveTexture;
}

void PlayerServer::kill()
{
	killed = true;
	effects = {};
	newLife.life = 0;
	lifeLastFrame.life = 0;
	notIncreasedLifeSinceTimeSecconds = 0;
	interactingWithBlock = 0;
	revisionNumberInteraction = 0;
	
	effectsTimers = {};
}

float PlayerServer::calculateHealingDelayTime()
{
	float rez = BASE_HEALTH_DELAY_TIME;

	for (int i = PlayerInventory::EQUIPEMENT_START_INDEX; i < PlayerInventory::EQUIPEMENT_START_INDEX +
		PlayerInventory::MAX_EQUIPEMENT_SLOTS; i++)
	{
		auto item = inventory.getItemFromIndex(i, 0);

		if (item->type == ItemTypes::bandage)
		{
			rez -= 5;
		}
	}


	return std::max(rez, 0.f);
}

float PlayerServer::calculateHealingRegenTime()
{
	return BASE_HEALTH_REGEN_TIME;
}

EntityStats getPlayerStats(PlayerInventory &inventory)
{
	EntityStats rez;

	//base player stats
	rez.armour = 0;
	rez.runningSpeed = 8;



	return rez;
}
