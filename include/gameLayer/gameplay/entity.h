#pragma once 
#include <gameplay/physics.h>
#include <glm/glm.hpp>
#include <random>
#include <glm/gtx/rotate_vector.hpp>

//basic entity structure
//
//
//	struct Zombie: public PhysicalEntity
//	{
//		//update method needed
//		void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
//	};
//
//
//	than add the updates in do all updates in entity maanger client
//	and also in the server game tick update.
//
//



struct ServerChunkStorer;


struct RubberBand
{
	
	glm::dvec3 direction = {};
	float initialSize = 0;
	//glm::dvec3 startPosition = {};
	//glm::dvec3 position = {};
	//float timer = 0; //the timer should be one for now

	void computeRubberBand(float deltaTime);

	void addToRubberBand(glm::dvec3 direction) { this->direction += direction; initialSize = glm::length(direction); }
};

void computeRubberBand(
	RubberBand &rubberBand, float deltaTime);


struct HasOrientationAndHeadTurnDirection
{
	glm::vec2 bodyOrientation = {0,-1};
	glm::vec3 lookDirectionAnimation = {0,0,-1};
};

template <typename T, typename = void>
constexpr bool hasBodyOrientation = false;

template <typename T>
constexpr bool hasBodyOrientation<T, std::void_t<decltype(std::declval<T>().bodyOrientation)>> = true;


struct MovementSpeedForLegsAnimations
{
	float movementSpeedForLegsAnimations = 0;
};

template <typename T, typename = void>
constexpr bool hasMovementSpeedForLegsAnimations = false;

template <typename T>
constexpr bool hasMovementSpeedForLegsAnimations<T, std::void_t<decltype(std::declval<T>().movementSpeedForLegsAnimations)>> = true;


template <typename T, typename Enable = void>
struct RubberBandOrientation
{};

template <typename T>
struct RubberBandOrientation <T, std::enable_if_t<hasBodyOrientation<T>>>
{
	glm::vec2 rubberBandOrientation = {0, -1};
	glm::vec3 rubberBandLookDirectionAnimation = {0,0,-1};


	void computeRubberBandOrientation(float deltaTime, glm::vec2 bodyOrientation,
		glm::vec3 lookDirectionAnimation)
	{

		float angleBody = std::atan2(bodyOrientation.y, bodyOrientation.x);
		float angleCurrent = std::atan2(rubberBandOrientation.y, rubberBandOrientation.x);


		float rotationSpeed = 3.141592653 * deltaTime * 2.f;

		if (angleCurrent > angleBody)
		{
			angleCurrent -= rotationSpeed;
			if (angleCurrent < angleBody) { angleCurrent = angleBody; }
		}
		else if (angleCurrent < angleBody)
		{
			angleCurrent += rotationSpeed;
			if (angleCurrent > angleBody) { angleCurrent = angleBody; }
		}

		// Calculate new orientation
		rubberBandOrientation.x = std::cos(angleCurrent);
		rubberBandOrientation.y = std::sin(angleCurrent);	

		//rubberBandOrientation = glm::mix(bodyOrientation, rubberBandOrientation, 0.2);
		//rubberBandLookDirectionAnimation = glm::mix(lookDirectionAnimation, rubberBandLookDirectionAnimation, 0.2);


		//head
		float angle = glm::acos(glm::dot(lookDirectionAnimation, rubberBandLookDirectionAnimation));

		if (angle > 0)
		{

			float rotationSpeed = 3.141592653 * deltaTime * 1.f;

			float rotationAngle = rotationSpeed;

			if (rotationAngle > angle) { rubberBandLookDirectionAnimation = lookDirectionAnimation; }
			else
			{
				// Calculate the rotation axis
				glm::vec3 rotationAxis = glm::cross(rubberBandLookDirectionAnimation, lookDirectionAnimation);
				// Rotate the vector towards the desired direction
				rubberBandLookDirectionAnimation = glm::rotate(rubberBandLookDirectionAnimation, rotationAngle, glm::normalize(rotationAxis));
			}
			
		}


		rubberBandOrientation = normalize(rubberBandOrientation);
		rubberBandLookDirectionAnimation = normalize(rubberBandLookDirectionAnimation);
	}

};


template <typename T, typename Enable = void>
struct LegsAnimator
{};


template <typename T>
struct LegsAnimator <T, std::enable_if_t< hasMovementSpeedForLegsAnimations<T>>>
{

	float legAngle = 0;
	int legsAnimatorDirection = 0;

