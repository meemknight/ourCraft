#include <gameplay/food.h>
#include <tickTimer.h>

Effects getItemEffects(Item &item)
{
	auto type = item.type;

	Effects ret;


	if (type == apple)
	{

		ret.allEffects[Effects::Satiety].timerMs = 60 * 1000;
			

	}



	return ret;
}

int getItemHealing(Item &item)
{
	auto type = item.type;

	if (type == apple)
	{

		return 20;

	}

	return 0;
}
