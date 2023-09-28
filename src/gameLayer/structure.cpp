#include <structure.h>
#include <safeSave.h>
#include <filesystem>

bool StructuresManager::loadAllStructures()
{
	auto it = std::filesystem::directory_iterator(RESOURCES_PATH "gameData/structures/trees"); //todo check errors

	for (const auto &entry : it)
	{
		if (entry.is_regular_file())
		{
			size_t s = 0;

			const char *c = entry.path().string().c_str();

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

			trees.push_back((StructureData*)sData);

		}
	
	}

	if (trees.empty()) { return 0; }

	return true;
}

void StructuresManager::clear()
{
	for (auto &i : trees)
	{
		unsigned char *d = (unsigned char*)i;
		delete[] d;
	}

	*this = {};
}
