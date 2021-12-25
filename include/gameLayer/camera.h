#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>

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
	float fovRadians = glm::radians(60.f);

	float closePlane = 0.01f;
	float farPlane = 200.f;


	glm::vec3 position = {};
	glm::vec3 viewDirection = {0,0,-1};

	glm::mat4x4 getProjectionMatrix();

	glm::mat4x4 getWorldToViewMatrix();

	glm::mat4x4 getViewProjectionMatrix();

	void rotateCamera(const glm::vec2 delta);

	void moveFPS(glm::vec3 direction);
	void rotateFPS(glm::ivec2 mousePos, float speed, bool shouldMove);


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


};