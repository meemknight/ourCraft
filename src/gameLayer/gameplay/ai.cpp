#include <gameplay/ai.h>


void lookAtPosition(glm::dvec3 position,
	glm::vec3 &lookDirectionAnimation,
	glm::dvec3 yourEntityPosition, glm::vec2 bodyOrientation
	, float tresshold)
{
	glm::vec3 viewDirection = position + glm::dvec3(0, 1.0, 0) - yourEntityPosition;
	lookAtDirection(viewDirection, lookDirectionAnimation, bodyOrientation, tresshold);
};


void lookAtDirection(glm::vec3 viewDirection,
	glm::vec3 &lookDirectionAnimation, glm::vec2 bodyOrientation
	, float tresshold)
{

	float l = glm::length(viewDirection);
	if (l > 0.01)
	{
		viewDirection /= l;
		lookDirectionAnimation = viewDirection;
	}

	removeBodyRotationFromHead(bodyOrientation, lookDirectionAnimation);

	//don't break their neck lol
	adjustVectorTowardsDirection(lookDirectionAnimation, {0,0,-1}, tresshold);
};

void lookAtDirectionWithBodyOrientation(glm::vec3 viewDirection,
	glm::vec3 &lookDirectionAnimation, glm::vec2 &bodyOrientation
	, float tresshold)
{

	float l = glm::length(viewDirection);
	if (l > 0.01)
	{
		viewDirection /= l;
		lookDirectionAnimation = viewDirection;
	}

	bodyOrientation.x = lookDirectionAnimation.x;
	bodyOrientation.y = lookDirectionAnimation.z;

	removeBodyRotationFromHead(bodyOrientation, lookDirectionAnimation);

	//don't break their neck lol
	adjustVectorTowardsDirection(lookDirectionAnimation, {0,0,-1}, tresshold);
};
