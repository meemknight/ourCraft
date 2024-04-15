#pragma once 
#include <gameplay/physics.h>
#include <glm/glm.hpp>



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




//todo move and change
constexpr static int simulationDistance = 6;


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
		RigidBody body;
		body.pos = position;
		body.lastPos = lastPosition;
		body.resolveConstrains(chunkGetter, &forces, deltaTime, colliderSize);

		lastPosition = body.pos;
		position = body.pos;
	}


};


template <class T>
struct ServerEntity
{
	T entity = {};
	float restantTime = 0; //todo not used for many things so move

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


