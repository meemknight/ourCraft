#include <gameplay/food.h>
#include <tickTimer.h>

Effects getItemEffects(Item &item)
{
	auto type = item.type;

	Effects ret;


	if (type == apple)
	{ ret.allEffects[Effects::Saturated].timerMs = 40 * 1000; }

	if (type == applePie)
	{
		ret.allEffects[Effects::Saturated].timerMs = 60 * 1000;
	}

	if (type == regenerationPotion)
	{ ret.allEffects[Effects::Regeneration].timerMs = 5 * 60 * 1000; } //5 minutes of regeneration
	
	if (type == poisonPotion)
	{ ret.allEffects[Effects::Poisoned].timerMs = 30 * 1000; } //30 secconds of poison

	if (type == shieldingPotion)
	{ ret.allEffects[Effects::Shielding].timerMs = 6 * 60 * 1000; } 


	return ret;
}

int getItemHealing(Item &item)
{
	auto type = item.type;

	if (type == apple)
	{
		return 25;
	}

	if (type == applePie)
	{

		return 50;

	}

	return 0;
}
