#include <structure.h>
#include <safeSave.h>
#include <filesystem>
#include <iostream>

bool StructuresManager::loadAllStructures()
{

	auto loadFolder = [](const char *path, std::vector<StructureData*> &structures) 
	{
		std::error_code err = {};
		auto it = std::filesystem::directory_iterator(path, err);

		if (err)
		{
			//todo err out
			std::cout << err.message() << "\n";
			return 0;
		}

		for (const auto &entry : it)
		{
			if (entry.is_regular_file())
			{
				size_t s = 0;

				sfs::getFileSize(entry.path().string().c_str(), s);

				if (s < sizeof(StructureData))
				{
					return 0;
				}

				unsigned char *sData = new unsigned char[s];

				if (sfs::readEntireFile(sData, s, entry.path().string().c_str(), true) != sfs::noError)
				{
					return 0;
				}

				auto structure = (StructureData *)sData;

				for (int x = 0; x < structure->size.x; x++)
					for (int z = 0; z < structure->size.z; z++)
						for (int y = 0; y < structure->size.y; y++)
				{
					structure->unsafeGet(x, y, z).lightLevel = 0;
				}

				structures.push_back((StructureData *)sData);
			}
		}

		return 1;
	};

	loadFolder(RESOURCES_PATH "gameData/structures/trees", trees);
	loadFolder(RESOURCES_PATH "gameData/structures/jungleTrees", jungleTrees);
	loadFolder(RESOURCES_PATH "gameData/structures/palm", palmTrees);
	loadFolder(RESOURCES_PATH "gameData/structures/treeHouses", treeHouses);
	loadFolder(RESOURCES_PATH "gameData/structures/smallpyramid", smallPyramids);
	loadFolder(RESOURCES_PATH "gameData/structures/birch", birchTrees);
	loadFolder(RESOURCES_PATH "gameData/structures/igloo", igloos);
	loadFolder(RESOURCES_PATH "gameData/structures/spruce", spruceTrees);
	loadFolder(RESOURCES_PATH "gameData/structures/spruceSlim", spruceTreesSlim);
	loadFolder(RESOURCES_PATH "gameData/structures/smallStones", smallStones);
	loadFolder(RESOURCES_PATH "gameData/structures/tallTree", tallTreesSlim);
	
	if (trees.empty()) { return 0; }
	if (jungleTrees.empty()) { return 0; }
	if (palmTrees.empty()) { return 0; }
	if (treeHouses.empty()) { return 0; }
	if (smallPyramids.empty()) { return 0; }
	if (birchTrees.empty()) { return 0; }
	if (igloos.empty()) { return 0; }
	if (spruceTrees.empty()) { return 0; }

	return true;
}

void StructuresManager::clear()
{
	for (auto &i : trees)
	{
		unsigned char *d = (unsigned char*)i;
		delete[] d;
	}

	for (auto &i : jungleTrees)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : palmTrees)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : treeHouses)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : smallPyramids)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : birchTrees)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : igloos)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : spruceTrees)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : spruceTreesSlim)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : smallStones)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	for (auto &i : tallTreesSlim)
	{
		unsigned char *d = (unsigned char *)i;
		delete[] d;
	}

	*this = {};
}
