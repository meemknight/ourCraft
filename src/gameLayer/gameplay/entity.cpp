#include <gameplay/entity.h>
#include <glm/glm.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <multyPlayer/serverChunkStorer.h>
#undef max
#undef min

//todo entities should freeze their state completely when chunks are missing

void appendMarker(std::ofstream &f, Marker marker)
{
	appendData(f, &marker, sizeof(Marker));
}

void appendEntityId(std::ofstream &f, std::uint64_t id)
{
	appendData(f, &id, sizeof(id));
}

void appendData(std::ofstream &f, void *data, size_t size)
{
	f.write((char*)data, size);
}

void basicEntitySave(std::ofstream &f, Marker marker, std::uint64_t id, void *data, size_t size)
{
	appendMarker(f, marker);
	appendEntityId(f, id);
	appendData(f, data, size);

}

bool readMarker(std::ifstream &f, Marker &marker)
{
	return readData(f, &marker, sizeof(Marker));
}

bool readEntityId(std::ifstream &f, std::uint64_t &id)
{
	return readData(f, &id, sizeof(id));
}

bool readData(std::ifstream &f, void *data, size_t size)
{
	f.read((char*)data, size);
	return f.gcount() == size && !f.fail();
}

void computeRubberBand(RubberBand &rubberBand, float deltaTime)
{
	//rubberBand = {};
	if (rubberBand.initialSize)
	{
		float speed = rubberBand.initialSize * deltaTime * 20.f;

		float newSize = glm::length(rubberBand.direction) - speed;

		if(newSize <= 0.01)
		{
			rubberBand = {};
		}
		else
		{
			if (newSize > 16.f)
			{
				newSize /= 2;
			}

			//std::cout << "ns: " << newSize << "\n";
			rubberBand.direction = glm::normalize(rubberBand.direction) * double(newSize);
		}
	}
	else
	{
		rubberBand = {};
	}

}

float normalizeAngle(float angle)
{
	while (angle < 0) angle += TWO_PI;
	while (angle >= TWO_PI) angle -= TWO_PI;
	return angle;
}

float shortestAngleDirection(float angleFrom, float angleTo)
{
	float diff = normalizeAngle(angleTo - angleFrom);
	if (diff > PI)
	{
		diff -= TWO_PI; // Rotate counterclockwise
	}
	return diff;
}

int getRandomNumber(std::minstd_rand &rng, int min, int max)
{
	return rng() % (max + 1 - min) + min;
}

float getRandomNumberFloat(std::minstd_rand &rng, float min, float max)
{
	return (getRandomNumber(rng, 0, 1000) / 1000.f) * (max-min) + min;
}

inline uint32_t hash(int x, int y, int z)
{
	uint32_t h = static_cast<uint32_t>(x) * 374761393 +
		static_cast<uint32_t>(y) * 668265263 +
		static_cast<uint32_t>(z) * 2147483647;
	h ^= (h >> 13);
	h *= 1274126177;
	h ^= (h >> 16);
	return h;
}

float getRandomNumberFloat(int x, int y, int z, float a, float b)
{
	std::minstd_rand rng;
	rng.seed(hash(x, y, z));

	return getRandomNumberFloat(rng, a, b);
}

void RubberBand::computeRubberBand(float deltaTime)
{
	::computeRubberBand(*this, deltaTime);
}


glm::vec2 getRandomUnitVector(std::minstd_rand &rng)
{
	// Generate a random angle in radians
	std::uniform_real_distribution<float> dist(0.0f, 2.0f * 3.14159265359);
	float angle = dist(rng);

	// Use trigonometric functions to calculate the components of the unit vector
	float x = std::sin(angle);
	float y = std::cos(angle);

	// Return the resulting unit vector
	return glm::normalize(glm::vec2(x, y));
}

glm::vec3 getRandomUnitVector3(std::minstd_rand &rng)
{
	// Generate two random angles in radians
	std::uniform_real_distribution<float> dist(0.0f, 2.0f * 3.14159265359f);
	float theta = dist(rng);  // azimuthal angle
	float phi = dist(rng);    // polar angle

	// Convert spherical coordinates to Cartesian coordinates
	float x = std::sin(phi) * std::cos(theta);
	float y = std::sin(phi) * std::sin(theta);
	float z = std::cos(phi);

	// Return the resulting unit vector
	return glm::normalize(glm::vec3(x, y, z));
}

