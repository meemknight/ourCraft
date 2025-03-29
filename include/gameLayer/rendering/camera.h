#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>


int getViewDirectionRotation(glm::vec3 vec);

struct Camera
{
	Camera() = default;
	Camera(float aspectRatio, float fovRadians)
		:aspectRatio(aspectRatio),
		fovRadians(fovRadians)
	{
	}

	glm::vec3 up = {0.f,1.f,0.f};

	float aspectRatio = 1;
	float fovRadians = glm::radians(70.f);

	float closePlane = 0.01f;
	float farPlane = 1200.f;


	glm::dvec3 position = {};
	glm::vec3 viewDirection = {0,0,-1};

	glm::mat4x4 getProjectionMatrix();
	glm::dmat4x4 getProjectionMatrixDouble();
	glm::mat4x4 getViewMatrix();
	glm::mat4x4 getViewMatrixWithPosition();
	glm::dmat4x4 getViewMatrixWithPositionDouble();
	glm::mat4x4 getViewProjectionWithPositionMatrix();
	
	glm::dmat4x4 getViewProjectionWithPositionMatrixDouble();

	int getViewDirectionRotation()
	{
		return ::getViewDirectionRotation(viewDirection);
	}

	void rotateCamera(const glm::vec2 delta);
	float yaw = 0.f;
	float pitch = 0.f;

	void moveFPS(glm::vec3 direction);
	void rotateFPS(glm::ivec2 mousePos, float speed, bool shouldMove);
	void rotateFPSController(glm::vec2 vector, float speed);
	glm::ivec2 lastMousePos = {};

	glm::mat4x4 lastFrameViewProjMatrix = getProjectionMatrix() * getViewMatrix();

	bool operator==(const Camera& other)
	{
		return
			(up == other.up)
			&& (aspectRatio == other.aspectRatio)
			&& (fovRadians == other.fovRadians)
			&& (closePlane == other.closePlane)
			&& (farPlane == other.farPlane)
			&& (position == other.position)
			&& (viewDirection == other.viewDirection)
			;
	};

	bool operator!=(const Camera& other)
	{
		return !(*this == other);
	};

	void decomposePosition(glm::vec3 &floatPart, glm::ivec3 &intPart);

	void decomposePosition(glm::vec3 &floatPart, glm::dvec3 in, glm::ivec3 &intPart);

};

void decomposePosition(glm::dvec3 in, glm::vec3 &floatPart, glm::ivec3 &intPart);
void decomposePosition(glm::dvec3 in, glm::vec4 &floatPart, glm::ivec4 &intPart);

glm::ivec3 from3DPointToBlock(glm::dvec3 in);


glm::mat4 lookAtSafe(glm::vec3 const &eye, glm::vec3 const &center, glm::vec3 const &upVec);

glm::dmat4 lookAtSafe(glm::dvec3 const &eye, glm::dvec3 const &center, glm::dvec3 const &upVec);



struct CameraShaker
{
	// Configuration constants (tweak these values)
	constexpr static float walkShakeFrequency = 14.0f;   // oscillation frequency for steps
	constexpr static float walkShakeAmplitude = 0.001f;   // bobbing amplitude
	constexpr static float hitShakeAmplitude = 0.05f;   // impulse shake on hit
	constexpr static float hitShakeDuration = 0.2f;   // how long the hit impulse lasts (seconds)
	constexpr static float fovSpeedThreshold = 5.0f;   // speed above which FOV is modified
	constexpr static float fovChangeFactor = 0.03f;  // how much extra speed adds to FOV change
	constexpr static float fovMaxChange = 0.3f;   // maximum FOV change
	constexpr static float fallTiltMultiplier = 0.5f;   // how much falling affects the tilt
	constexpr static float lateralTiltMultiplier = 0.01f;   // how much lateral movement tilts the view
	constexpr static float damping = 5.0f;   // general damping factor for smoothing

	// Internal state variables
	float timeAccumulator = 0.0f;            // accumulates time for sine functions
	float hitShakeTimer = 0.0f;            // countdown timer for hit impulse

	glm::vec3 positionOffset = glm::vec3(0.0f); // accumulated positional shake
	glm::vec3 upOffset = glm::vec3(0.0f); // additional up vector modification
	float fovOffset = 0.0f;            // accumulated FOV change
	glm::vec3 viewDirOffset = glm::vec3(0.0f); // offset for the view direction

	// Keep track of previous state (could be used to detect rapid stops)
	glm::vec3 prevInputMovement = glm::vec3(0.0f);
	glm::vec3 prevRealMovement = glm::vec3(0.0f);

	// Public variables that will be applied to the camera
	glm::vec3 up = {0.f,1.f,0.f};
	float fovChange = 0.0f;
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 viewDirection = {0.f,0.f,-1.f};

