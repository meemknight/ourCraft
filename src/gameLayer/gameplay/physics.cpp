#define GLM_ENABLE_EXPERIMENTAL
#include "gameplay/physics.h"
#include <glm/glm.hpp>
#include <chunkSystem.h>
#include <iostream>
#include <glm/gtx/intersect.hpp>


//used for air friction
void applyDrag(glm::vec3 &force, glm::vec3 drag)
{
	auto lastForce = force;
	force += drag;

	if ((lastForce.x < 0 && force.x > 0) || (lastForce.x > 0 && force.x < 0)) { force.x = 0; }
	if ((lastForce.y < 0 && force.y > 0) || (lastForce.y > 0 && force.y < 0)) { force.y = 0; }
	if ((lastForce.z < 0 && force.z > 0) || (lastForce.z > 0 && force.z < 0)) { force.z = 0; }
}

//used for surface friction
void applyDrag2(glm::vec3 &force, glm::vec3 drag)
{

	if (force.x > 0) { force.x -= drag.x; if(force.x < 0) {force.x = 0;} } else
	if (force.x < 0) { force.x += drag.x;  if(force.x > 0) {force.x = 0;} }

	if (force.y > 0) { force.y -= drag.y; if(force.y < 0) {force.y = 0;} } else
	if (force.y < 0) { force.y += drag.y; if(force.y > 0) {force.y = 0;} }

	if (force.z > 0) { force.z -= drag.z; if(force.z < 0) {force.z = 0;} } else
	if (force.z < 0) { force.z += drag.z; if(force.z > 0) {force.z = 0;} }


}

bool resolveConstrains(
	glm::dvec3 &pos, glm::dvec3 &lastPos,
	decltype(chunkGetterSignature) *chunkGetter, 
	MotionState *forces, float deltaTime, glm::vec3 colliderSize, PhysicalSettings physicalSettings)
{
	bool rez = 0;

	float distance = glm::length(lastPos - pos);
	const float BLOCK_SIZE = 1;
	const float RESOLUTION = 0.4;

	forces->colides = 0;

	if (distance < BLOCK_SIZE * RESOLUTION)
	{
		rez = checkCollisionBrute(pos,
			lastPos,
			chunkGetter, forces, deltaTime, colliderSize, physicalSettings
		);
	}
	else
	{
		glm::dvec3 newPos = lastPos;
		glm::vec3 delta = pos - lastPos;
		delta = glm::normalize(delta);
		delta *= RESOLUTION * BLOCK_SIZE;

		int hardLimit = 10000;

		do
		{
			newPos += delta;
			glm::dvec3 posTest = newPos;
			rez = checkCollisionBrute(newPos,
				lastPos, chunkGetter, forces, deltaTime, colliderSize, physicalSettings
				);

			if (newPos != posTest)
			{
				pos = newPos;
				goto end;
			}

		} while (glm::length((newPos + glm::dvec3(delta)) - pos) > 1.0f * BLOCK_SIZE && hardLimit-- > 0);

		rez = checkCollisionBrute(pos,
			lastPos,
			chunkGetter, forces, deltaTime, colliderSize, physicalSettings);
	}

end:






	//clamp the box if needed
	//if (pos.x < 0) { pos.x = 0; }
	//if (pos.x + dimensions.x > (mapData.w) * BLOCK_SIZE) { pos.x = ((mapData.w) * BLOCK_SIZE) - dimensions.x; }
	
	return rez;

}

