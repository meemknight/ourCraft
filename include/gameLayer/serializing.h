#pragma once
#include <vector>



void writeData(std::vector<unsigned char> &vector, void *data, size_t size);


template <class T>
void writeData(std::vector<unsigned char> &vector, T data)
{
	writeData(vector, &data, sizeof(data));
}

template <class T>
void writeDataUnsafe(void *into, T data)
{
	memcpy(into, &data, sizeof(data));
}

template <class T>
void readDataUnsafe(void *data, T &t)
{
	memcpy(&t, data, sizeof(T));
}

void readDataIntoVectorUnsafeUnresized(void *data, size_t pos,
	size_t size,
	std::vector<unsigned char> &v);