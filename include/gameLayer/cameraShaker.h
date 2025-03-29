#pragma once
#include <rendering/camera.h>
#include <iostream>


struct CameraShaker
{
	// Configuration constants (tweak these values)
	constexpr static float walkShakeFrequency = 14.0f;   // oscillation frequency for steps
	constexpr static float walkShakeAmplitude = 0.001f;   // bobbing amplitude
	constexpr static float hitShakeAmplitude = 0.05f;   // impulse shake on hit
	constexpr static float hitShakeDuration = 0.2f;   // how long the hit impulse lasts (seconds)
	constexpr static float fovSpeedThreshold = 0.1f;   // speed above which FOV is modified
	constexpr static float fovChangeFactor = 0.05f;  // how much extra speed adds to FOV change
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

	float realSpeedLast1 = 0;
	float realSpeedLast2 = 0;
	float realSpeedLast3 = 0;
	float realSpeedLast4 = 0;

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
			//upOffset.x -= inputMovement.x * 0.01f;
			//upOffset.x = glm::clamp(upOffset.x, -0.2f, 0.2f);
			//upOffset.x = glm::clamp(upOffset.x, -0.2f * inputMovement.x * 0.09f, 0.2f * inputMovement.x * 0.09f);

		}

		// ---------------------------
		// 3. Dynamic FOV change at high speeds
		// ---------------------------

		realMovement *= 100.f;

		float realSpeed = glm::length(realMovement);
		float avgRealSpeed = std::max({realSpeed, realSpeedLast1, realSpeedLast2, realSpeedLast3, realSpeedLast4});
		realSpeedLast4 = realSpeedLast3;
		realSpeedLast3 = realSpeedLast2;
		realSpeedLast2 = realSpeedLast1;
		realSpeedLast1 = realSpeed;

	#pragma region fov
		if(0)
		{
			

			//std::cout << avgRealSpeed << " ";
			//if(0)

			float desiredFov = 0;

			if (avgRealSpeed > fovSpeedThreshold)
			{
				//float excessSpeed = avgRealSpeed - fovSpeedThreshold;
				//desiredFov = glm::min(excessSpeed * 0.05f, fovMaxChange);

				//desiredFov = fovMaxChange / 2.f;
				desiredFov = fovMaxChange;
			}

			//if (avgRealSpeed > fovSpeedThreshold * 2)
			//{
			//	desiredFov = fovMaxChange / 1.5f;
			//}
			//
			//if (avgRealSpeed > fovSpeedThreshold * 4)
			//{
			//	desiredFov = fovMaxChange / 1;
			//}

			//if (avgRealSpeed > fovSpeedThreshold)
			//{
			//	float excessSpeed = avgRealSpeed - fovSpeedThreshold;
			//	fovOffset += excessSpeed * fovChangeFactor;
			//	fovOffset = glm::min(fovOffset, fovMaxChange);
			//	//fovOffset = glm::min(fovOffset, excessSpeed);
			//}
			//else
			//{
			//	fovOffset *= damp;
			//}
			//desiredFov = 0;

			if (desiredFov == 0)
			{
				fovOffset *= damp;
			}
			else
			{
				if (desiredFov > fovOffset)
				{
					fovOffset += deltaTime;
					if (fovOffset > desiredFov) { fovOffset = desiredFov; }
				}
				else if (desiredFov < fovOffset)
				{
					fovOffset -= deltaTime;
					if (fovOffset < desiredFov) { fovOffset = desiredFov; }
				}
			}
		}
	#pragma endregion

		// ---------------------------
		// 4. Falling and lateral tilting
		// ---------------------------
		if (0)
			if (realMovement.y < -0.1f)  // when falling (negative y indicates downward movement)
			{
				// A tilt effect from falling (pitching the camera slightly downward)
				float fallTilt = -realMovement.y * fallTiltMultiplier;
				// Lateral movement causes a roll effect (tilting the camera left/right)
				float lateralTilt = realMovement.x * lateralTiltMultiplier;

				//viewDirOffset += glm::vec3(lateralTilt, fallTilt, 0.0f);
				upOffset += glm::vec3(lateralTilt, 0.f, 0.0f);
			}

		// ---------------------------
		// 5. Sudden stops / deceleration shake
		// ---------------------------
		// Compare current input to previous frame
		if (0)
		{
			glm::vec3 inputDelta = inputMovement * deltaTime - prevInputMovement;
			// If there is a significant drop in movement, add a subtle shake
			if (glm::length(inputDelta) < -0.5f)
			{
				positionOffset += glm::vec3(0.0f, -0.05f, 0.0f);
			}
			prevInputMovement = inputMovement;
			prevRealMovement = realMovement;
		}

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