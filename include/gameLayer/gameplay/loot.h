#pragma once
#include <gameplay/items.h>
#include <gameplay/entity.h>




struct LootEntry
{
	float chance = 1; //draw one item every chance picks. Lower means more common
	unsigned short minAmount = 1;
	unsigned short maxAmount = 1;

	Item item;
};


//bonus luck is [-100 100]
Item drawLoot(std::vector<LootEntry> &loot, std::minstd_rand &rng, float bonusLuck);

int getRandomLootNumber(int min, int max, std::minstd_rand &rng, float bonusLuck);






