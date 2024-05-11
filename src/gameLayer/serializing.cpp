#include <serializing.h>
#include <cstring>

void writeData(std::vector<unsigned char> &vector, void *data, size_t size)
{
	vector.resize(vector.size() + size);
	std::memcpy(&vector[vector.size() - size], data, size);
}

