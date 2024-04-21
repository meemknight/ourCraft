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

	void computeRubberBand(glm::dvec3 &position, float deltaTime);

	void add(glm::dvec3 direction) { this->direction += direction; initialSize = glm::length(direction); }
};

void computeRubberBand(
	RubberBand &rubberBand,
	glm::dvec3 &position, float deltaTime);




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

	glm::dvec3 getRubberBandPosition()
	{
		return rubberBand.direction + entity.position;
	}

	glm::dvec3 &getPosition()
	{
		return entity.position;
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