bool checkCollisionBrute(glm::dvec3 &pos, glm::dvec3 lastPos,
	decltype(chunkGetterSignature) *chunkGetter, MotionState *forces, float deltaTime
	, glm::vec3 colliderSize, PhysicalSettings physicalSettings)
{
	glm::dvec3 delta = pos - lastPos;
	const float BLOCK_SIZE = 1;

	bool rez = 0;
	
	//pos.y = performCollision({lastPos.x, pos.y, lastPos.z}, colliderSize, {0, delta.y, 0},
	//	chunkGetter).y;

	
	glm::dvec3 newPos = pos;

	glm::vec3 drag = {};
	
	if (delta.x)
	{
		newPos.x = performCollision({pos.x, lastPos.y, lastPos.z}, lastPos, colliderSize, {delta.x, 0, 0},
			chunkGetter, rez, forces, deltaTime, drag, physicalSettings).x;
	}
	
	if (delta.z)
	{
		newPos.z = performCollision({newPos.x, lastPos.y, pos.z}, lastPos, colliderSize, {0, 0, delta.z},
			chunkGetter, rez, forces, deltaTime, drag, physicalSettings).z;
	}
	
	if (delta.y)
	{
		newPos.y = performCollision({newPos.x, pos.y, newPos.z}, lastPos, colliderSize, {0, delta.y, 0},
			chunkGetter, rez, forces, deltaTime, drag, physicalSettings).y;
	}
	
	//goofy ahh collision
	//newPos = performCollision(newPos, lastPos, colliderSize, delta,
	//	chunkGetter, rez, forces, deltaTime, drag, physicalSettings);

	pos = newPos;

	if (forces)
	{
		applyDrag2(forces->velocity, drag * deltaTime);
	}

	/*
	glm::vec3 newPos = performCollision({pos.x, lastPos.y, lastPos.z}, colliderSize, {delta.x, 0, 0},
		chunkGetter);
	
	newPos = performCollision({newPos.x, pos.y, lastPos.z}, colliderSize, {0, delta.y, 0},
		chunkGetter);
	
	//pos.x = newPos.x;
	//pos.y = newPos.y;
	//
	
	pos = performCollision({newPos.x, newPos.y, pos.z}, colliderSize, {0, 0, delta.z},
		chunkGetter);
	*/

	if (!rez)
	{
		pos = lastPos;
	}

	return rez;
}


bool boxColide(glm::dvec3 p1, glm::vec3 s1,
	glm::dvec3 p2, glm::vec3 s2)
{

	const double delta = -0.000001;
	s2.x += delta;
	s2.z += delta;
	s2.y += delta/2.0;
	//p2.y -= delta / 2.f;

	if (
		(p1.x + s1.x / 2.0 > p2.x - s2.x / 2.0) &&
		(p1.x - s1.x / 2.0 < p2.x + s2.x / 2.0) &&

		(p1.y + s1.y > p2.y) &&
		(p1.y < p2.y + s2.y) &&
		//(p1.y + s1.y / 2.0 > p2.y - s2.y / 2.0) &&
		//(p1.y - s1.y / 2.0 < p2.y + s2.y / 2.0) &&

		(p1.z + s1.z / 2.0 > p2.z - s2.z / 2.0) &&
		(p1.z - s1.z / 2.0 < p2.z + s2.z / 2.0)
		)
	{
		return true;
	}
	else
		return false;

}

bool pointInsideBox(glm::dvec3 p, glm::dvec3 box, glm::vec3 size, float delta)
{
	size += glm::vec3(delta);
	box.y -= delta / 2.f;

	return(
		p.x > box.x - size.x / 2.f && p.x < box.x + size.x / 2.f &&
		p.z > box.z - size.z / 2.f && p.z < box.z + size.z / 2.f &&
		p.y > box.y && p.y < box.y + size.y
		);
}

bool boxColideBlock(glm::dvec3 p1, glm::vec3 s1, glm::ivec3 b)
{
	const float BLOCK_SIZE = 1.f;
	return boxColide(p1, s1, {b.x,b.y - BLOCK_SIZE / 2.f,b.z}, glm::vec3(BLOCK_SIZE));
}


bool boxColideBlockWithCollider(glm::dvec3 p1, glm::vec3 s1, glm::ivec3 b, BlockCollider collider)
{
	const float BLOCK_SIZE = 1.f;
	//b is block pos.
	return boxColide(p1, s1, glm::vec3{b.x,b.y - BLOCK_SIZE / 2.f,b.z} + collider.offset,
		collider.size);
}


bool lineIntersectBoxGetPos(glm::dvec3 start, glm::dvec3 dir, glm::dvec3 box, glm::dvec3 size,
	glm::dvec3 &outPos, float &outDist, int &outFace
)
{


	dir = glm::normalize(dir);
	glm::dvec3 planeBottom = {box};
	glm::dvec3 planeFront = {box + glm::dvec3(0,0,size.z / 2)};
	glm::dvec3 planeBack = {box + glm::dvec3(0,0,-size.z / 2)};
	glm::dvec3 planeTop = {box + glm::dvec3(0,size.y,0)};
	glm::dvec3 planeLeft = {box + glm::dvec3(-size.x / 2,0,0)};
	glm::dvec3 planeRight = {box + glm::dvec3(size.x / 2,0,0)};

	//std::cout << planeBottom.x << " " << planeBottom.y << " " << planeBottom.z << " ---- \n";
	//std::cout << start.x << " " << start.y << " " << start.z << " ---- \n";

	outDist = 99999999999999.f;
	bool retVal = 0;
	

	auto doTest = [&](glm::dvec3 plane, glm::dvec3 normal, int face)
	{
		double distance = 0;
		if (glm::intersectRayPlane(start, dir, plane, normal, distance))
		{
			if (distance >= 0)
			{
				glm::dvec3 intersectPoint = start + dir * distance;

				if (pointInsideBox(intersectPoint, box, size, 0.000001))
				{
					if (distance < outDist)
					{
						outDist = distance;
						retVal = true;
						outFace = face;
						outPos = intersectPoint;
					}

					return 1;
				}
			};
		}
		return 0;
	};

	doTest(planeFront, {0,0,1}, 0);
	doTest(planeBack, {0,0,-1}, 1);
	doTest(planeTop, {0,1,0}, 2);
	doTest(planeBottom, {0,-1,0}, 3);
	doTest(planeLeft, {-1,0,0}, 4);
	doTest(planeRight, {1,0,0}, 5);


	return retVal;

}


