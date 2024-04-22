#pragma once
#include <gameplay/entity.h>
#include <random>





struct Pig: public PhysicalEntity, public HasOrientationAndHeadTurnDirection,
	public MovementSpeedForLegsAnimations
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);


};


struct PigClient: public ClientEntity<Pig>
{
	
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

	void setEntityMatrix(glm::mat4 *skinningMatrix);
};


struct PigServer: public ServerEntity<Pig>
{

	glm::vec2 direction = {};
	int moving = 0;
	float waitTime = 1;


	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);


};