#include "gameplay/physics.h"
#include <glm/glm.hpp>
#include <chunkSystem.h>
#include <iostream>



void applyDrag(glm::vec3 &force, glm::vec3 drag)
{
	auto lastForce = force;
	force += drag;

	if ((lastForce.x < 0 && force.x > 0) || (lastForce.x > 0 && force.x < 0)) { force.x = 0; }
	if ((lastForce.y < 0 && force.y > 0) || (lastForce.y > 0 && force.y < 0)) { force.y = 0; }
	if ((lastForce.z < 0 && force.z > 0) || (lastForce.z > 0 && force.z < 0)) { force.z = 0; }
}

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
	MotionState *forces, float deltaTime, glm::vec3 colliderSize)
{
	bool rez = 0;

	float distance = glm::length(lastPos - pos);
	const float BLOCK_SIZE = 1;

	forces->colides = 0;

	if (distance < BLOCK_SIZE)
	{
		rez = checkCollisionBrute(pos,
			lastPos,
			chunkGetter, forces, deltaTime, colliderSize
		);
	}
	else
	{
		glm::dvec3 newPos = lastPos;
		glm::vec3 delta = pos - lastPos;
		delta = glm::normalize(delta);
		delta *= 0.9 * BLOCK_SIZE;

		int hardLimit = 10000;

		do
		{
			newPos += delta;
			glm::dvec3 posTest = newPos;
			rez = checkCollisionBrute(newPos,
				lastPos, chunkGetter, forces, deltaTime, colliderSize
				);

			if (newPos != posTest)
			{
				pos = newPos;
				goto end;
			}

		} while (glm::length((newPos + glm::dvec3(delta)) - pos) > 1.0f * BLOCK_SIZE && hardLimit-- > 0);

		rez = checkCollisionBrute(pos,
			lastPos,
			chunkGetter, forces, deltaTime, colliderSize);
	}

end:






	//clamp the box if needed
	//if (pos.x < 0) { pos.x = 0; }
	//if (pos.x + dimensions.x > (mapData.w) * BLOCK_SIZE) { pos.x = ((mapData.w) * BLOCK_SIZE) - dimensions.x; }
	
	return rez;

}

bool checkCollisionBrute(glm::dvec3 &pos, glm::dvec3 lastPos,
	decltype(chunkGetterSignature) *chunkGetter, MotionState *forces, float deltaTime
	, glm::vec3 colliderSize)
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
			chunkGetter, rez, forces, deltaTime, drag).x;
	}
	else
	{
		//check for collisions
		//performCollision({pos.x, lastPos.y, lastPos.z}, lastPos, colliderSize, {0.001, 0, 0},
		//	chunkGetter, rez, forces, deltaTime, drag);
		//performCollision({pos.x, lastPos.y, lastPos.z}, lastPos, colliderSize, {-0.001, 0, 0},
		//	chunkGetter, rez, forces, deltaTime, drag);
	}

	if (delta.z)
	{
		newPos.z = performCollision({newPos.x, lastPos.y, pos.z}, lastPos, colliderSize, {0, 0, delta.z},
			chunkGetter, rez, forces, deltaTime, drag).z;
	}
	else
	{
		//check for collisions
		//performCollision({newPos.x, newPos.y, pos.z}, lastPos, colliderSize, {0, 0, 0.001},
		//	chunkGetter, rez, forces, deltaTime, drag).z;
		//performCollision({newPos.x, newPos.y, pos.z}, lastPos, colliderSize, {0, 0, -0.001},
		//	chunkGetter, rez, forces, deltaTime, drag).z;
	}

	if (delta.y)
	{
		newPos.y = performCollision({newPos.x, pos.y, newPos.z}, lastPos, colliderSize, {0, delta.y, 0},
			chunkGetter, rez, forces, deltaTime, drag).y;
	}
	

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


