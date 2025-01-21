#pragma once 
#define NO_MIN_MAX
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <gameplay/physics.h>
#include <glm/glm.hpp>
#include <random>
#include <glm/gtx/rotate_vector.hpp>
#include <fstream>
#include <unordered_set>
#include <rendering/camera.h>
#include <gameplay/effects.h>
#include <rendering/model.h>
#include <easing.h>


//basic entity structure
//
//
//	struct Zombie: public PhysicalEntity
//	{
//		//update method needed
//		void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
//			ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng);
// 
//		//save method needed
//		void appendDataToDisk(std::ofstream &f, std::uint64_t eId);
//	};
//
//
//	than add the updates in do all updates in entity maanger client
//	and also in the server game tick update.
//

//todo reuse entity types here?
namespace Markers
{
	enum
	{
		none = 0,
		droppedItem,


	};
};

using Marker = unsigned int;

void appendMarker(std::ofstream &f, Marker marker);

void appendEntityId(std::ofstream &f, std::uint64_t id);

void appendData(std::ofstream &f, void *data, size_t size);

void basicEntitySave(std::ofstream &f, Marker marker, std::uint64_t id, void *data, size_t size);


bool readMarker(std::ifstream &f, Marker &marker);

bool readEntityId(std::ifstream &f, std::uint64_t &id);

bool readData(std::ifstream &f, void *data, size_t size);



struct PathFindingNode
{
	glm::ivec3 returnPos = {};
	int level = 0;
};

struct PositionAndEID
{
	glm::dvec3 pos;
	std::uint64_t eid;
};


struct ServerChunkStorer;

struct RubberBand
{
	
	glm::dvec3 direction = {};
	float initialSize = 0;
	//glm::dvec3 startPosition = {};
	//glm::dvec3 position = {};
	//float timer = 0; //the timer should be one for now

	void computeRubberBand(float deltaTime);

	void addToRubberBand(glm::dvec3 direction) 
	{ 
		if (glm::length(direction) > 0.01)
		{
			this->direction += direction;
			initialSize = glm::length(direction);

			//auto copy = this->direction + direction;
			//
			//if (glm::length(copy) >= glm::length(direction))
			//{
			//	this->direction = copy;
			//	initialSize = glm::length(direction);
			//}
		}

	}
};

void computeRubberBand(
	RubberBand &rubberBand, float deltaTime);


//the pupils are actually optinal
struct HasEyesAndPupils
{
	constexpr static bool eyesAndPupils = true;
};

template <typename T, typename = void>
constexpr bool hasEyesAndPupils = false;

template <typename T>
constexpr bool hasEyesAndPupils<T, std::void_t<decltype(std::declval<T>().eyesAndPupils)>> = true;


struct HasOrientationAndHeadTurnDirection
{
	glm::vec2 bodyOrientation = {0,-1};
	glm::vec3 lookDirectionAnimation = {0,0,-1};
};

template <typename T, typename = void>
constexpr bool hasBodyOrientation = false;

template <typename T>
constexpr bool hasBodyOrientation<T, std::void_t<decltype(std::declval<T>().bodyOrientation)>> = true;

template <typename T, typename = void>
constexpr bool hasLookDirectionAnimation = false;

template <typename T>
constexpr bool hasLookDirectionAnimation <T, std::void_t<decltype(std::declval<T>().lookDirectionAnimation)>> = true;

template <typename T, typename = void>
constexpr bool hasCleanup = false;

template <typename T>
constexpr bool hasCleanup<T, std::void_t<decltype(std::declval<T>().cleanup)>> = true;


struct MovementSpeedForLegsAnimations
{
	float movementSpeedForLegsAnimations = 0;
};

template <typename T, typename = void>
constexpr bool hasMovementSpeedForLegsAnimations = false;

template <typename T>
constexpr bool hasMovementSpeedForLegsAnimations<T, std::void_t<decltype(std::declval<T>().movementSpeedForLegsAnimations)>> = true;


