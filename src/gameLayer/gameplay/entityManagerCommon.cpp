#include <gameplay/entityManagerCommon.h>


//todo entities should freeze their state completely when chunks are missing

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