bool lineIntersectBox(glm::dvec3 start, glm::dvec3 dir, glm::dvec3 box, glm::dvec3 size)
{

	if (pointInsideBox(start, box, size, 0.1))
	{
		return 1;
	}


	dir = glm::normalize(dir);
	glm::dvec3 planeBottom = {box};
	glm::dvec3 planeFront = {box + glm::dvec3(0,0,size.z / 2)};
	glm::dvec3 planeBack = {box + glm::dvec3(0,0,-size.z / 2)};
	glm::dvec3 planeTop = {box + glm::dvec3(0,size.y,0)};
	glm::dvec3 planeLeft = {box + glm::dvec3(-size.x / 2,0,0)};
	glm::dvec3 planeRight = {box + glm::dvec3(size.x / 2,0,0)};

	//std::cout << planeBottom.x << " " << planeBottom.y << " " << planeBottom.z << " ---- \n";
	//std::cout << start.x << " " << start.y << " " << start.z << " ---- \n";

	auto doTest = [&](glm::dvec3 plane, glm::dvec3 normal)
	{
		double distance = 0;
		if (glm::intersectRayPlane(start, dir, plane, normal, distance))
		{
			if (distance >= 0)
			{
				glm::dvec3 intersectPoint = start + dir * distance;

				//std::cout << distance << " ";
				if (pointInsideBox(intersectPoint, box, size, 0.1))
				{
					return 1;
				}
			};
		}
		return 0;
	};

	if (doTest(planeBottom, {0,-1,0})) { return 1; };
	if (doTest(planeTop, {0,1,0})) { return 1; };
	if (doTest(planeFront, {0,0,1})) { return 1; };
	if (doTest(planeBack, {0,0,-1})) { return 1; };
	if (doTest(planeLeft, {-1,0,0})) { return 1; };
	if (doTest(planeRight, {1,0,0})) { return 1; };


	return 0;

}

bool lineIntersectBoxMaxDistance(glm::dvec3 start, glm::dvec3 dir, 
	glm::dvec3 box, glm::dvec3 size, float maxDistance)

{
	float maxColliderDimension = std::max(std::max(size.x, size.y), size.z);

	//early reject.
	if (glm::distance(start, box) > maxDistance + maxColliderDimension)
	{
		return 0;
	}


	if (pointInsideBox(start, box, size, 0.2))
	{
		return 1;
	}


	dir = glm::normalize(dir);
	glm::dvec3 planeBottom = {box};
	glm::dvec3 planeFront = {box + glm::dvec3(0,0,size.z / 2)};
	glm::dvec3 planeBack = {box + glm::dvec3(0,0,-size.z / 2)};
	glm::dvec3 planeTop = {box + glm::dvec3(0,size.y,0)};
	glm::dvec3 planeLeft = {box + glm::dvec3(-size.x / 2,0,0)};
	glm::dvec3 planeRight = {box + glm::dvec3(size.x / 2,0,0)};

	//std::cout << planeBottom.x << " " << planeBottom.y << " " << planeBottom.z << " ---- \n";
	//std::cout << start.x << " " << start.y << " " << start.z << " ---- \n";

	auto doTest = [&](glm::dvec3 plane, glm::dvec3 normal)
	{
		double distance = 0;
		if (glm::intersectRayPlane(start, dir, plane, normal, distance))
		{
			if (distance >= 0 && distance <= maxDistance)
			{
				glm::dvec3 intersectPoint = start + dir * distance;

				//std::cout << distance << " ";
				if (pointInsideBox(intersectPoint, box, size, 0.2))
				{
					return 1;
				}
			};
		}
		return 0;
	};

	if (doTest(planeBottom, {0,1,0})) { return 1; };
	if (doTest(planeTop, {0,1,0})) { return 1; };
	if (doTest(planeFront, {0,0,1})) { return 1; };
	if (doTest(planeBack, {0,0,-1})) { return 1; };
	if (doTest(planeLeft, {-1,0,0})) { return 1; };
	if (doTest(planeRight, {1,0,0})) { return 1; };

	return 0;
}




