#include <gameplay/food.h>
#include <tickTimer.h>


Effects getItemEffects(Item &item, PlayerInventory &inventory)
{
	auto type = item.type;

	bool fruitEffectsLonger = 0;

	for (int i = PlayerInventory::EQUIPEMENT_START_INDEX; i < PlayerInventory::EQUIPEMENT_START_INDEX +
		PlayerInventory::MAX_EQUIPEMENT_SLOTS; i++)
	{
		auto item = inventory.getItemFromIndex(i, nullptr);
		if (item->type == fruitPeeler) { fruitEffectsLonger = 1; }
	}

	Effects ret;

	int FRUIT_BASE_SATURATION = 15 * 1000;

	if (type == apple)
	{ ret.allEffects[Effects::Saturated].timerMs = 40 * 1000; }

	if (type == applePie)
	{
		ret.allEffects[Effects::Saturated].timerMs = 60 * 1000;
	}

	if (type == regenerationPotion)
	{ ret.allEffects[Effects::Regeneration].timerMs = 8 * 60 * 1000; } //8 minutes of regeneration
	
	if(type == strawberry)
	{ 
		ret.allEffects[Effects::Regeneration].timerMs = 1 * 60 * 1000
			+ fruitEffectsLonger * (30 * 1000); 
		ret.allEffects[Effects::Saturated].timerMs = FRUIT_BASE_SATURATION;
	}

	if (type == poisonPotion)
	{ ret.allEffects[Effects::Poisoned].timerMs = 30 * 1000; } //30 secconds of poison

	if (type == shieldingPotion)
	{ ret.allEffects[Effects::Shielding].timerMs = 8 * 60 * 1000; } 


	for (int i = PlayerInventory::EQUIPEMENT_START_INDEX; i < PlayerInventory::EQUIPEMENT_START_INDEX +
		PlayerInventory::MAX_EQUIPEMENT_SLOTS; i++)
	{
		auto item = inventory.getItemFromIndex(i, nullptr);

		if (item->type == ItemTypes::gumBox)
		{
			ret.allEffects[Effects::Saturated].timerMs -= 5 * 1000;
			ret.allEffects[Effects::Saturated].timerMs = std::max(ret.allEffects[Effects::Saturated].timerMs, 0);
		}

	}


	return ret;
}

int getItemHealing(Item &item, PlayerInventory &inventory)
{
	auto type = item.type;

	int rez = 0;
	bool hasFruitPeeler = 0;

	for (int i = PlayerInventory::EQUIPEMENT_START_INDEX; i < PlayerInventory::EQUIPEMENT_START_INDEX +
		PlayerInventory::MAX_EQUIPEMENT_SLOTS; i++)
	{
		auto item = inventory.getItemFromIndex(i, nullptr);

		if (item->type == ItemTypes::vitamins)
		{
			rez += 15;
		}

		if (item->type == fruitPeeler) { hasFruitPeeler = 1; }


	}

	if (type == apple)
	{
		rez += 25;

		if (hasFruitPeeler)
		{
			rez += 10;
		}
	}

	if (type == applePie)
	{
		rez += 50;
	}

	return std::max(0, rez);
}
