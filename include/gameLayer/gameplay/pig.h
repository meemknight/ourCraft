#pragma once
#include <gameplay/entity.h>
#include <random>





struct Pig: public PhysicalEntity
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);


};


struct PigClient: public ClientEntity<Pig>
{
	
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

};


struct PigServer: public ServerEntity<Pig>
{


	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);


};