//todo implement
//glm::vec3 boxColideDistance(const glm::dvec3 &p1, const glm::vec3 &s1,
//	const glm::dvec3 &p2, const glm::vec3 &s2)
//{
//	const double delta = -0.000001;
//
//	glm::vec3 halfSize1 = s1 / 2.0f;
//	glm::vec3 halfSize2 = s2 / 2.0f;
//
//	glm::vec3 distance = glm::abs(glm::vec3(p2 - p1) - halfSize1 - halfSize2);
//	glm::vec3 overlap(0.0f);
//
//	overlap.x = std::max(0.0f, halfSize1.x + halfSize2.x - distance.x);
//	overlap.y = std::max(0.0f, halfSize1.y + halfSize2.y - distance.y);
//	overlap.z = std::max(0.0f, halfSize1.z + halfSize2.z - distance.z);
//
//	return overlap;
//}

glm::dvec3 performCollision(glm::dvec3 pos, glm::dvec3 lastPos, glm::vec3 size, glm::dvec3 delta,
	decltype(chunkGetterSignature) *chunkGetter, bool &chunkLoaded, MotionState *forces, float deltaTime,
	glm::vec3 &drag, PhysicalSettings physicalSettings)
{
	chunkLoaded = true;

	

	const float BLOCK_SIZE = 1.f;

	int minX = (pos.x - abs(delta.x) - BLOCK_SIZE - size.x / 2.f) / BLOCK_SIZE - 2;
	int maxX = ceil((pos.x + abs(delta.x) + BLOCK_SIZE + size.x/2.f) / BLOCK_SIZE) + 2;

	//int minX = pos.x - 1;
	//int maxX = pos.x + 2;

	int minY = (pos.y - abs(delta.y) - BLOCK_SIZE) / BLOCK_SIZE - 2;
	int maxY = ceil((pos.y + abs(delta.y) + BLOCK_SIZE + size.y) / BLOCK_SIZE) + 2;

	int minZ = (pos.z - abs(delta.z) - BLOCK_SIZE - size.z / 2.f) / BLOCK_SIZE - 2;
	int maxZ = ceil((pos.z + abs(delta.z) + BLOCK_SIZE + size.z/2.f) / BLOCK_SIZE) + 2;

	//int minZ = pos.z - 1;
	//int maxZ = pos.z + 2;

	minY = std::max(0, minY);
	maxY = std::min(CHUNK_HEIGHT, maxY);


		for (int x = minX; x < maxX; x++)
			for (int z = minZ; z < maxZ; z++)
			{

				auto chunkPos = fromBlockPosToChunkPos(x, z);

				auto c = chunkGetter(chunkPos);

				if (c)
				{

					for (int y = minY; y < maxY; y++)
					{
						auto blockPos = fromBlockPosToBlockPosInChunk({x,y,z});

						auto b = c->unsafeGet(blockPos.x, blockPos.y, blockPos.z);
						//todo remove later and use unsafe get
						
						if (b.isColidable())
						{
							float friction = b.getFriction();
							auto collider = b.getCollider();

							auto doCollision = [&](BlockCollider collider) -> bool
							{
								if (boxColideBlockWithCollider(pos, size, {x,y,z}, collider))
								{

									if (!boxColideBlockWithCollider(lastPos, size, {x,y,z}, collider))
									{
										if (delta.x != 0)
										{
											if (forces)
											{
												drag.y = std::max(friction * physicalSettings.sideFriction, drag.y);
												drag.z = std::max(friction * physicalSettings.sideFriction, drag.z);

												forces->acceleration.x = 0;
												forces->velocity.x = 0;
											}

											if (delta.x < 0) // moving left
											{
												if (forces)forces->setColidesLeft(true);
												//pos.x = x * BLOCK_SIZE + BLOCK_SIZE / 2.0 + size.x / 2.0;
												pos.x = x * BLOCK_SIZE
													+ collider.size.x / 2 + collider.offset.x
													+ size.x / 2.0;
												return 1;
											}
											else
											{
												if (forces)forces->setColidesRight(true);
												//pos.x = x * BLOCK_SIZE - BLOCK_SIZE / 2.0 - size.x / 2.0;
												pos.x = x * BLOCK_SIZE
													- collider.size.x / 2 + collider.offset.x
													- size.x / 2.0;
												return 1;
											}

										}
										else if (delta.y != 0)
										{
											if (forces)
											{
												drag.x = std::max(friction, drag.x);
												drag.z = std::max(friction, drag.z);

												forces->acceleration.y = 0;
												forces->velocity.y = 0;
											}

											if (delta.y < 0) //moving down
											{
												if (forces)forces->setColidesBottom(true);
												pos.y = y * BLOCK_SIZE - (BLOCK_SIZE / 2.f) + collider.size.y
													+ collider.offset.y
													;
												return 1;
											}
											else //moving up
											{
												if (forces)forces->setColidesTop(true);
												pos.y = y * BLOCK_SIZE - BLOCK_SIZE / 2.0 - size.y
													+ collider.offset.y
													;
												return 1;
											}

										}
										else if (delta.z != 0)
										{
											if (forces)
											{
												drag.x = std::max(drag.x, friction * physicalSettings.sideFriction);
												drag.y = std::max(drag.y, friction * physicalSettings.sideFriction);

												forces->acceleration.z = 0;
												forces->velocity.z = 0;
											}

											//std::cout << "Yes ";

											if (delta.z < 0) // moving back
											{
												if (forces)forces->setColidesFront(true);
												pos.z = z * BLOCK_SIZE
													+ (collider.size.z / 2) + collider.offset.z
													+ size.z / 2.0;
												return 1;
											}
											else
											{
												if (forces)forces->setColidesBack(true);
												pos.z = z * BLOCK_SIZE
													- (collider.size.z / 2) + collider.offset.z
													- size.z / 2.0;
												return 1;
											}


										}
									}


								};

								return 0;
							};

							bool rez = 0;

							rez = doCollision(collider);

							if (b.hasSecondCollider())
							{
								rez |= doCollision(b.getSecondCollider());
							}

							if (rez)
							{
								goto end;
							}

						}

					}

				}
				else
				{
					chunkLoaded = false;
				}
				

			}

end:
	
		return pos;
}


