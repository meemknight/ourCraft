#pragma once 
#define NO_MIN_MAX
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <gameplay/physics.h>
#include <glm/glm.hpp>
#include <random>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <rendering/camera.h>
#include <gameplay/effects.h>
#include <rendering/model.h>
#include <easing.h>
#include <gameplay/weaponStats.h>

struct Client;

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
			initialSize = glm::length(this->direction);

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


//the pupils are actually optinal, this will add the animation logic for the eyes and pupils.
//they will work if the model has them, if else, it won't do anything.

#define EYE_ANIMATION_TYPE_NORMAL 1
#define EYE_ANIMATION_TYPE_PLAYER 2

template <unsigned char EYE_ANIMATION_TYPE = 1>
struct HasEyesAndPupils
{
	constexpr static unsigned char eyesAndPupils = EYE_ANIMATION_TYPE;
	//1 means normal
	//2 means player style
};

template <typename T, typename = void>
constexpr bool hasEyesAndPupils = false;

template <typename T>
constexpr bool hasEyesAndPupils<T, std::void_t<decltype(std::declval<T>().eyesAndPupils)>> = true;


//takes the look direction that is relative to body orientation and the body orientation to compute the real look direction
[[nodiscard]]
glm::vec3 computeLookDirection(glm::vec2 bodyOrientation, glm::vec3 lookDirection);

[[nodiscard]]
glm::vec3 moveVectorRandomly(glm::vec3 vector, std::minstd_rand &rng, float radiansX, float radiansY);

[[nodiscard]]
glm::vec3 moveVectorRandomlyBiasKeepCenter(glm::vec3 vector, std::minstd_rand &rng, float radiansX, float radiansY);

[[nodiscard]]
glm::vec3 orientVectorTowards(glm::vec3 vector, glm::vec3 target, float speed);


struct HasOrientationAndHeadTurnDirection
{
	glm::vec2 bodyOrientation = {0,-1};
	
	//the look direction animation is relative to body orientatio!
	glm::vec3 lookDirectionAnimation = {0,0,-1};

	glm::vec3 getLookDirection()
	{
		glm::vec3 realLookDirection = glm::normalize(computeLookDirection(bodyOrientation, lookDirectionAnimation));
		return realLookDirection;
	}

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

//todo remove in the future
template <typename T, typename = void>
constexpr bool hasSkinBindlessTexture = false;

template <typename T>
constexpr bool hasSkinBindlessTexture<T, std::void_t<decltype(std::declval<T>().skinBindlessTexture)>> = true;


#pragma region has life
	// Check if a type has a member named `life`
	template <typename T, typename = void>
	struct HasLifeMember: std::false_type {};
	
	template <typename T>
	struct HasLifeMember<T, std::void_t<decltype(std::declval<T>().life)>>: std::true_type {};
	
	// Check if a type has a member named `newLife`
	template <typename T, typename = void>
	struct HasNewLifeMember: std::false_type {};
	
	template <typename T>
	struct HasNewLifeMember<T, std::void_t<decltype(std::declval<T>().newLife)>>: std::true_type {};
	
	// Combine both checks: true if either `life` or `newLife` exists
	template <typename T>
	constexpr bool hasLife = std::disjunction_v<HasLifeMember<T>, HasNewLifeMember<T>>;
#pragma endregion



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

	void move(glm::vec2 move);

	void moveDynamic(glm::vec2 move, float deltaTime);
};

struct CanPushOthers { constexpr static bool canPushOthers = true; };
template <typename T, typename = void>
constexpr bool hasCanPushOthers = false;
template <typename T>
constexpr bool hasCanPushOthers<T, std::void_t<decltype(T::canPushOthers)>> = true;

struct PositionBasedID { constexpr static bool positionBasedID = true; };
template <typename T, typename = void>
constexpr bool hasPositionBasedID = false;
template <typename T>
constexpr bool hasPositionBasedID<T, std::void_t<decltype(T::positionBasedID)>> = true;

