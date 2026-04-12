#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace resourceResolver
{
	bool tryResolvePath(const std::filesystem::path &requested, std::filesystem::path &resolved);
	bool tryResolvePath(const std::filesystem::path &requested, std::filesystem::path &resolved,
		const std::vector<std::filesystem::path> &extraCandidates);
	void reportMissingPathOnce(const std::filesystem::path &requested);
}
