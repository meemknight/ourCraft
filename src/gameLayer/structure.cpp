#include <structure.h>
#include <safeSave.h>
#include <filesystem>

bool StructuresManager::loadAllStructures()
{
	for (const char* path : structurePaths)
	{
		std::vector<StructureData*> structures;

		auto it = std::filesystem::directory_iterator(path);

		for (const auto& entry : it)
		{
			if (!entry.is_regular_file())
				continue;

			size_t s = 0;
			sfs::getFileSize(entry.path().string().c_str(), s);

			if (s < sizeof(StructureData))
				return false;

			unsigned char* sData = new unsigned char[s];
			if (sfs::readEntireFile(sData, s, entry.path().string().c_str(), true) != sfs::noError)
				return false;

			structures.push_back((StructureData*)sData);
		}
		allStructures.push_back(std::move(structures));
	}

	for (const auto& structures : allStructures)
	{
		if (structures.empty())
			return false;
	}

	return true;
}

void StructuresManager::clear()
{
	for (auto& structures : allStructures)
	{
		for (auto& structure : structures)
		{
			unsigned char* d = reinterpret_cast<unsigned char*>(structure);
			delete[] d;
		}
		structures.clear();
	}
	allStructures.clear();

	*this = {};
}
