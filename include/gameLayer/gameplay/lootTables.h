#pragma once
#include <gameplay/loot.h>
#include <glm/vec2.hpp>
#include <vector>

struct LootTable
{
	std::vector<LootEntry> loot;
	glm::ivec2 money = {};

	float secondDropChange = 0;
	std::vector<LootEntry> loot2;

};



LootTable &getScareCrawLootTable();
LootTable &getEmptyLootTable();