//this means the server won't automatically send packets and stuff like that to keep the entity sync
struct NotSyncronizedEntity { constexpr static bool notSyncronizedEntity = true; };
template <typename T, typename = void>
constexpr bool hasNotSyncronizedEntity = false;
template <typename T>
constexpr bool hasNotSyncronizedEntity<T, std::void_t<decltype(T::notSyncronizedEntity)>> = true;


template <typename T, typename = void>
constexpr bool hasGetColliderOffset = false;
template <typename T>
constexpr bool hasGetColliderOffset<T, std::void_t<decltype(std::declval<T>().getColliderOffset())>> = true;


template <typename T, typename = void>
constexpr bool hasForces = false;

template <typename T>
constexpr bool hasForces<T, std::void_t<decltype(T::forces)>> = true;


template <typename T, typename = void>
constexpr bool hasAnimations = false;
template <typename T>
constexpr bool hasAnimations<T, std::void_t<decltype(T::animationStateServer)>> = true;


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



constexpr std::uint64_t MASK_24 = 0xFFFFFF;
constexpr std::uint64_t MASK_8 = 0xFF;

std::uint64_t fromBlockPosToEntityID(int x, unsigned char y, int z, unsigned char entityType);
glm::ivec3 fromEntityIDToBlockPos(std::uint64_t entityId, unsigned char *outEntityType = 0);


template<class T>
struct StaticCircularBufferForBuffering
{

	constexpr static int BUFFER_CAPACITY = 10;

	T elements[BUFFER_CAPACITY] = {};
	std::uint64_t timeAdded[BUFFER_CAPACITY] = {};
	std::uint64_t timeUpdatedOnServer[BUFFER_CAPACITY] = {};

	unsigned char startIndex = 0;
	unsigned char count = 0;

