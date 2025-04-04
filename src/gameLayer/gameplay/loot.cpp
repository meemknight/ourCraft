#include "gameplay/loot.h"


int getRandomLootNumber(int min, int max, std::minstd_rand &rng, float bonusLuck)
{

	int result = getRandomNumber(rng, min, max);

	if (bonusLuck == 0)
	{
		return result;
	}
	else
	{
		bonusLuck = glm::clamp(bonusLuck, -100.f, 100.f);

		int result2 = getRandomNumber(rng, min, max);
		int result3 = getRandomNumber(rng, min, max);
		int result4 = getRandomNumber(rng, min, max);

		int original = result;

		if (bonusLuck > 0)
		{
			if (result3 > result2) { result2 = result3; }
			if (result4 > result2) { result2 = result4; }

			if (result > result2)
			{
				return result;
			}
			else
			{
				return glm::mix(result, result2, sqrt((bonusLuck / 100.f)));
			}
		}
		else
		{
			if (result3 < result2) { result2 = result3; }
			if (result4 < result2) { result2 = result4; }

			if (result > result2)
				{ std::swap(result, result2); }

			return std::min(glm::mix(result2, result, -bonusLuck / 100.f), original);
		}
	}

}


Item drawLoot(std::vector<LootEntry> &loot, std::minstd_rand &rng, float bonusLuck)
{
	//bonusLuck is in percentage so 10 means 10% more luck, -10 means 10% negative luck, and 
	//100% would mean something like very high chance of getting a good item.

	if (loot.empty())
		return Item();

	// Calculate weights
	std::vector<float> weights;
	weights.reserve(loot.size());

	float totalWeight = 0.0f;
	float maxWeight = -1000000;
	float minWeight = 1000000;
	for (const auto &entry : loot)
	{
		// Base weight (inverted chance, lower chance = higher weight)
		float baseWeight = 1.0f / std::max(entry.chance, 0.001f); 

		// Optional rarity factor for bonusLuck effect
		//float rarityFactor = std::max(entry.chance, 0.001f);
		//float luckMultiplier = 1.0f + (bonusLuck * 0.01f * rarityFactor);
	
		if (baseWeight > maxWeight)
		{
			maxWeight = baseWeight;
		}

		if (baseWeight < minWeight)
		{
			minWeight = baseWeight;
		}

		weights.push_back(baseWeight);
		totalWeight += baseWeight;
	}

	if (bonusLuck > 0)
	{
		bonusLuck = glm::clamp(bonusLuck, 0.f, 100.f) / 100.f;

		float difference = maxWeight - minWeight;

		for (auto &w : weights)
		{
			float weightFlipped = w;
			
			//weightFlipped = (difference - (weightFlipped - minWeight)) + minWeight;
			weightFlipped = maxWeight - weightFlipped + minWeight; //simplified version


			w = glm::mix(w, weightFlipped, bonusLuck);
		}

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
