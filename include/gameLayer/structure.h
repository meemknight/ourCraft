#pragma once
#include <blocks.h>
#include <vector>

struct StructureData
{
	glm::ivec3 size;
	int unused = 0;

	//data is held outside

	BlockType& unsafeGet(int x, int y, int z)
	{
		assert(x >= 0 && y >= 0 && z >= 0 && x < size.x && y < size.y && z < size.z);

		return ((BlockType*)(this + 1))[y + z * size.y + x * size.y * size.z];
	}

	BlockType& unsafeGetRotated(int x, int y, int z, int r)
	{
		if (r == 0)
		{
			return unsafeGet(x, y, z);
		}
		else if (r == 1)
		{
			return unsafeGet(size.z - 1 - z, y, x);
		}
		else if (r == 2)
		{
			return unsafeGet(size.x - x - 1, y, size.z - z - 1);
		}
		else if (r == 3)
		{
			return unsafeGet(z, y, size.x - x - 1);
		}
		else
		{
			assert(0);
		}

	}

	BlockType* safeGet(int x, int y, int z)
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
	std::vector<const char*> structurePaths = {
		RESOURCES_PATH "gameData/structures/trees",
		RESOURCES_PATH "gameData/structures/jungleTrees",
		RESOURCES_PATH "gameData/structures/palm",
		RESOURCES_PATH "gameData/structures/treeHouses",
		RESOURCES_PATH "gameData/structures/smallpyramid",
		RESOURCES_PATH "gameData/structures/birch",
		RESOURCES_PATH "gameData/structures/igloo",
		RESOURCES_PATH "gameData/structures/spruce"
	};
	std::vector<std::vector<StructureData*>> allStructures;

	void clear();
};