	void addElement(T &element, std::uint64_t timeLocal, std::uint64_t timeUpdatedEntityOnServer)
	{

		if (count < BUFFER_CAPACITY)
		{
			int newPosition = startIndex + count;
			newPosition %= BUFFER_CAPACITY;
			elements[newPosition] = element;
			timeAdded[newPosition] = timeLocal;
			timeUpdatedOnServer[newPosition] = timeUpdatedEntityOnServer;
			count++;
		}
		else
		{
			elements[startIndex] = element;
			timeAdded[startIndex] = timeLocal;
			timeUpdatedOnServer[startIndex] = timeUpdatedEntityOnServer;
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

	void clear()
	{
		startIndex = 0;
		count = 0;
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

	std::uint64_t getNewestElementTimeUpdatedOnServer()
	{
		//todo add perma assert back here
		if (count == 0)
		{
			std::cout << "circular buffer buffer underflow !!!!!!!!!!!!";
			exit(0);
		}
		//permaAssertComment(count != 0, "circular buffer buffer underflow");
		int index = startIndex + count - 1;
		if (index < 0) { index = BUFFER_CAPACITY - 1; }
		index = index % BUFFER_CAPACITY;

		return timeUpdatedOnServer[index];
	}

	T &getOldestElementReff()
	{
		//todo add perma assert back here
		if (count == 0)
		{
			std::cout << "circular buffer buffer underflow !!!!!!!!!!!!";
			exit(0);
		}

		return elements[startIndex];
	}

	std::uint64_t getNewestElementTimer()
	{
		//todo add perma assert back here
		if (count == 0)
		{
			std::cout << "circular buffer buffer underflow !!!!!!!!!!!!";
			exit(0);
		}
		//permaAssertComment(count != 0, "circular buffer buffer underflow");
		int index = startIndex + count - 1;
		if (index < 0) { index = BUFFER_CAPACITY - 1; }
		index = index % BUFFER_CAPACITY;

		return timeAdded[index];
	}


};




//dropped item doesn't inherit from this class, so if you want
//to add something here make another type of server entity.
template <class T>
struct ServerEntity
{

	ServerEntity() //default values
	{
		if constexpr (::hasLookDirectionAnimation<T>)
		{
			wantToLookDirection = glm::vec3(0, 0, -1);
		}
	};

	T entity = {};

	glm::dvec3 &getPosition()
	{
		//todo assert 0 !
		if constexpr (hasPositionBasedID<T>)
		{
			thread_local static glm::dvec3 stub;
			stub = {};
			return stub;
		}
		else
		{
			return entity.position;
		}
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

	ConditionalMember<::hasLookDirectionAnimation<T>, glm::vec3> wantToLookDirection = {};

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
	ConditionalMember<hasAnimations<T>, AnimationStateClient> animationStateClient = {};

	glm::dvec3 getRubberBandPosition()
	{

		if constexpr (!hasPositionBasedID<T>)
		{
			return rubberBand.direction + entityBuffered.position;
		}
		else
		{
			return glm::dvec3{};
		}

	}

	glm::dvec3 &getPosition()
	{
		if constexpr (!hasPositionBasedID<T>)
		{
			return entityBuffered.position;
		}
		else
		{
			thread_local static glm::dvec3 stub;
			stub = {};
			return stub;
		}
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
		std::minstd_rand &rng, glm::mat4 rotMatrix)
	{

		if constexpr (hasAnimations<T>)
		{
			animationStateClient.update(deltaTime);
			//animator.setAnimation(Animator::running);


			auto computeOneAnimationBone = [&](Animation &animation, float animationTime, int i)
			{
				auto &keyFrames = animation.kayFrames[i];

				//todo only one element case!
				for (int k = 0; k < keyFrames.size() - 1; k++)
				{

					if ((keyFrames[k].timestamp <= animationTime &&
						keyFrames[k + 1].timestamp >= animationTime
						)
						|| k == keyFrames.size() - 2)
					{
						auto rez = interpolateKeyFrames(keyFrames[k], keyFrames[k + 1], animationTime);
						auto matrix = rez.getMatrix();

						//skinningMatrix[i] = skinningMatrix[i] * matrix;
						skinningMatrix[i] = rotMatrix * matrix;
						break;
					}

				}
			};


		#pragma region first normal animation

			int animationIndex = model.animationsIndex[animationStateClient.currentAnimation];


			if (animationIndex >= 0)
			{
				Animation &animation = model.animations[animationIndex];

				//todo
				if (animationStateClient.animationTime >= animation.animationLength)
				{
					animationStateClient.animationTime = 0;
				}

				//attack
				//if (animationStateClient.isAttacking)
				//{
				//	if (animationStateClient.attackTimer >= animation.animationLength)
				//	{
				//		animationStateClient.attackTimer = animation.animationLength;
				//		animationStateClient.isAttacking = false;
				//	}
				//}

				//std::cout << animator.animationTime << "\n";

				float animationTime = animationStateClient.animationTime;

				//attack
				//if (animationStateClient.isAttacking)
				//{
				//	animationTime = animationStateClient.attackTimer;
				//}

				size_t animationBonesSize = animation.kayFrames.size();
				if (animationBonesSize)
				{
					assert(animationBonesSize == model.transforms.size());

					for (int i = 0; i < animationBonesSize; i++)
					{
						computeOneAnimationBone(animation, animationTime, i);
					}

				}


			}
		#pragma endregion

		#pragma region attacking animation

			//attack
			if (animationStateClient.isAttacking)
			{
				int animationIndex = model.animationsIndex[Animation::AnimationType::meleHit];

				if (animationIndex >= 0)
				{
					Animation &animation = model.animations[animationIndex];

					//attack
					if (animationStateClient.isAttacking)
					{
						if (animationStateClient.attackTimer >= animation.animationLength)
						{
							animationStateClient.attackTimer = animation.animationLength;
							animationStateClient.isAttacking = false;
						}
					}

					float animationTime = animationStateClient.attackTimer;

					if (model.lArmIndex > -1) { computeOneAnimationBone(animation, animationTime, model.lArmIndex); }
					if (model.rArmIndex > -1) { computeOneAnimationBone(animation, animationTime, model.rArmIndex); }
					if (model.lArmArmourIndex > -1) { computeOneAnimationBone(animation, animationTime, model.lArmArmourIndex); }
					if (model.rArmArmourIndex > -1) { computeOneAnimationBone(animation, animationTime, model.rArmArmourIndex); }

				}



			}

		#pragma endregion





		}




		//if(0)
		if constexpr (hasLookDirectionAnimation<T>)
		{
			auto rotation = glm::toMat4(glm::quatLookAt(glm::normalize(getRubberBandLookDirection()), glm::vec3(0, 1, 0)));

			//eyes and pupils stuff
			{
				if (model.pupilsIndex > -1)
				{
					skinningMatrix[model.pupilsIndex] = skinningMatrix[model.pupilsIndex] * rotation;
				}

				if (model.lEyeIndex > -1)
				{
					skinningMatrix[model.lEyeIndex] = skinningMatrix[model.lEyeIndex] * rotation;
				}

				if (model.rEyeIndex > -1)
				{
					skinningMatrix[model.rEyeIndex] = skinningMatrix[model.rEyeIndex] * rotation;
				}
			}

			if (model.headIndex > -1)
			{
				skinningMatrix[model.headIndex] = skinningMatrix[model.headIndex] * rotation;
			}

			if (model.headArmourIndex > -1)
			{
				skinningMatrix[model.headArmourIndex] = skinningMatrix[model.headArmourIndex] * rotation;
			}

		};

		auto scaleDownMatrix = [](float x)
		{
			return glm::translate(glm::vec3(0, -x/2.f, 0)) * glm::scale(glm::vec3{1,x + 1.f,1});
		};

		if constexpr (hasEyesAndPupils<T>)
		{
			int stuff = 0;

			//blink
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

				//player, version
				if (T().eyesAndPupils == EYE_ANIMATION_TYPE_PLAYER)
				{
					float pupilScale = pupilDisplacement * 1;
					pupilDisplacement *= 2.0f * -(1.f / 16.f);

					skinningMatrix[model.pupilsIndex] =
						skinningMatrix[model.pupilsIndex] *
						glm::translate(glm::vec3(0, pupilDisplacement, 0))
						* scaleDownMatrix(pupilScale);
				}
				else //other
				{
					pupilDisplacement *= 3.9f * -(1.f / 16.f);

					skinningMatrix[model.pupilsIndex] =
						skinningMatrix[model.pupilsIndex] *
						glm::translate(glm::vec3(0, pupilDisplacement, 0));
				}

				

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

		BASE_CLIENT *baseClient = (BASE_CLIENT *)this;

	#pragma region compute buffering		
		if constexpr(!hasPositionBasedID<T>) //no rubber banding for position based entities for now
		{

			//remove buffering and just use the latest event
			if(0)
			if (bufferedEntityData.count)
			{
				auto oldPosition = entityBuffered.position;
				restantTime = computeRestantTimer(bufferedEntityData.getNewestElementTimeUpdatedOnServer(),
					serverTimer);

				entityBuffered = bufferedEntityData.getNewestElementReff();

				entityBuffered.update(restantTime, chunkGetter);
				restantTime = 0;

				rubberBand.addToRubberBand(oldPosition - entityBuffered.position);


				bufferedEntityData.clear();
			}

			//if(0)
			if (bufferedEntityData.count)
			{
				int timeDelay = 200;
				bool maxCapacity = bufferedEntityData.count == bufferedEntityData.BUFFER_CAPACITY;

				if (bufferedEntityData.count > 1)
				{
					int clearCount = 0;
					float interpolator = 1;
					int elementStart = bufferedEntityData.startIndex;
					int nextElement = elementStart;

					std::uint64_t time = bufferedEntityData.timeUpdatedOnServer[elementStart];
					std::uint64_t myTimer = 0;
					if (serverTimer > timeDelay) { myTimer = serverTimer - timeDelay; }

					if (myTimer < time)
					{
						//we don't update it yet unless we start to overflow
						//entityBuffered = bufferedEntityData.elements[elementStart];

						if (maxCapacity)
						{
							restantTime = computeRestantTimer(bufferedEntityData.timeUpdatedOnServer[bufferedEntityData.startIndex],
								serverTimer);
							auto oldPosition = entityBuffered.position;
							entityBuffered = bufferedEntityData.getOldestElementReff();

							entityBuffered.update(restantTime, chunkGetter);
							restantTime = 0;

							rubberBand.addToRubberBand(oldPosition - entityBuffered.position);
							
							bufferedEntityData.removeLastElement();
						}
					}
					else
					{
						for (int i = 0; i < bufferedEntityData.count - 1; i++)
						{

							nextElement = elementStart;
							nextElement++;
							nextElement %= bufferedEntityData.BUFFER_CAPACITY;


							std::uint64_t time = bufferedEntityData.timeUpdatedOnServer[elementStart];
							std::uint64_t time2 = bufferedEntityData.timeUpdatedOnServer[nextElement];


							if (myTimer >= time && myTimer <= time2 && time <= time2)
							{
								interpolator = 1;
								if (time != time2)
								{
									interpolator = (myTimer - time) / float(time2 - time);
									interpolator = glm::clamp(interpolator, 0.f, 1.f);
								}

								break;
							}

							if (i == bufferedEntityData.count - 2)
							{
								clearCount = bufferedEntityData.count;
								break;
							}

							elementStart++;
							elementStart %= bufferedEntityData.BUFFER_CAPACITY;
							clearCount++;
						}

						T &entity1 = bufferedEntityData.elements[elementStart];
						T &entity2 = bufferedEntityData.elements[nextElement];

						//todo interpolate time or something?
						restantTime = computeRestantTimer(bufferedEntityData.timeUpdatedOnServer[elementStart],
							serverTimer);
						auto oldPosition = entityBuffered.position;
						entityBuffered = entity1;

						entityBuffered.update(restantTime, chunkGetter);
						restantTime = 0;

						rubberBand.addToRubberBand(oldPosition - baseClient->getPosition());

						
						//entityBuffered.position = lerp(entityBuffered.position, entity2.position, interpolator);
						elementStart++;

						for (int i = 0; i < clearCount; i++)
						{
							bufferedEntityData.removeLastElement();
						}
					}



				}
				else
				{

					std::uint64_t time = bufferedEntityData.getNewestElementTimeUpdatedOnServer();
					std::uint64_t myTimer = 0;
					if (serverTimer > timeDelay) { myTimer = serverTimer - timeDelay; }

					if (myTimer < time)
					{
						//we don't update it yet
						//entityBuffered = bufferedEntityData.getNewestElementReff();
					}
					else
					{
						restantTime = computeRestantTimer(bufferedEntityData.getNewestElementTimeUpdatedOnServer(),
							serverTimer);
						auto oldPosition = entityBuffered.position;
						entityBuffered = bufferedEntityData.getNewestElementReff();
						entityBuffered.update(restantTime, chunkGetter);
						restantTime = 0;

						rubberBand.addToRubberBand(oldPosition - entityBuffered.position);

						bufferedEntityData.clear();
					}


				}



			}

		}
	#pragma endregion


		//baseClient->rubberBand = {};

		if (restantTime < 0)
		{
			float timer = deltaTime + restantTime;
			if (timer > 0)
			{
				//auto oldPosition = baseClient->oldPositionForRubberBand;
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

		//animations
		if constexpr (hasAnimations<T>)
		{
			entityBuffered.animationStateServer.update(deltaTime);
			
			if (entityBuffered.animationStateServer.attacked)
			{
				animationStateClient.signalAttack();
			}

			MotionState forces = {};
			if constexpr (hasForces<T>) { forces = entityBuffered.forces; }

			if (!forces.colidesBottom())
			{
				animationStateClient.setAnimation(Animation::falling);
			}
			else
			{
				if (entityBuffered.animationStateServer.runningTime > 0)
				{
					animationStateClient.setAnimation(Animation::running);
				}
				else
				{
					animationStateClient.setAnimation(Animation::idle);
				}

			}

		}

		if (rubberBand.initialSize)
		{
			rubberBand.computeRubberBand(deltaTime * 1);
		}

		if constexpr (hasBodyOrientation<T>)
		{
			if (!wasKilled)
				rubberBandOrientation.computeRubberBandOrientation(deltaTime,
				entityBuffered.bodyOrientation,
				entityBuffered.lookDirectionAnimation);
		}

		//TODO probably remove in the future
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
		}
		else
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
float getRandomNumberFloat(int x, int y, int z, float a, float b);

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

