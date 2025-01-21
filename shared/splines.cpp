#include <splines.h>
#include <glm/glm.hpp>



glm::vec2 lerp(glm::vec2 a, glm::vec2 b, float r)
{
	return a * (1.f - r) + b * r;
}

glm::dvec3 lerp(glm::dvec3 a, glm::dvec3 b, float r)
{
	return a * (1.0 - (double)r) + b * (double)r;
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

void Spline::removeSpline()
{
	if (size > 2)
	{
		size--;
		points[size - 1].x = 1;
	}
}

float Spline::applySpline(float p)
{
	return ::applySpline(p, *this);
}

std::string Spline::saveSettings(int tabs)
{
	std::string rez;
	rez.reserve(100);

	auto addTabs = [&]() { for (int i = 0; i < tabs; i++) { rez += '\t'; } };

	addTabs();
	rez += "{\n";

	for (int i = 0; i < std::min(MAX_SPLINES_COUNT, size); i++)
	{
		addTabs();
		rez += std::to_string(points[i].x);
		rez += ",";
		rez += std::to_string(points[i].y);
		rez += ";\n";
	};


	addTabs();
	rez += "}\n";


	return rez;
}

void Spline::sanitize()
{
	for (int i = 0; i < size; ++i)
	{
		points[i] = glm::clamp(points[i], {0,0}, {1,1});

		if (i > 0)
		{
			points[i] = glm::clamp(points[i], {points[i - 1].x,0}, {1,1});
		}
	}
}