template <typename T, typename = void>
constexpr bool hasGetDataToSend = false;

template <typename T>
constexpr bool hasGetDataToSend<T, std::void_t<decltype(std::declval<T>().getDataToSend())>> = true;

template <typename T, typename = void>
constexpr bool hasSkinBindlessTexture = false;

template <typename T>
constexpr bool hasSkinBindlessTexture<T, std::void_t<decltype(std::declval<T>().skinBindlessTexture)>> = true;


template <typename T, typename = void>
constexpr bool hasLife = false;

template <typename T>
constexpr bool hasLife<T, std::void_t<decltype(std::declval<T>().life)>> = true;




constexpr float PI = 3.141592653f;
constexpr float TWO_PI = 2.0f * PI;

float normalizeAngle(float angle);

float shortestAngleDirection(float angleFrom, float angleTo);


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

		if (bodyOrientation == glm::vec2{0,0}) { bodyOrientation = {1,0}; }

		float angleBody = std::atan2(bodyOrientation.y, bodyOrientation.x);
		float angleCurrent = std::atan2(rubberBandOrientation.y, rubberBandOrientation.x);

		float rotationSpeed = 3.141592653 * deltaTime * 2.f;
		
		float diff = shortestAngleDirection(angleCurrent, angleBody);

		if (std::abs(diff) <= rotationSpeed)
		{
			angleCurrent = angleBody;
		}
		else
		{
			angleCurrent += (diff > 0 ? rotationSpeed : -rotationSpeed);
			angleCurrent = normalizeAngle(angleCurrent); // Normalize again after the update
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
		}else
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

	void bringBack(float speed)
	{
		legsAnimatorDirection = 0;

		if (glm::abs(legAngle) - speed < 0)
		{
			legAngle = 0;
		}
		else if (legAngle > 0)
		{
			legAngle -= speed;
		}
		else
		{
			legAngle += speed;
		}
	}

};


template <typename T, typename Enable = void>
struct EyesAndPupilsAnimator
{
};


template <typename T>
struct EyesAndPupilsAnimator <T, std::enable_if_t< hasEyesAndPupils<T>>>
{

	EasingEngine pupilAnimation;
	float pupilWaitTimer = 2;

	EasingEngine eyeLookingDirectionAnimator;
	EasingEngine eyeLookingDirectionUpDown;
	float eyeUpDownTimer = 2;


	EasingEngine eyeLookingDirectionLeftRight;
	float eyeLeftRightTimer = 0;
	float pupilDirection = 0;

};



struct PhysicalEntity
{
	glm::dvec3 position = {};
	glm::dvec3 lastPosition = {};
	MotionState forces = {};

	void updateForces(float deltaTime, bool applyGravity, PhysicalSettings physicalSettings = {})
	{
		::updateForces(position, forces, deltaTime, applyGravity, physicalSettings);
	}

	void resolveConstrainsAndUpdatePositions(
		decltype(chunkGetterSignature) *chunkGetter,
		float deltaTime, glm::vec3 colliderSize, PhysicalSettings physicalSettings = {}
	)
	{
		resolveConstrains(position, lastPosition, chunkGetter,
			&forces, deltaTime, colliderSize, physicalSettings);

		lastPosition = position;
	}

	void updatePositions()
	{
		lastPosition = position;
	}

	void jump(float impulse = BASIC_JUMP_IMPULSE);
};

struct CanPushOthers
{
	constexpr static bool canPushOthers = true;
};

template <typename T, typename = void>
constexpr bool hasCanPushOthers = false;

template <typename T>
constexpr bool hasCanPushOthers<T, std::void_t<decltype(T::canPushOthers)>> = true;


template <typename T, typename = void>
constexpr bool hasForces = false;

template <typename T>
constexpr bool hasForces<T, std::void_t<decltype(T::forces)>> = true;



struct CollidesWithPlacedBlocks
{
	constexpr static bool collidesWithPlacedBlocks = true;
};

