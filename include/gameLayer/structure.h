#pragma once
#include <blocks.h>
#include <vector>
#include <glm/vec2.hpp>

struct StructureData
{
	glm::ivec3 size;
	int unused = 0;

	//data is held outside

	Block &unsafeGet(int x, int y, int z)
	{
		assert(x >= 0 && y >= 0 && z >= 0 && x < size.x && y < size.y && z < size.z);

		return ((Block *)(this + 1))[y + z * size.y + x * size.y*size.z];
	}

	Block &unsafeGetRotated(int x, int y, int z, int r)
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

	Block *safeGet(int x, int y, int z)
	{
		if (x < 0 || y < 0 || z < 0 || x >= size.x || y >= size.y || z >= size.z)
		{
			return nullptr;
		}

		return &unsafeGet(x, y, z);
	}

};

struct StructureDataAndFlags
{
	//todo other flags

	struct PerCollomFlags
	{
		glm::ivec2 minMax = {9999, -9999};
		bool hasBlocks = 0;
	};

	std::vector<PerCollomFlags> perCollomFlags;

	PerCollomFlags &getPerCollomFlagsUnsafeRotated(int x, int z, int r, glm::ivec3 size)
	{
		if (r == 0)
		{
			return getPerCollomFlagsUnsafe(x, z);
		}
		else if (r == 1)
		{
			return getPerCollomFlagsUnsafe(size.z - 1 - z, x);
		}
		else if (r == 2)
		{
			return getPerCollomFlagsUnsafe(size.x - x - 1, size.z - z - 1);
		}
		else if (r == 3)
		{
			return getPerCollomFlagsUnsafe(z, size.x - x - 1);
		}
		else
		{
			assert(0);
		}
	}

	PerCollomFlags &getPerCollomFlagsUnsafe(int x, int z)
	{
		return perCollomFlags[x + z * data->size.x];
	}

	StructureData *data;
};


struct StructuresManager
{

	bool loadAllStructures();

	std::vector<StructureDataAndFlags> trees;
	std::vector<StructureDataAndFlags> jungleTrees;
	std::vector<StructureDataAndFlags> palmTrees;
	std::vector<StructureDataAndFlags> treeHouses;
	std::vector<StructureDataAndFlags> smallPyramids;
	std::vector<StructureDataAndFlags> birchTrees;
	std::vector<StructureDataAndFlags> igloos;
	std::vector<StructureDataAndFlags> spruceTrees;
	std::vector<StructureDataAndFlags> spruceTreesSlim;
	std::vector<StructureDataAndFlags> tallTreesSlim;
	std::vector<StructureDataAndFlags> smallStones;
	std::vector<StructureDataAndFlags> abandonedHouse;
	std::vector<StructureDataAndFlags> goblinTower;

	void clear();
};