	// Helper function: returns a random float in [0,1]
	float randFloat()
	{
		return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}

	void updateCameraShake(float deltaTime, glm::vec3 inputMovement, glm::vec3 realMovement, bool wasJustHit)
	{
		// Update time accumulator (for smooth oscillation)
		timeAccumulator += deltaTime;

		// Apply damping to all offsets so they gradually settle
		float damp = glm::clamp(1.0f - damping * deltaTime, 0.0f, 1.0f);
		positionOffset *= damp;
		upOffset *= damp;
		fovOffset *= damp;
		viewDirOffset *= damp;

		// ---------------------------
		// 1. Hit impulse shake
		// ---------------------------
		if (wasJustHit)
		{
			// Restart the hit shake timer
			hitShakeTimer = hitShakeDuration;
		}

		if (hitShakeTimer > 0.0f)
		{
			// Compute an impulse factor that decays over the hit duration
			float impulseFactor = hitShakeAmplitude * (hitShakeTimer / hitShakeDuration);

			// Add a random offset to simulate a jolt (position and view direction)
			positionOffset += glm::vec3(
				impulseFactor * (randFloat() - 0.5f),
				impulseFactor * (randFloat() - 0.5f),
				impulseFactor * (randFloat() - 0.5f)
			);

			viewDirOffset += glm::vec3(
				impulseFactor * (randFloat() - 0.5f),
				impulseFactor * (randFloat() - 0.5f),
				0.0f // avoid messing too much with depth
			);

			hitShakeTimer -= deltaTime;
		}

		// ---------------------------
		// 2. Walking bob (wobble shake)
		// ---------------------------
		float inputSpeed = glm::length(inputMovement);
		if (inputSpeed > 0.01f)
		{

			inputSpeed = glm::clamp(inputSpeed, 5.f, 20.f);
			float walkSpeed = glm::clamp((inputSpeed / 14.f), 1.f, 2.5f);

			//TODO INCREASE THE SHAKE SPEED WITH SPEED
			// Sine oscillation for bobbing (vertical motion)
			float bob = sin(timeAccumulator * walkShakeFrequency * walkSpeed) * walkShakeAmplitude * inputSpeed;
			float bobx = sin(timeAccumulator * walkShakeFrequency * walkSpeed * 0.5 + 3.14) * walkShakeAmplitude * inputSpeed * 0.65;
			positionOffset.y += bob;
			positionOffset.x += bobx;

			// Slight lateral tilt based on the horizontal (x) input
			//upOffset.z += inputMovement.x * 0.02f;
		}

		// ---------------------------
		// 3. Dynamic FOV change at high speeds
		// ---------------------------
		float realSpeed = glm::length(realMovement);
		if (inputSpeed > fovSpeedThreshold * deltaTime)
		{
			float excessSpeed = inputSpeed - fovSpeedThreshold * deltaTime;
			fovOffset += glm::min(excessSpeed * fovChangeFactor, fovMaxChange);
			fovOffset = glm::min(fovOffset, fovMaxChange);
		}

		// ---------------------------
		// 4. Falling and lateral tilting
		// ---------------------------
		if(0)
		if (realMovement.y < -0.1f)  // when falling (negative y indicates downward movement)
		{
			// A tilt effect from falling (pitching the camera slightly downward)
			float fallTilt = -realMovement.y * fallTiltMultiplier;
			// Lateral movement causes a roll effect (tilting the camera left/right)
			float lateralTilt = realMovement.x * lateralTiltMultiplier;

			viewDirOffset += glm::vec3(lateralTilt, fallTilt, 0.0f);
		}

		// ---------------------------
		// 5. Sudden stops / deceleration shake
		// ---------------------------
		// Compare current input to previous frame
		glm::vec3 inputDelta = inputMovement - prevInputMovement;
		// If there is a significant drop in movement, add a subtle shake
		if (glm::length(inputDelta) < -0.5f)
		{
			positionOffset += glm::vec3(0.0f, -0.05f, 0.0f);
		}
		prevInputMovement = inputMovement;
		prevRealMovement = realMovement;

		// ---------------------------
		// 6. Update the public variables
		// ---------------------------
		position = positionOffset;
		fovChange = fovOffset;
		// Compute the new up vector, adding the offset to the base (0,1,0) and normalizing.
		up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) + upOffset);
		// Update view direction: start from base (0,0,-1) and add any directional offsets.
		viewDirection = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f) + viewDirOffset);
	};

	void applyCameraShake(Camera &c)
	{
		c.up = up;
		c.fovRadians += fovChange;
		c.position += position;
		//c.viewDirection += viewDirection;
		//c.viewDirection = glm::normalize(c.viewDirection);

		if (fovChange == 0 && position == glm::vec3{} && up == glm::vec3(0, 1, 0))
		{
			timeAccumulator = 0;
		}
	};

};