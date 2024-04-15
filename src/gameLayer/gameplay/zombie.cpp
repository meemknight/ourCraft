#include <gameplay/zombie.h>
#include <multyPlayer/serverChunkStorer.h>


void Zombie::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{

	updateForces(deltaTime, true);

	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, {0.8, 1.8, 0.8});

}

void ZombieServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer)
{
	getPosition().x += 1 * deltaTime;

	entity.update(deltaTime, chunkGetter);
}

void ZombieClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	entity.update(deltaTime, chunkGetter);
}