template <typename T, typename = void>
constexpr bool hasCollidesWithPlacedBlocks = false;

template <typename T>
constexpr bool hasCollidesWithPlacedBlocks<T, std::void_t<decltype(T::collidesWithPlacedBlocks)>> = true;


//todo probably remove this one and just use life or something
struct CanBeKilled
{
	constexpr static bool canBeKilled = true;
};

template <typename T, typename = void>
constexpr bool hasCanBeKilled = false;

template <typename T>
constexpr bool hasCanBeKilled <T, std::void_t<decltype(T::canBeKilled)>> = true;


struct CanBeAttacked
{
	constexpr static bool canBeAttacked = true;
};
template <typename T, typename = void>
constexpr bool hasCanBeAttacked = false;
template <typename T>
constexpr bool hasCanBeAttacked <T, std::void_t<decltype(T::canBeAttacked)>> = true;


struct CanHaveEffects
{
	constexpr static bool canHaveEffects = true;
};
template <typename T, typename = void>
constexpr bool hasCanHaveEffects = false;
template <typename T>
constexpr bool hasCanHaveEffects <T, std::void_t<decltype(T::canHaveEffects)>> = true;



template<bool B, typename T>
using ConditionalMember = typename std::conditional<B, T, unsigned char>::type;



template<class T>
struct StaticCircularBufferForBuffering
{

	constexpr static int BUFFER_CAPACITY = 20;

	T elements[BUFFER_CAPACITY] = {};
	std::uint64_t timeAdded[BUFFER_CAPACITY] = {};

	unsigned char startIndex = 0;
	unsigned char count = 0;

	void addElement(T &element, std::uint64_t time)
	{

		if (count < BUFFER_CAPACITY)
		{
			int newPosition = startIndex + count;
			newPosition %= BUFFER_CAPACITY;
			elements[newPosition] = element;
			timeAdded[newPosition] = time;
			count++;
		}
		else
		{
			elements[startIndex] = element;
			timeAdded[startIndex] = time;
			startIndex++;
			startIndex = startIndex % BUFFER_CAPACITY;
		}
	
	}

	void removeLastElement() 
	{
		
		if (count > 0)
		{
			count--;

			startIndex++;
			startIndex = startIndex % BUFFER_CAPACITY;
		}
	}

	T &getNewestElementReff()
	{
		//todo add perma assert back here
		if(count == 0)
		{
			std::cout << "circular buffer buffer underflow !!!!!!!!!!!!";
			exit(0);
		}
		//permaAssertComment(count != 0, "circular buffer buffer underflow");
		int index = startIndex + count - 1;
		if (index < 0) { index = BUFFER_CAPACITY - 1; }
		index = index % BUFFER_CAPACITY;

		return elements[index];
	}

};




//dropped item doesn't inherit from this class, so if you want
//to add something here make another type of server entity.
template <class T>
struct ServerEntity
{
	T entity = {};

	glm::dvec3 &getPosition()
	{
		return entity.position;
	}

	//knock back, this does not does any calculations.
	void applyHitForce(glm::vec3 force)
	{
		if constexpr(hasForces<T>)
		{
			entity.forces.velocity += force;
		}
	}

	constexpr static bool hasCanHaveEffects()
	{
		return ::hasCanHaveEffects<T>;
	}

	ConditionalMember<hasCanHaveEffects(), Effects> effects = {};

	bool hasUpdatedThisTick = 0;
	glm::ivec2 lastChunkPositionWhenAnUpdateWasSent = {};
};

template <class T, class BASE_CLIENT>
struct ClientEntity
{

	StaticCircularBufferForBuffering<T> bufferedEntityData;

	T entityBuffered = {};
	RubberBand rubberBand = {};
	glm::dvec3 oldPositionForRubberBand = {};

	float restantTime = 0;

	RubberBandOrientation<T> rubberBandOrientation = {};

	LegsAnimator<T> legAnimator = {};
	EyesAndPupilsAnimator<T> eyesAndPupilsAnimator = {};