void updateForces(glm::dvec3 &pos, MotionState &forces, float deltaTime, bool applyGravity, PhysicalSettings physicalSettings)
{
	updateForces(pos, forces.velocity, forces.acceleration, deltaTime, applyGravity, physicalSettings);
}

//todo right implementation here...
void updateForces(glm::dvec3 &pos, glm::vec3 &velocity, glm::vec3 &acceleration,
	float deltaTime, bool applyGravity, PhysicalSettings physicalSettings)
{
	if (applyGravity)
	{
		acceleration += glm::vec3(0, GRAVITY * physicalSettings.gravityModifier, 0);
	}

	acceleration = glm::clamp(acceleration, glm::vec3(-MAX_ACCELERATION), glm::vec3(MAX_ACCELERATION));

	velocity += acceleration * deltaTime;
	velocity = glm::clamp(velocity, glm::vec3(-MAX_VELOCITY), glm::vec3(MAX_VELOCITY));

	pos += velocity * deltaTime;

	glm::vec3 dragForce = AIR_DRAG_COEFICIENT * -velocity * glm::abs(velocity) / 2.f;

	float length = glm::length(dragForce);

	if (length)
	{
		if (length > MAX_AIR_DRAG)
		{
			dragForce /= length;
			dragForce *= MAX_AIR_DRAG;
		}

		applyDrag(velocity, dragForce * deltaTime);

	}


	acceleration = {};
}

void applyImpulse(MotionState &force, glm::vec3 impulse, float mass)
{
	force.velocity += impulse * mass;
}

void MotionState::jump(float impulse)
{
	if (colidesBottom())
	{
		applyImpulse(*this, glm::vec3{0,impulse,0});
		setColidesBottom(false);
	}
}


void colideWithOthers(glm::dvec3 &pos, glm::vec3 collider, MotionState &forces, std::vector<ColidableEntry> &others)
{

	for (auto &o : others)
	{
		glm::vec3 delta = pos - o.position;

		delta.y *= OTHERS_PUSHING_YOU_BUAS_Y_DOWN;

		float l = glm::length(delta);

		if (l == 0)
		{
			delta = {0.5,0,0.5};
		}
		else
		{
			delta /= l;
		}

		delta *= OTHERS_PUSHING_YOU_FORCE;// *15;
		//delta.y *= 0.001;

		applyImpulse(forces, delta);
	}

}