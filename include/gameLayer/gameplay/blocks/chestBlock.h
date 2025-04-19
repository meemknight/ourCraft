#pragma once
#include <vector>
#include <gameplay/items.h>


constexpr static int CHEST_CAPACITY = 27;

struct ChestBlock
{

	Item items[CHEST_CAPACITY] = {};
		
	size_t formatIntoData(std::vector<unsigned char> &appendTo);

	bool readFromBuffer(unsigned char *data, size_t s, size_t &outReadSize);

	bool isDataValid();

	void normalize();
};