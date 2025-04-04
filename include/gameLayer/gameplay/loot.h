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


Item drawRandomLoot(std::vector<LootEntry> &loot, std::minstd_rand &rng, float bonusLuck)
{
	//bonusLuck is in percentage so 10 means 10% more luck, -10 means 10% negative luck, and 
	//100% would mean something like very high chance of getting a good item.
	
	if (loot.empty())
		return Item();

	// Calculate weights
	std::vector<float> weights;
	weights.reserve(loot.size());

	float totalWeight = 0.0f;
	for (const auto &entry : loot)
	{
		// Base weight (inverted chance, lower chance = higher weight)
		float baseWeight = 1.0f / std::max(entry.chance, 0.001f); // prevent division by zero

		// Optional rarity factor for bonusLuck effect (can tune more)
		float rarityFactor = (1.0f / std::max(entry.chance, 0.001f));
		float luckMultiplier = 1.0f + (bonusLuck * 0.01f * rarityFactor);

		float finalWeight = baseWeight * luckMultiplier;
		weights.push_back(finalWeight);
		totalWeight += finalWeight;
	}

	// Pick random float in range [0, totalWeight)
	float pick = getRandomNumberFloat(rng, 0.0f, totalWeight);

	// Find the corresponding item
	float cumulative = 0.0f;
	for (size_t i = 0; i < loot.size(); ++i)
	{
		cumulative += weights[i];
		if (pick <= cumulative)
		{
			auto rez = loot[i].item;
			rez.sanitize();
			return rez;
		}
	}

	std::cout << "ERROR THE FALLBACK WAS REACHED IN LOOT FOR SOME REASON!\n";
	// Fallback (shouldn't happen)
	auto rez = loot[0].item;
	rez.sanitize();
	return rez;


}





