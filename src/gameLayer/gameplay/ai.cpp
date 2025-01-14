#include <gameplay/ai.h>


void lookAtPosition(glm::dvec3 position,
	glm::vec3 &lookDirectionAnimation,
	glm::dvec3 yourEntityPosition, glm::vec2 bodyOrientation
	, float tresshold)
{
	glm::vec3 vireDirection = position + glm::dvec3(0, 1.0, 0) - yourEntityPosition;
	float l = glm::length(vireDirection);
	if (l > 0.01)
	{
		vireDirection /= l;
		lookDirectionAnimation = vireDirection;
	}

	removeBodyRotationFromHead(bodyOrientation, lookDirectionAnimation);

	//don't break their neck lol
	adjustVectorTowardsDirection(lookDirectionAnimation, {0,0,-1}, tresshold);
};
