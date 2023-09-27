#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <string>

constexpr static int MAX_SPLINES_COUNT = 16;

struct Spline
{

	glm::vec2 points[MAX_SPLINES_COUNT] = {{0,0}, {1,1}};
	int size = 2;

	void addSpline();

	float applySpline(float p);

	std::string saveSettings(int tabs);

	void sanitize();
};


float lerp(float a, float b, float r);
glm::vec2 lerp(glm::vec2 a, glm::vec2 b, float r);

float applySpline(float p, glm::vec2 *points, size_t s);

float applySpline(float p, Spline &s);

