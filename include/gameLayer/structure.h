#pragma once
#include <blocks.h>
#include <vector>

struct StructureData
{
	glm::ivec3 size;
	int unused = 0;

	//data is held outside

	BlockType &unsafeGet(int x, int y, int z)
	{
		return ((BlockType*)(this + 1))[y + z * size.y + x * size.y*size.z];
	}

	BlockType *safeGet(int x, int y, int z)
	{
		if (x < 0 || y < 0 || z < 0 || x >= size.x || y >= size.y || z >= size.z)
		{
			return nullptr;
		}

		return &unsafeGet(x, y, z);
	}

};


struct StructuresManager
{

	bool loadAllStructures();

	std::vector<StructureData*> trees;

	void clear();
};

