#include "gameplay/physics.h"
#include <glm/glm.hpp>
#include <chunkSystem.h>


void RigidBody::resolveConstrains(decltype(chunkGetterSignature) *chunkGetter)
{

	float distance = glm::length(lastPos - pos);
	const float BLOCK_SIZE = 1;

	if (distance < BLOCK_SIZE)
	{
		checkCollisionBrute(pos,
			lastPos,
			chunkGetter
		);
	}
	else
	{
		glm::dvec3 newPos = lastPos;
		glm::vec3 delta = pos - lastPos;
		delta = glm::normalize(delta);
		delta *= 0.9 * BLOCK_SIZE;

		do
		{
			newPos += delta;
			glm::dvec3 posTest = newPos;
			checkCollisionBrute(newPos,
				lastPos, chunkGetter
				);

			if (newPos != posTest)
			{
				pos = newPos;
				goto end;
			}

		} while (glm::length((newPos + glm::dvec3(delta)) - pos) > 1.0f * BLOCK_SIZE);

		checkCollisionBrute(pos,
			lastPos,
			chunkGetter);
	}

end:

	//clamp the box if needed
	//if (pos.x < 0) { pos.x = 0; }
	//if (pos.x + dimensions.x > (mapData.w) * BLOCK_SIZE) { pos.x = ((mapData.w) * BLOCK_SIZE) - dimensions.x; }
	void;



}

void RigidBody::checkCollisionBrute(glm::dvec3 &pos, glm::dvec3 lastPos,
	decltype(chunkGetterSignature) *chunkGetter)
{
	glm::vec3 delta = pos - lastPos;
	const float BLOCK_SIZE = 1;

	//todo
	//if (
	//	(pos.y < -dimensions.y)
	//	|| (pos.x < -dimensions.x)
	//	|| (pos.y > mapData.h * BLOCK_SIZE)
	//	|| (pos.x > mapData.w * BLOCK_SIZE)
	//	)
	//{
	//	return;
	//}


	glm::vec3 newPos = performCollision({pos.x, lastPos.y, lastPos.z}, colliderSize, {delta.x, 0, 0},
		chunkGetter);

	newPos = performCollision({newPos.x, pos.y, lastPos.z}, colliderSize, {0, delta.y, 0},
		chunkGetter);
	
	//pos.x = newPos.x;
	//pos.y = newPos.y;
	//
	
	pos = performCollision({newPos.x, newPos.y, pos.z}, colliderSize, {0, 0, delta.z},
		chunkGetter);

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

//todo add a delta
bool boxColide(glm::dvec3 p1, glm::vec3 s1,
	glm::dvec3 p2, glm::vec3 s2)
{

	if (
		(p1.x + s1.x / 2.0 > p2.x - s2.x / 2.0) &&
		(p1.x - s1.x / 2.0 < p2.x + s2.x / 2.0) &&

		(p1.y + s1.y / 2.0 > p2.y - s2.y / 2.0) &&
		(p1.y - s1.y / 2.0 < p2.y + s2.y / 2.0) &&

		(p1.z + s1.z / 2.0 > p2.z - s2.z / 2.0) &&
		(p1.z - s1.z / 2.0 < p2.z + s2.z / 2.0)
		)
	{
		return true;
	}

}

glm::dvec3 RigidBody::performCollision(glm::dvec3 pos, glm::vec3 size, glm::vec3 delta,
	decltype(chunkGetterSignature) *chunkGetter)
{

	const float BLOCK_SIZE = 1.f;

	int minX = (pos.x - abs(delta.x) - BLOCK_SIZE) / BLOCK_SIZE;
	int maxX = ceil((pos.x + abs(delta.x) + BLOCK_SIZE + size.x) / BLOCK_SIZE);

	int minY = (pos.y - abs(delta.y) - BLOCK_SIZE) / BLOCK_SIZE;
	int maxY = ceil((pos.y + abs(delta.y) + BLOCK_SIZE + size.y) / BLOCK_SIZE);

	int minZ = (pos.z - abs(delta.z) - BLOCK_SIZE) / BLOCK_SIZE;
	int maxZ = ceil((pos.z + abs(delta.z) + BLOCK_SIZE + size.z) / BLOCK_SIZE);

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
						assert(b); //todo remove later and use unsafe get
						
						if (b->isColidable())
						{

							if (boxColide(pos, size, {x,y,z}, glm::vec3(BLOCK_SIZE)))
							{

								if (delta.x != 0)
								{
									if (delta.x < 0) // moving left
									{
										//leftTouch = 1;
										pos.x = x * BLOCK_SIZE + BLOCK_SIZE / 2.f + size.x / 2.f;
										goto end;
									}
									else
									{
										//rightTouch = 1;
										pos.x = x * BLOCK_SIZE - BLOCK_SIZE / 2.f - size.x / 2.f;
										goto end;
									}
								}
								else if(delta.y != 0)
								{
									if (delta.y < 0) //moving down
									{
										pos.y = y * BLOCK_SIZE + BLOCK_SIZE / 2.f + size.y / 2.f;
										goto end;
									}
									else
									{
										pos.y = y * BLOCK_SIZE - BLOCK_SIZE / 2.f - size.y / 2.f;
										goto end;
									}
								}
								else
								{
									if (delta.z < 0) // moving left
									{
										//leftTouch = 1;
										pos.z = z * BLOCK_SIZE + BLOCK_SIZE / 2.f + size.z / 2.f;
										goto end;
									}
									else
									{
										//rightTouch = 1;
										pos.z = z * BLOCK_SIZE - BLOCK_SIZE / 2.f - size.z / 2.f;
										goto end;
									}
								}



							};

						}

					}

				}
				else
				{
					//todo freeze player if he is outside the chunk
				}
				

			}
end:
	return pos;
}

void Player::moveFPS(glm::vec3 direction)
{
	lookDirection = glm::normalize(lookDirection);

	//forward
	float forward = -direction.z;
	float leftRight = direction.x;
	float upDown = direction.y;

	glm::vec3 move = {};

	move += glm::vec3(0, 1, 0) * upDown;
	move += glm::normalize(glm::cross(lookDirection, glm::vec3(0, 1, 0))) * leftRight;
	move += lookDirection * forward;

	this->body.pos += move;
}

void RigidBody::updateMove()
{
	lastPos = pos;
}