bool aabb(glm::vec4 b1, glm::vec4 b2)
{
	if (((b1.x - b2.x < b2.z)
		&& b2.x - b1.x < b1.z
		)
		&& ((b1.y - b2.y < b2.w)
		&& b2.y - b1.y < b1.w
		)
		)
	{
		return 1;
	}
	return 0;
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

glm::dvec3 performCollision(glm::dvec3 pos, glm::dvec3 lastPos, glm::vec3 size, glm::dvec3 delta,
	decltype(chunkGetterSignature) *chunkGetter, bool &chunkLoaded, MotionState *forces, float deltaTime,
	glm::vec3 &drag)
{
	chunkLoaded = true;

	const float DRAG_CONSTANT = 4.f;

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
	maxY = std::min(CHUNK_HEIGHT-1, maxY);


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

						auto b = c->safeGet(blockPos);
						if (!b)
						{
							std::cout << "ERROR WELP IT DIDN'T WORK\n";
						}
						//todo remove later and use unsafe get
						
						if (b->isCollidable())
						{


							if (boxColide(pos, size, {x,y - BLOCK_SIZE/2.f,z}, glm::vec3(BLOCK_SIZE)))
							{

								if (!boxColide(lastPos, size, {x,y - BLOCK_SIZE / 2.f,z}, glm::vec3(BLOCK_SIZE)))
								{
									if (delta.x != 0)
									{
										if (forces)
										{
											//drag.y = DRAG_CONSTANT;
											//drag.z = DRAG_CONSTANT;

											forces->acceleration.x = 0;
											forces->velocity.x = 0;
										}

										if (delta.x < 0) // moving left
										{
											if (forces)forces->setColidesLeft(true);
											pos.x = x * BLOCK_SIZE + BLOCK_SIZE / 2.0 + size.x / 2.0;
											goto end;
										}
										else
										{
											if (forces)forces->setColidesRight(true);
											pos.x = x * BLOCK_SIZE - BLOCK_SIZE / 2.0 - size.x / 2.0;
											goto end;
										}
										
									}
									else if (delta.y != 0)
									{
										if (forces)
										{
											drag.x = DRAG_CONSTANT;
											drag.z = DRAG_CONSTANT;

											forces->acceleration.y = 0;
											forces->velocity.y = 0;
										}

										if (delta.y < 0) //moving down
										{
											if(forces)forces->setColidesBottom(true);
											pos.y = y * BLOCK_SIZE + BLOCK_SIZE / 2.0;
											goto end;
										}
										else //moving up
										{
											if (forces)forces->setColidesTop(true);
											pos.y = y * BLOCK_SIZE - BLOCK_SIZE / 2.0 - size.y;
											goto end;
										}
										
									}
									else if (delta.z != 0)
									{
										if (forces)
										{
											//drag.x = DRAG_CONSTANT;
											//drag.y = DRAG_CONSTANT;

											forces->acceleration.z = 0;
											forces->velocity.z = 0;
										}

										if (delta.z < 0) // moving left
										{
											if (forces)forces->setColidesFront(true);
											pos.z = z * BLOCK_SIZE + BLOCK_SIZE / 2.0 + size.z / 2.0;
											goto end;
										}
										else
										{
											if (forces)forces->setColidesBack(true);
											pos.z = z * BLOCK_SIZE - BLOCK_SIZE / 2.0 - size.z / 2.0;
											goto end;
										}

										
									}
								}


							};

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


void updateForces(glm::dvec3 &pos, MotionState &forces, float deltaTime, bool applyGravity)
{
	updateForces(pos, forces.velocity, forces.acceleration, deltaTime, applyGravity);
}


void updateForces(glm::dvec3 &pos, glm::vec3 &velocity, glm::vec3 &acceleration,
	float deltaTime, bool applyGravity)
{
	if (applyGravity)
	{
		acceleration += glm::vec3(0, -9.8, 0);
	}
	acceleration = glm::clamp(acceleration, glm::vec3(-1000), glm::vec3(1000));

	velocity += acceleration * deltaTime;
	velocity = glm::clamp(velocity, glm::vec3(-1000), glm::vec3(1000));

	pos += velocity * deltaTime;

	const float drag = 0.1f;
	glm::vec3 dragForce = drag * -velocity * glm::abs(velocity) * 0.5f;

	applyDrag(velocity, dragForce * deltaTime);

	acceleration = {};
}

void applyImpulse(MotionState &force, glm::vec3 impulse, float mass)
{
	force.velocity += impulse * mass;
}

void MotionState::jump()
{
	if (colidesBottom())
	{
		applyImpulse(*this, glm::vec3{0,5,0});
		setColidesBottom(false);
	}
}
