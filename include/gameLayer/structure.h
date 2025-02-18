#pragma once
#include <blocks.h>
#include <vector>

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


struct StructuresManager
{

	bool loadAllStructures();

	std::vector<StructureData *> trees;
	std::vector<StructureData *> jungleTrees;
	std::vector<StructureData *> palmTrees;
	std::vector<StructureData *> treeHouses;
	std::vector<StructureData *> smallPyramids;
	std::vector<StructureData *> birchTrees;
	std::vector<StructureData *> igloos;
	std::vector<StructureData *> spruceTrees;
	std::vector<StructureData *> spruceTreesSlim;
	std::vector<StructureData *> tallTreesSlim;
	std::vector<StructureData *> smallStones;

	void clear();
};

