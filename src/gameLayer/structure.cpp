#include <structure.h>
#include <safeSave.h>
#include <filesystem>

bool StructuresManager::loadAllStructures()
{

	auto loadFolder = [](const char *path, std::vector<StructureData*> &structures) 
	{
		auto it = std::filesystem::directory_iterator(path); //todo check errors
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

				structures.push_back((StructureData *)sData);
			}
		}
	};

	loadFolder(RESOURCES_PATH "gameData/structures/trees", trees);
	loadFolder(RESOURCES_PATH "gameData/structures/jungleTrees", jungleTrees);
	loadFolder(RESOURCES_PATH "gameData/structures/palm", palmTrees);
	loadFolder(RESOURCES_PATH "gameData/structures/treeHouses", treeHouses);
	loadFolder(RESOURCES_PATH "gameData/structures/smallpyramid", smallPyramids);
	loadFolder(RESOURCES_PATH "gameData/structures/birch", birchTrees);
	loadFolder(RESOURCES_PATH "gameData/structures/igloo", igloos);
	

	if (trees.empty()) { return 0; }
	if (jungleTrees.empty()) { return 0; }
	if (palmTrees.empty()) { return 0; }
	if (treeHouses.empty()) { return 0; }
	if (smallPyramids.empty()) { return 0; }
	if (birchTrees.empty()) { return 0; }
	if (igloos.empty()) { return 0; }

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

	*this = {};
}