glm::vec3 getRandomUnitVector3Oriented(std::minstd_rand &rng, glm::vec3 targetDirection, float maxAngle)
{

	// Generate a random axis perpendicular to the target direction
	glm::vec3 randomAxis = glm::normalize(glm::cross(targetDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
	if (glm::length(randomAxis) < 0.1f)
	{
		randomAxis = glm::normalize(glm::cross(targetDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	// Generate a random angle within the range [-maxAngle, maxAngle]
	std::uniform_real_distribution<float> dist(-maxAngle, maxAngle);
	std::uniform_real_distribution<float> dist2(0, 3.14159265359*2);
	float angle1 = dist(rng);

	// Rotate the target direction by the random angle around the random axis
	glm::quat rotation1 = glm::angleAxis(angle1, randomAxis);
	glm::vec3 rotatedDirection = glm::rotate(rotation1, targetDirection);

	// Generate a random angle between 0 and 360 degrees
	float angle2 = dist2(rng);

	// Rotate the direction around the initial direction by the random angle
	glm::quat rotation2 = glm::angleAxis(angle2, targetDirection);
	rotatedDirection = glm::rotate(rotation2, rotatedDirection);

	return glm::normalize(rotatedDirection);

}

void removeBodyRotationFromHead(glm::vec2 &bodyOrientation, glm::vec3 &lookDirection)
{
	float rotation = -std::atan2(bodyOrientation.y, bodyOrientation.x) - glm::radians(90.f);
	lookDirection = glm::rotateY(lookDirection, -rotation);
}

glm::vec3 computeLookDirection(glm::vec2 bodyOrientation, glm::vec3 lookDirection)
{
	float rotation = -std::atan2(bodyOrientation.y, bodyOrientation.x) - glm::radians(90.f);
	lookDirection = glm::rotateY(lookDirection, rotation);
	return lookDirection;
}


[[nodiscard]]
glm::vec3 orientVectorTowards(glm::vec3 vector, glm::vec3 target, float speed)
{
	vector = glm::normalize(vector);
	target = glm::normalize(target);

	// Compute the quaternion that rotates from vector to target
	glm::quat rotation = glm::rotation(vector, target);

	// Get the angle of rotation
	float angle = 2.0f * glm::acos(glm::clamp(rotation.w, -1.0f, 1.0f));

	// Prevent overshooting
	if (angle < speed)
		return target;

	// Perform slerp with a fraction of the full rotation (scaled by speed)
	glm::quat interpolatedRotation = glm::slerp(glm::quat(1, 0, 0, 0), rotation, speed / angle);

	// Apply the quaternion rotation to the vector
	return glm::normalize(interpolatedRotation * vector);
}

[[nodiscard]]
glm::vec3 moveVectorRandomly(glm::vec3 vector, std::minstd_rand &rng, float radiansX, float radiansY)
{
	vector = glm::normalize(vector);

	float randYaw = getRandomNumberFloat(rng, -radiansX, radiansX);
	float randPitch = getRandomNumberFloat(rng, -radiansY, radiansY);

	glm::mat3 yawRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), randYaw, glm::vec3(0, 1, 0)));
	glm::vec3 newVector = yawRotation * vector;

	glm::vec3 right = glm::normalize(glm::cross(newVector, glm::vec3(0, 1, 0)));
	glm::mat3 pitchRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), randPitch, right));
	newVector = pitchRotation * newVector;

	return glm::normalize(newVector);
}

[[nodiscard]]
glm::vec3 moveVectorRandomlyBiasKeepCenter(glm::vec3 vector, std::minstd_rand &rng, float radiansX, float radiansY)
{
	vector = glm::normalize(vector);

	float randYaw = getRandomNumberFloat(rng, -radiansX, radiansX);
	float randYaw2 = getRandomNumberFloat(rng, -radiansX, radiansX);
	float randPitch = getRandomNumberFloat(rng, -radiansY, radiansY);
	float randPitch2 = getRandomNumberFloat(rng, -radiansY, radiansY);

	if (std::abs(randYaw2) < std::abs(randYaw)) { randYaw = randYaw2; }
	if (std::abs(randPitch2) < std::abs(randPitch)) { randPitch = randPitch2; }


	glm::mat3 yawRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), randYaw, glm::vec3(0, 1, 0)));
	glm::vec3 newVector = yawRotation * vector;

	glm::vec3 right = glm::normalize(glm::cross(newVector, glm::vec3(0, 1, 0)));
	glm::mat3 pitchRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), randPitch, right));
	newVector = pitchRotation * newVector;

	return glm::normalize(newVector);
}


