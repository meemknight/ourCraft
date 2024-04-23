#pragma once
#include <gameplay/entity.h>
#include <random>
#include <gameplay/ai.h>



struct Pig: public PhysicalEntity, public HasOrientationAndHeadTurnDirection,
	public MovementSpeedForLegsAnimations
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);


};


struct PigClient: public ClientEntity<Pig, PigClient>
{
	
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

	void setEntityMatrix(glm::mat4 *skinningMatrix);
};



struct PigServer: public ServerEntity<Pig>, public AnimalBehaviour < PigServer, PigDefaultSettings >
{



	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);



};