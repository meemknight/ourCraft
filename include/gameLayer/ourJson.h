#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct Entry
{
	enum
	{
		none,
		numberType,
		boolType,
		stringType,
	};


	float numberValue = 0;
	int intValue = 0;
	std::string strValue;

	unsigned char type = 0;
};

struct KeyValuePair
{


	std::unordered_map<std::string, Entry> elements;

	bool loadElementsFromFile(const char *fileName);
	bool loadElementsFromData(const char *data);

	void printAll();

	std::vector<char> formatIntoFileData();
	void writeIntoFile(const char *fileName);
};
