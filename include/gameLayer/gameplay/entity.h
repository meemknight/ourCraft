#pragma once 
#include <gameplay/physics.h>
#include <glm/glm.hpp>
#include <random>


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


template <typename T, typename Enable = void>
struct RubberBandOrientation
{

	// Stub implementation for when T doesn't have bodyOrientation
};


template <typename T>
struct RubberBandOrientation <T, std::enable_if_t<hasBodyOrientation<T>>>
{
	glm::vec2 rubberBandOrientation = {0, -1};
	glm::vec3 rubberBandLookDirectionAnimation = {0,0,-1};


	void computeRubberBandOrientation(float deltaTime, glm::vec2 bodyOrientation,
		glm::vec3 lookDirectionAnimation)
	{
		rubberBandOrientation = glm::mix(bodyOrientation, rubberBandOrientation, 0.2);
		rubberBandLookDirectionAnimation = glm::mix(lookDirectionAnimation, rubberBandLookDirectionAnimation, 0.2);

		rubberBandOrientation = normalize(rubberBandOrientation);
		rubberBandLookDirectionAnimation = normalize(rubberBandLookDirectionAnimation);
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