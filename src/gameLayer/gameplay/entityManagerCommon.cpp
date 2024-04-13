#include <gameplay/entityManagerCommon.h>
#include <glm/glm.hpp>

//todo entities should freeze their state completely when chunks are missing

void computeRubberBand(RubberBand &rubberBand, glm::dvec3 &position, float deltaTime)
{
	rubberBand.timer -= deltaTime;
	if (rubberBand.timer < 0) { rubberBand.timer = 0; }

	rubberBand.position = glm::mix(rubberBand.startPosition, position, rubberBand.timer);
	rubberBand.startPosition = rubberBand.position;
}

void RubberBand::computeRubberBand(glm::dvec3 &position, float deltaTime)
{
	::computeRubberBand(*this, position, deltaTime);
}


void updateDroppedItem(DroppedItem &item, float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{

	updateForces(item.position, item.forces, deltaTime, true);

	RigidBody body;
	body.pos = item.position;
	body.lastPos = item.lastPosition;

	body.resolveConstrains(chunkGetter, &item.forces, deltaTime, {0.4,0.4,0.4});

	item.lastPosition = body.pos;
	item.position = body.pos;


}