bool getRandomChance(std::minstd_rand &rng, float chance)
{
	float dice = getRandomNumberFloat(rng, 0.0, 1.0);
	return dice < chance;
}

void doCollisionWithOthers(glm::dvec3 &positiom, glm::vec3 colider, MotionState &forces,
	ServerChunkStorer &serverChunkStorer, std::uint64_t &yourEID)
{
	auto collisions = serverChunkStorer.getCollisionsListThatCanPush(positiom, colider, yourEID);
	colideWithOthers(positiom, colider, forces, collisions);
}

void addFear(unsigned short &current, unsigned short base, float ammount)
{
	unsigned short ammountShort = fromFloatToUShort(ammount);

	int newFear = (int)base + (int)ammountShort;
	if (newFear > USHORT_MAX)
	{
		current = USHRT_MAX;
	}
	else
	{
		current = std::max(current, (unsigned short)newFear);
	}
}

glm::vec2 fromDirectionToAngles(glm::vec3 direction)
{
	if (direction == glm::vec3(0, 1, 0))
	{
		return glm::vec2(0, 0);
	}
	else
	{
		glm::vec3 zenith(0, 1, 0);
		float zenithCos = glm::dot(zenith, direction);
		float zenithAngle = std::acos(zenithCos);

		glm::vec3 north(0, 0, -1);
		glm::vec3 projectedVector(direction.x, 0, direction.z);
		projectedVector = glm::normalize(projectedVector);

		float azmuthCos = glm::dot(north, projectedVector);
		float azmuthAngle = std::acos(azmuthCos);

		return glm::vec2(zenithAngle, azmuthAngle);
	}
}

void removeBodyRotationFromHead(glm::vec3 &lookDirection)
{
	float zenith = fromDirectionToAngles(lookDirection).x;

	lookDirection = glm::rotate(-zenith + glm::radians(90.f), glm::vec3{1,0,0}) * glm::vec4(0, 0, -1, 1);
}

void setBodyAndLookOrientation(glm::vec2 &bodyOrientation, glm::vec3 &lookDirection, glm::vec3 moveDir,
	glm::vec3 cameraLook)
{

	cameraLook = glm::normalize(cameraLook);

	//forward
	float forward = -moveDir.z;
	float leftRight = moveDir.x;
	float upDown = moveDir.y;

	glm::vec3 moveAbsolute = {};

	moveAbsolute += glm::vec3(0, 1, 0) * upDown;
	moveAbsolute += glm::normalize(glm::cross(cameraLook, glm::vec3(0, 1, 0))) * leftRight;
	moveAbsolute += cameraLook * forward;

	glm::vec2 cameraLook2(cameraLook.x, cameraLook.z);
	cameraLook2 = glm::normalize(cameraLook2);

	glm::vec2 moveDir2(moveAbsolute.x, moveAbsolute.z);

	if (glm::length(moveDir2))
	{

		moveDir2 = glm::normalize(moveDir2);


		float d = glm::dot(moveDir2, cameraLook2);

		bodyOrientation = moveDir2;

		if (d < -0.6)
		{
			bodyOrientation = -moveDir2;
		}
		else if (d > 0.6)
		{
			bodyOrientation = moveDir2;
		}
	}
	else
	{
		bodyOrientation = cameraLook2;
	}

	lookDirection = cameraLook;
	removeBodyRotationFromHead(bodyOrientation, lookDirection);

}

void PhysicalEntity::jump(float impulse)
{
	forces.jump(impulse);
}

void PhysicalEntity::move(glm::vec2 move)
{
	position.x += move.x;
	position.z += move.y;
}

void adjustVectorTowardsDirection(glm::vec3 &vector, glm::vec3 desiredDirection, float threshold)
{
	// Calculate the angle between the vector and the desired direction
	float angle = glm::acos(glm::dot(glm::normalize(vector), glm::normalize(desiredDirection)));

	// If the angle is bigger than the threshold, rotate the vector towards the desired direction
	if (angle > threshold)
	{
		// Calculate the rotation axis
		glm::vec3 rotationAxis = glm::cross(vector, desiredDirection);

		// Calculate the rotation angle to reach the threshold
		float rotationAngle = threshold - angle;

		// Rotate the vector towards the desired direction
		vector = glm::rotate(vector, -rotationAngle, glm::normalize(rotationAxis));
	}

}