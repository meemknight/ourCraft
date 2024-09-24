#pragma once
#include "glm/vec3.hpp"

struct BaseBlock
{
	char name[32] = {};

	std::uint8_t offsetX = 0;
	std::uint8_t offsetY = 0;
	std::uint8_t offsetZ = 0;

	std::uint8_t sizeX = 0;
	std::uint8_t sizeY = 0;
	std::uint8_t sizeZ = 0;
};