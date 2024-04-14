#include <gameplay/entity.h>
#include <glm/glm.hpp>
#include <iostream>

//todo entities should freeze their state completely when chunks are missing

void computeRubberBand(RubberBand &rubberBand, glm::dvec3 &position, float deltaTime)
{
	if (rubberBand.initialSize)
	{
		float speed = rubberBand.initialSize * deltaTime * 18.f;

		float newSize = glm::length(rubberBand.direction) - speed;

		if(newSize <= 0.01)
		{
			rubberBand = {};
		}
		else
		{
			//std::cout << "ns: " << newSize << "\n";
			rubberBand.direction = glm::normalize(rubberBand.direction) * double(newSize);
		}
	}
	else
	{
		rubberBand = {};
	}

}

void RubberBand::computeRubberBand(glm::dvec3 &position, float deltaTime)
{
	::computeRubberBand(*this, position, deltaTime);
}

