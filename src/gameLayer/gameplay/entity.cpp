#include <gameplay/entity.h>
#include <glm/glm.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>

//todo entities should freeze their state completely when chunks are missing

void computeRubberBand(RubberBand &rubberBand, glm::dvec3 &position, float deltaTime)
{
	if (rubberBand.initialSize)
	{
		float speed = rubberBand.initialSize * deltaTime * 18.f;

		float newSize = glm::length(rubberBand.direction) - speed;

		if(newSize <= 0.01)
		{
			rubberBand = {};
		}
		else
		{
			//std::cout << "ns: " << newSize << "\n";
			rubberBand.direction = glm::normalize(rubberBand.direction) * double(newSize);
		}
	}
	else
	{
		rubberBand = {};
	}

}

int getRandomNumber(std::minstd_rand &rng, int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}

float getRandomNumberFloat(std::minstd_rand &rng, float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

void RubberBand::computeRubberBand(glm::dvec3 &position, float deltaTime)
{
	::computeRubberBand(*this, position, deltaTime);
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

void removeBodyRotationFromHead(glm::vec2 &bodyOrientation, glm::vec3 &lookDirection)
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

void PhysicalEntity::jump()
{
	if (forces.colidesBottom())
	{
		//std::cout << "Jump\n";
		applyImpulse(forces, glm::vec3{0,5,0});
	}

}
