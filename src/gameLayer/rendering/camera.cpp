#include "rendering/camera.h"
#include <math.h>

glm::mat4x4 Camera::getProjectionMatrix()
{
	if (std::isinf(this->aspectRatio) || std::isnan(this->aspectRatio) || this->aspectRatio == 0)
	{
		return glm::mat4x4(1.f);
	}

	auto mat = glm::perspective(this->fovRadians, this->aspectRatio, this->closePlane,
		this->farPlane);

	return mat;
}

glm::mat4x4 Camera::getViewMatrix()
{
	return  glm::lookAt(glm::vec3{0,0,0}, viewDirection, up);
}

glm::mat4x4 Camera::getViewMatrixWithPosition()
{
	glm::vec3 dir = glm::vec3(position) + glm::vec3(viewDirection);
	return  glm::lookAt(glm::vec3(position), dir, up);
}

glm::mat4x4 Camera::getViewProjectionWithPositionMatrix()
{
	return getProjectionMatrix() * getViewMatrixWithPosition();
}

//todo better rotate function
void Camera::rotateCamera(const glm::vec2 delta)
{
//	glm::vec3 rotateYaxe = glm::cross(viewDirection, up);
//
//	viewDirection = glm::mat3(glm::rotate(delta.x, up)) * viewDirection;
//
//	if (delta.y < 0)
//	{	//down
//		if (viewDirection.y < -0.99999)
//			goto noMove;
//	}
//	else
//	{	//up
//		if (viewDirection.y > 0.99999)
//			goto noMove;
//	}
//
//	viewDirection = glm::mat3(glm::rotate(delta.y, rotateYaxe)) * viewDirection;
//noMove:
//
//	viewDirection = glm::normalize(viewDirection);

	constexpr float PI = 3.1415926;

	yaw += delta.x;
	pitch += delta.y;

	if (yaw >= 2 * PI)
	{
		yaw -= 2 * PI;
	}
	else if (yaw < 0)
	{
		yaw = 2 * PI - yaw;
	}

	if (pitch > PI/2.f - 0.01) { pitch = PI / 2.f - 0.01; }
	if (pitch < -PI / 2.f + 0.01) { pitch = -PI / 2.f + 0.01; }

	viewDirection = glm::vec3(0,0,-1);

	viewDirection = glm::mat3(glm::rotate(yaw, up)) * viewDirection;

	glm::vec3 rotatePitchAxe = glm::cross(viewDirection, up);
	viewDirection = glm::mat3(glm::rotate(pitch, rotatePitchAxe)) * viewDirection;

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
	::decomposePosition(position, floatPart, intPart);
}

void decomposePosition(glm::dvec3 in, glm::vec3 &floatPart, glm::ivec3 &intPart)
{
	intPart = glm::floor(in);

	glm::dvec3 doublePosition = in;
	doublePosition -= intPart;
	floatPart = doublePosition;
}

glm::ivec3 from3DPointToBlock(glm::dvec3 in)
{
	in += glm::dvec3(0.5, 0.5, 0.5);
	return glm::floor(in);
}







