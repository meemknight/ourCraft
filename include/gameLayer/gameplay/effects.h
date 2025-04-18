#pragma once



struct Effect
{
	int timerMs = 0; 
	//todo change to unsigned short? but make sure the calculations dont overflow!!

	float getTimerInSecconds();
};

struct Effects
{
	enum EffectsNames
	{
		Saturated,
		Regeneration,
		Poisoned,
		Shielding,

		Effects_Count
	};

	Effect allEffects[Effects_Count] = {};

	void passTimeMs(int ms);

	void applyEffects(Effects &other);

	int getArmour() 
	{
		if (allEffects[Shielding].timerMs > 0)
		{
			return 8;
		}

		return 0;
	}
};


