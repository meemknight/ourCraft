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


	waitTime -= deltaTime;

	if (waitTime < 0)
	{
		moving = getRandomNumber(rng, 0, 1);
		waitTime += getRandomNumberFloat(rng, 1, 8);

		if (moving)
		{
			direction = getRandomUnitVector(rng);
		}
	}

	if (moving)
	{
		auto move = 1.f * deltaTime * direction;
		getPosition().x += move.x;
		getPosition().z += move.y;

		entity.bodyOrientation = move;
		entity.lookDirectionAnimation;
	}


	entity.update(deltaTime, chunkGetter);

}
