#include <gameplay/pig.h>



void Pig::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	updateForces(deltaTime, true);



	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, {0.8,0.8,0.8});
}

void PigClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	entity.update(deltaTime, chunkGetter);

}

void PigServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng)
{


	entity.update(deltaTime, chunkGetter);

}