	void updateLegAngle(float deltaTime, float speed)
	{

		if (!speed)
		{
			legsAnimatorDirection = 0;
		}
		else if(!legsAnimatorDirection)
		{
			legsAnimatorDirection = 1;
		}

		if (legsAnimatorDirection == 0)
		{
			if (legAngle)
			{
				if (legAngle > 0)
				{
					legAngle -= deltaTime * 2.f;
					if (legAngle < 0)
					{
						legAngle = 0;
					}
				}
				else
				{
					legAngle += deltaTime * 2.f;
					if (legAngle > 0)
					{
						legAngle = 0;
					}
				}

			}
		}
		{
			deltaTime *= speed;

			legAngle += deltaTime * legsAnimatorDirection;

			if (legsAnimatorDirection == 1)
			{
				if (legAngle >= glm::radians(40.f))
				{
					legAngle = glm::radians(40.f);
					legsAnimatorDirection = -1;
				}
			}
			else
			{
				if (legAngle <= glm::radians(-40.f))
				{
					legAngle = glm::radians(-40.f);
					legsAnimatorDirection = 1;
				}
			}

		}

	}


};


//todo add a way to add some static collider sizes
struct PhysicalEntity
{
	glm::dvec3 position = {};
	glm::dvec3 lastPosition = {};
	MotionState forces = {};

	void updateForces(float deltaTime, bool applyGravity)
	{
		::updateForces(position, forces, deltaTime, applyGravity);
	}

	void resolveConstrainsAndUpdatePositions(
		decltype(chunkGetterSignature) *chunkGetter,
		float deltaTime, glm::vec3 colliderSize
	)
	{
		resolveConstrains(position, lastPosition, chunkGetter,
			&forces, deltaTime, colliderSize);

		lastPosition = position;
	}

	void updatePositions()
	{
		lastPosition = position;
	}

	void jump();
};



template <class T>
struct ServerEntity
{
	T entity = {};

	glm::dvec3 &getPosition()
	{
		return entity.position;
	}
};

template <class T>
struct ClientEntity
{
	T entity = {};
	RubberBand rubberBand = {};
	float restantTime = 0;

	RubberBandOrientation<T> rubberBandOrientation = {};

	LegsAnimator<T> legAnimator = {};

	glm::dvec3 getRubberBandPosition()
	{
		return rubberBand.direction + entity.position;
	}

	glm::dvec3 &getPosition()
	{
		return entity.position;
	}

	glm::vec2 getRubberBandOrientation()
	{
		if constexpr (hasBodyOrientation<T>)
		{
			return rubberBandOrientation.rubberBandOrientation;
		}
		else
		{
			return {0,-1};
		}
	}

	glm::vec3 getRubberBandLookDirection()
	{
		if constexpr (hasBodyOrientation<T>)
		{
			return rubberBandOrientation.rubberBandLookDirectionAnimation;
		}
		else
		{
			return {0,0,-1};
		}
	}

	//todo maybe an update internal here for all this components

	float getLegsAngle()
	{
		if constexpr (hasMovementSpeedForLegsAnimations<T>)
		{
			return legAnimator.legAngle;
		}
		else
		{
			return 0.f;
		}
	}
};

template <typename T, typename = void>
constexpr bool hasRestantTimer = false;

template <typename T>
constexpr bool hasRestantTimer<T, std::void_t<decltype(std::declval<T>().restantTime)>> = true;


int getRandomNumber(std::minstd_rand &rng, int min, int max);

float getRandomNumberFloat(std::minstd_rand &rng, float min, float max);

glm::vec2 getRandomUnitVector(std::minstd_rand &rng);

void setBodyAndLookOrientation(glm::vec2 &bodyOrientation, glm::vec3 &lookDirection, glm::vec3 moveDir,
	glm::vec3 cameraLook);


void removeBodyRotationFromHead(glm::vec3 &lookDirection);
void removeBodyRotationFromHead(glm::vec2 &bodyOrientation, glm::vec3 &lookDirection);

glm::vec2 getRandomUnitVector(std::minstd_rand &rng);
glm::vec3 getRandomUnitVector3(std::minstd_rand &rng);


void adjustVectorTowardsDirection(glm::vec3 &vector, glm::vec3 desiredDirection = {0,0,-1}, float threshold = glm::radians(85.f));

glm::vec3 getRandomUnitVector3Oriented(std::minstd_rand &rng, glm::vec3 targetDirection = {0,0,-1}, float maxAngle = 3.14159/3.f);
