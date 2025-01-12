#pragma once
#include "glm/vec3.hpp"
#include <vector>

struct BaseBlock
{
	char name[32] = {};

	std::int8_t offsetX = 0;
	std::int8_t offsetY = 0;
	std::int8_t offsetZ = 0;

	std::uint8_t sizeX = 0;
	std::uint8_t sizeY = 0;
	std::uint8_t sizeZ = 0;

	size_t formatIntoData(std::vector<unsigned char> &appendTo);

	bool readFromBuffer(unsigned char *data, size_t s, size_t &outReadSize);

	bool isDataValid();
};