#include "camera.h"


glm::mat4x4 Camera::getProjectionMatrix()
{
	auto mat = glm::perspective(this->fovRadians, this->aspectRatio, this->closePlane,
		this->farPlane);

	return mat;
}

//todo better rotate function
void Camera::rotateCamera(const glm::vec2 delta)
{
	glm::vec3 rotateYaxe = glm::cross(viewDirection, up);

	viewDirection = glm::mat3(glm::rotate(delta.x, up)) * viewDirection;

	if (delta.y < 0)
	{	//down
		if (viewDirection.y < -0.99)
			goto noMove;
	}
	else
	{	//up
		if (viewDirection.y > 0.99)
			goto noMove;
	}

	viewDirection = glm::mat3(glm::rotate(delta.y, rotateYaxe)) * viewDirection;
noMove:

	viewDirection = glm::normalize(viewDirection);
}

void Camera::moveFPS(glm::vec3 direction)
{
	viewDirection = glm::normalize(viewDirection);

	//forward
	float forward = -direction.z;
	float leftRight = direction.x;
	float upDown = direction.y;

	glm::vec3 move = {};

	move += up * upDown;
	move += glm::normalize(glm::cross(viewDirection, up)) * leftRight;
	move += viewDirection * forward;

	this->position += move;
}

void Camera::rotateFPS(glm::ivec2 mousePos, float speed, bool shouldMove)
{
	static glm::ivec2 lastMousePos = {};
	if (shouldMove)
	{
		glm::ivec2 currentMousePos = mousePos;


		glm::vec2 delta = lastMousePos - currentMousePos;
		delta *= speed;

		rotateCamera(delta);

		lastMousePos = currentMousePos;
	}
	else
	{
		lastMousePos = mousePos;
	}

}

void Camera::decomposePosition(glm::vec3& floatPart, glm::ivec3& intPart)
{
	intPart = glm::ivec3(position);

	glm::dvec3 doublePosition = position;
	doublePosition -= intPart;
	floatPart = doublePosition;

}