	ConditionalMember<hasCanBeKilled<T>, bool> wasKilled = 0;
	ConditionalMember<hasCanBeKilled<T>, float> wasKilledTimer = 0;


	glm::dvec3 getRubberBandPosition()
	{
		return rubberBand.direction + entityBuffered.position;
	}

	glm::dvec3 &getPosition()
	{
		return entityBuffered.position;
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

	void flushCircularBuffer()
	{
		if (bufferedEntityData.count)
		{
			entityBuffered = bufferedEntityData.getNewestElementReff();

			bufferedEntityData.count = 0;
			bufferedEntityData.startIndex = 0;
		}

	}

	void setEntityMatrixFull(glm::mat4 *skinningMatrix, Model &model, float deltaTime,
		std::minstd_rand &rng)
	{

	

		//if(0)
		if constexpr (hasLookDirectionAnimation<T>)
		{
			auto rotation = glm::toMat4(glm::quatLookAt(glm::normalize(getRubberBandLookDirection()), glm::vec3(0, 1, 0)));

			if constexpr (hasEyesAndPupils<T>)
			{
				if (model.pupilsIndex > -1)
					{
						
						skinningMatrix[model.pupilsIndex] = skinningMatrix[model.pupilsIndex] * rotation; 
				
					}

				if (model.lEyeIndex > -1)
					{ skinningMatrix[model.lEyeIndex] = skinningMatrix[model.lEyeIndex] * rotation; }

				if (model.rEyeIndex > -1)
					{ skinningMatrix[model.rEyeIndex] = skinningMatrix[model.rEyeIndex] * rotation; }
			}

			if (model.headIndex > -1)
			{
				skinningMatrix[model.headIndex] = skinningMatrix[model.headIndex] * rotation;
			}

		};


		if constexpr (hasEyesAndPupils<T>)
		{
			int stuff = 0;

			if (model.pupilsIndex > -1)
			{

				float pupilDisplacement = eyesAndPupilsAnimator.pupilAnimation.update(deltaTime)
					.sin(eyesAndPupilsAnimator.pupilWaitTimer, 2, 0.01, 0.10).burst(0.5, 0, 1, 8)
					.clamp().result();

				if (eyesAndPupilsAnimator.pupilAnimation.hasRestarted)
				{
					eyesAndPupilsAnimator.pupilWaitTimer =
						getRandomNumberFloat(rng, 0.4, 6) +
						getRandomNumberFloat(rng, 0.1, 4);

				}

				pupilDisplacement *= 3.9f * -(1.f / 16.f);

				skinningMatrix[model.pupilsIndex] =
					skinningMatrix[model.pupilsIndex] *
					glm::translate(glm::vec3(0, pupilDisplacement, 0));
				;
			}

			if (model.lEyeIndex > -1 ||
				model.rEyeIndex > -1
				)
			{

				float pupilUpDown = eyesAndPupilsAnimator.eyeLookingDirectionUpDown.update(deltaTime)
					.constant(eyesAndPupilsAnimator.eyeUpDownTimer)
					.sin(5, 1, -1, 1)
					.goTowards(1).result();

				pupilUpDown *= (1.f / 16.f) * 0.1;
				if (eyesAndPupilsAnimator.eyeLookingDirectionUpDown.hasRestarted)
				{
					eyesAndPupilsAnimator.eyeUpDownTimer =
						getRandomNumberFloat(rng, 0.4, 10);
				}


				float pupilLeftRight = eyesAndPupilsAnimator.eyeLookingDirectionLeftRight.update(deltaTime)
					.constant(eyesAndPupilsAnimator.eyeLeftRightTimer)
					.goTowards(0.5, 1)
					.constant(2)
					.goTowards(0.8).result();
				if (eyesAndPupilsAnimator.eyeLookingDirectionLeftRight.hasRestarted)
				{
					eyesAndPupilsAnimator.eyeLeftRightTimer =
						getRandomNumberFloat(rng, 1, 12);
					eyesAndPupilsAnimator.pupilDirection =
						std::sqrt(getRandomNumberFloat(rng, 0, 1));
					if (getRandomChance(rng, 0.5))
					{
						eyesAndPupilsAnimator.pupilDirection *= -1;
					}

				}

				float rEye = 0;
				float lEye = 0;
				pupilLeftRight *= (1.f / 16.f) * 0.6f;


				if (eyesAndPupilsAnimator.pupilDirection > 0)
				{
					rEye = pupilLeftRight * eyesAndPupilsAnimator.pupilDirection;
					lEye = rEye * 0.4;
				}
				else
				{
					lEye = pupilLeftRight * eyesAndPupilsAnimator.pupilDirection;
					rEye = lEye * 0.4;
				}

				rEye += 0.01;
				lEye += -0.01;

				if (model.lEyeIndex)
				{
					skinningMatrix[model.lEyeIndex] =
						skinningMatrix[model.lEyeIndex] *
						glm::translate(glm::vec3(lEye, pupilUpDown, 0));
				}

				if (model.rEyeIndex)
				{
					skinningMatrix[model.rEyeIndex] =
						skinningMatrix[model.rEyeIndex] *
						glm::translate(glm::vec3(rEye, pupilUpDown, 0));
				}

			}


		}


		BASE_CLIENT *baseClient = (BASE_CLIENT *)this;
		baseClient->setEntityMatrix(skinningMatrix);
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

	glm::mat4 getBodyRotationMatrix()
	{

		auto orient = getRubberBandOrientation();
		auto rotMatrix = glm::rotate(-std::atan2(orient.y, orient.x)
			- glm::radians(90.f),
			glm::vec3(0, 1, 0));
		return rotMatrix;

	}

	void clientEntityUpdate(float deltaTime, ChunkData *(chunkGetter)(glm::ivec2),
		std::uint64_t serverTimer)
	{

	#pragma region compute buffering		
		{
			if (bufferedEntityData.count)
			{
				
				if (bufferedEntityData.count > 1)
				{
					float interpolator = 1;
					int timeDelay = 4000;
					int elementStart = bufferedEntityData.startIndex;
					int nextElement = elementStart;
					for (int i = 0; i < bufferedEntityData.count - 1; i++)
					{

						nextElement = elementStart;
						nextElement++;
						nextElement %= bufferedEntityData.BUFFER_CAPACITY;


						std::uint64_t time = bufferedEntityData.timeAdded[elementStart];
						std::uint64_t time2 = bufferedEntityData.timeAdded[nextElement];
						std::uint64_t myTimer = 0;
						if (serverTimer > timeDelay) { myTimer -= timeDelay; }

						if (myTimer >= time && myTimer <= time2 && time <= time2)
						{
							interpolator = 1;
							if (time != time2)
							{
								interpolator = (myTimer - time) /  float(time2 - time);
							}
							
							break;
						}

						if (i == bufferedEntityData.count - 2)
						{
							break;
						}

						elementStart++;
						elementStart %= bufferedEntityData.BUFFER_CAPACITY;
					}

					T &entity1 = bufferedEntityData.elements[elementStart];
					T &entity2 = bufferedEntityData.elements[nextElement];

					entityBuffered = entity1;

					entityBuffered.position = lerp(entityBuffered.position, entity2.position, interpolator);

				}
				else
				{
					entityBuffered = bufferedEntityData.getNewestElementReff();

				}



			}

		}
	#pragma endregion



		BASE_CLIENT *baseClient = (BASE_CLIENT *)this;
		//baseClient->rubberBand = {};

		if (restantTime < 0)
		{
			float timer = deltaTime + restantTime;
			if (timer > 0)
			{
				auto oldPosition = baseClient->oldPositionForRubberBand;
				//baseClient->rubberBand.addToRubberBand(oldPosition - baseClient->getPosition());
				
				//auto oldPosition = baseClient->getPosition();
				baseClient->update(timer, chunkGetter);
				//auto newPosition = baseClient->getPosition();

			}
		}
		else
		{
			if (restantTime > 0)
			{
				//baseClient->update(restantTime, chunkGetter);
				//auto oldPosition = baseClient->oldPositionForRubberBand;
				//
				////baseClient->rubberBand.addToRubberBand(oldPosition - baseClient->getPosition());
				//
				//baseClient->update(deltaTime, chunkGetter);

				baseClient->update(deltaTime + restantTime, chunkGetter);

			}
			else
			{
				baseClient->update(deltaTime, chunkGetter);
			}


		}

		if (rubberBand.initialSize)
		{
			rubberBand.computeRubberBand(deltaTime * 1);
		}

		if constexpr (hasBodyOrientation<T>)
		{
			if(!wasKilled)
				rubberBandOrientation.computeRubberBandOrientation(deltaTime,
				entityBuffered.bodyOrientation,
				entityBuffered.lookDirectionAnimation);
		}


		if constexpr (hasCanBeKilled<T> && hasMovementSpeedForLegsAnimations<T>)
		{
			if (wasKilled)
			{
				legAnimator.bringBack(deltaTime);
			}
			else
			{

				legAnimator.updateLegAngle(deltaTime, entityBuffered.
					movementSpeedForLegsAnimations);
			}
		}else
		if constexpr (hasMovementSpeedForLegsAnimations<T>)
		{
			legAnimator.updateLegAngle(deltaTime, entityBuffered.
				movementSpeedForLegsAnimations);
		}

		restantTime = 0;

	};

};


template <typename T, typename = void>
constexpr bool hasRestantTimer = false;
template <typename T>
constexpr bool hasRestantTimer<T, std::void_t<decltype(std::declval<T>().restantTime)>> = true;


int getRandomNumber(std::minstd_rand &rng, int min, int max);

float getRandomNumberFloat(std::minstd_rand &rng, float min, float max);

bool getRandomChance(std::minstd_rand &rng, float chance);

glm::vec2 getRandomUnitVector(std::minstd_rand &rng);

void setBodyAndLookOrientation(glm::vec2 &bodyOrientation, glm::vec3 &lookDirection, glm::vec3 moveDir,
	glm::vec3 cameraLook);


void removeBodyRotationFromHead(glm::vec3 &lookDirection);
void removeBodyRotationFromHead(glm::vec2 &bodyOrientation, glm::vec3 &lookDirection);

glm::vec2 getRandomUnitVector(std::minstd_rand &rng);
glm::vec3 getRandomUnitVector3(std::minstd_rand &rng);


void adjustVectorTowardsDirection(glm::vec3 &vector, glm::vec3 desiredDirection = {0,0,-1}, float threshold = glm::radians(85.f));

glm::vec3 getRandomUnitVector3Oriented(std::minstd_rand &rng, glm::vec3 targetDirection = {0,0,-1}, float maxAngle = 3.14159/3.f);

constexpr static unsigned short USHORT_MAX = 65'535;

constexpr float fromUchartoFloat(unsigned char a)
{
	return (a) / 255.f;
}

constexpr float fromUShortToFloat(unsigned short a)
{
	return (a) / (float)USHORT_MAX;
}

constexpr unsigned short fromFloatToUShort(float a)
{
	static_assert(sizeof(unsigned short) == 2); //well if you have an error here I'm sorry for you :)), replace unsigned shorts with std::uint16_t;

	a = std::clamp(a, 0.f, 1.f);
	return a * (float)USHORT_MAX;
}

constexpr unsigned char fromFloattoUchar(float a)
{
	a = std::clamp(a, 0.f, 1.f);
	return a * 255;
}

void addFear(unsigned short &current, unsigned short base, float ammount);

struct ServerChunkStorer;

void doCollisionWithOthers(glm::dvec3 &positiom, glm::vec3 colider, 
	MotionState &forces, ServerChunkStorer &serverChunkStorer, std::uint64_t &yourEID);