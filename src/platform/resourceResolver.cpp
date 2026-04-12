#include <platform/resourceResolver.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace
{
	std::string toLowerCopy(std::string value)
	{
		std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
		{
			return static_cast<char>(std::tolower(c));
		});
		return value;
	}

	std::string toSnakeCase(const std::string &value)
	{
		std::string result;
		result.reserve(value.size() + 8);

		for (std::size_t i = 0; i < value.size(); ++i)
		{
			unsigned char current = static_cast<unsigned char>(value[i]);

			if (std::isupper(current))
			{
				if (!result.empty() && result.back() != '_')
				{
					result.push_back('_');
				}
				result.push_back(static_cast<char>(std::tolower(current)));
			}
			else
			{
				result.push_back(static_cast<char>(std::tolower(current)));
			}
		}

		return result;
	}

	std::pair<std::string, std::string> splitTextureSuffix(const std::string &stem)
	{
		for (auto suffix : {std::string("_n"), std::string("_s"), std::string("_b")})
		{
			if (stem.size() > suffix.size() && stem.rfind(suffix) == stem.size() - suffix.size())
			{
				return {stem.substr(0, stem.size() - suffix.size()), suffix};
			}
		}

		return {stem, {}};
	}

	std::string applyLegacyAlias(const std::string &baseStem)
	{
		static const std::unordered_map<std::string, std::string> aliases = {
			{"goldOre", "gold_ore"},
			{"copperOre", "copper_ore"},
			{"ironOre", "iron_ore"},
			{"silverOre", "iron_ore"},
			{"leadOre", "coal_ore"},
			{"pathBlock", "dirt_path_top"},
			{"stoneAndLog", "oak_log"},
			{"notClearGlass", "glass"},
			{"notClearGlass2", "glass"},
			{"glass2", "glass"},
			{"vitral1", "red_stained_glass"},
			{"vitral2", "blue_stained_glass"},
			{"strippedOakLog", "stripped_oak_log"},
			{"strippedBirchLog", "stripped_birch_log"},
			{"strippedSpruceLog", "stripped_spruce_log"},
			{"sprucePlank", "spruce_planks"},
			{"mossyCobblestone", "mossy_cobblestone"},
			{"cloth", "white_wool"},
			{"hardSandStone", "sandstone"},
			{"dungeonBricks", "stone_bricks"},
			{"volcanicHotRock", "magma"},
			{"vulcanicRock", "blackstone"},
			{"smoothStone", "smooth_stone"},
			{"limeStone", "end_stone"},
			{"smoothLimeStone", "end_stone_bricks"},
			{"marble", "quartz_block_side"},
			{"smoothMarble", "quartz_block_top"},
			{"marbleBricks", "quartz_bricks"},
			{"marblePillar", "quartz_pillar"},
			{"marbleTop", "quartz_pillar_top"},
			{"yellow_grass_block_top", "grass_block_top"},
			{"yellow_grass_block_side", "grass_block_side"},
			{"yellowGrass", "grass"},
			{"blueBricks", "prismarine_bricks"},
			{"bricksVariation", "bricks"},
			{"blueBricksVariation", "prismarine_bricks"},
			{"dungeonStone", "stone"},
			{"dungeonCobbleStone", "cobblestone"},
			{"dungeonSmoothStone", "smooth_stone"},
			{"dungeonPillar", "quartz_pillar"},
			{"dungeonPillarTop", "quartz_pillar_top"},
			{"dungeonBrickSkull", "chiseled_stone_bricks"},
			{"dungeonGlass", "glass"},
			{"crate", "oak_planks"},
			{"ladder", "oak_planks"},
			{"vines", "vine"},
			{"lamp", "glowstone"},
			{"woodenTorch", "torch"},
			{"hay", "hay_block_side"},
			{"hayTop", "hay_block_top"},
			{"trainingDummyBase", "stripped_oak_log"},
			{"goblinTorch", "torch"},
			{"workBench", "workbench"}
		};

		auto exact = aliases.find(baseStem);
		if (exact != aliases.end())
		{
			return exact->second;
		}

		auto snakeStem = toSnakeCase(baseStem);
		auto snake = aliases.find(snakeStem);
		if (snake != aliases.end())
		{
			return snake->second;
		}

		return {};
	}

	void addUniqueCandidate(std::vector<std::filesystem::path> &candidates, const std::filesystem::path &candidate)
	{
		if (candidate.empty()) { return; }
		auto normalized = candidate.lexically_normal();
		if (std::find(candidates.begin(), candidates.end(), normalized) == candidates.end())
		{
			candidates.push_back(normalized);
		}
	}

	std::vector<std::filesystem::path> buildCandidates(const std::filesystem::path &requested,
		const std::vector<std::filesystem::path> &extraCandidates)
	{
		std::vector<std::filesystem::path> candidates;
		addUniqueCandidate(candidates, requested);

		auto requestedString = requested.generic_string();
		auto texturePackMarker = requestedString.find("/texturePacks/");
		if (texturePackMarker != std::string::npos)
		{
			auto packRootEnd = requestedString.find('/', texturePackMarker + std::string("/texturePacks/").size());
			if (packRootEnd != std::string::npos && packRootEnd + 1 < requestedString.size())
			{
				auto fallback = requestedString.substr(0, texturePackMarker) + "/assets/" + requestedString.substr(packRootEnd + 1);
				addUniqueCandidate(candidates, fallback);
			}
		}

		auto addStemVariants = [&](const std::filesystem::path &path)
		{
			if (!path.has_filename()) { return; }

			auto stem = path.stem().string();
			auto extension = path.extension().string();
			auto split = splitTextureSuffix(stem);

			auto snakeStem = toSnakeCase(split.first);
			if (snakeStem != split.first)
			{
				addUniqueCandidate(candidates, path.parent_path() / (snakeStem + split.second + extension));
			}

			auto aliasStem = applyLegacyAlias(split.first);
			if (!aliasStem.empty())
			{
				addUniqueCandidate(candidates, path.parent_path() / (aliasStem + split.second + extension));
			}
		};

		addStemVariants(requested);

		for (const auto &candidate : extraCandidates)
		{
			addUniqueCandidate(candidates, candidate);
			addStemVariants(candidate);
		}

		return candidates;
	}

	bool tryResolveCaseInsensitive(const std::filesystem::path &candidate, std::filesystem::path &resolved)
	{
		if (candidate.empty()) { return false; }

		std::filesystem::path current = candidate.is_absolute() ? candidate.root_path() : std::filesystem::current_path();
		auto relative = candidate.is_absolute() ? candidate.lexically_relative(candidate.root_path()) : candidate;

		for (const auto &part : relative)
		{
			if (part == ".") { continue; }
			if (part == "..")
			{
				current /= part;
				continue;
			}

			auto direct = current / part;
			if (std::filesystem::exists(direct))
			{
				current = direct;
				continue;
			}

			if (!std::filesystem::exists(current) || !std::filesystem::is_directory(current))
			{
				return false;
			}

			auto wanted = toLowerCopy(part.string());
			bool found = false;

			for (const auto &entry : std::filesystem::directory_iterator(current))
			{
				if (toLowerCopy(entry.path().filename().string()) == wanted)
				{
					current = entry.path();
					found = true;
					break;
				}
			}

			if (!found)
			{
				return false;
			}
		}

		resolved = current.lexically_normal();
		return true;
	}
}

namespace resourceResolver
{
	bool tryResolvePath(const std::filesystem::path &requested, std::filesystem::path &resolved)
	{
		return tryResolvePath(requested, resolved, {});
	}

	bool tryResolvePath(const std::filesystem::path &requested, std::filesystem::path &resolved,
		const std::vector<std::filesystem::path> &extraCandidates)
	{
		for (const auto &candidate : buildCandidates(requested, extraCandidates))
		{
			if (std::filesystem::exists(candidate))
			{
				resolved = candidate.lexically_normal();
				return true;
			}

			if (tryResolveCaseInsensitive(candidate, resolved))
			{
				return true;
			}
		}

		return false;
	}

	void reportMissingPathOnce(const std::filesystem::path &requested)
	{
		static std::unordered_set<std::string> reported;
		auto key = requested.lexically_normal().string();
		if (reported.insert(key).second)
		{
			std::cout << "Missing resource: " << key << "\n";
		}
	}
}
