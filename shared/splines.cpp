#include <splines.h>

float lerp(float a, float b, float r)
{
	return a * (1 - r) + b * r;
}

glm::vec2 lerp(glm::vec2 a, glm::vec2 b, float r)
{
	return a * (1.f - r) + b * r;
}

float applySpline(float p, glm::vec2 *points, size_t s)
{
	if (s >= 2)
	{
		for (int i = 1; i < s; i++)
		{

			if (p <= points[i].x)
			{
				auto p1 = points[i - 1];
				auto p2 = points[i];

				return lerp(p1.y, p2.y, (p - p1.x) / (p2.x - p1.x));
			}
		}
	}

	return p;
}

float applySpline(float p, Spline &s)
{
	return applySpline(p, s.points, s.size);
}

void Spline::addSpline()
{
	if (size < MAX_SPLINES_COUNT)
	{
		if (size >= 2)
		{
			points[size] = points[size - 1];
			points[size - 1] = lerp(points[size - 2], points[size], 0.5f);
			size++;
		}
		else
		{
			size = 2;
			points[0] = {0,0};
			points[1] = {1,1};
		}
	}
}

float Spline::applySpline(float p)
{
	return ::applySpline(p, *this);
}
