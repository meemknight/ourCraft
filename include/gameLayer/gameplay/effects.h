#pragma once



struct Effect
{
	int timerMs = 0;

	float getTimerInSecconds();
};

struct Effects
{
	enum
	{
		Satiety,



		Effects_Count
	};

	Effect allEffects[Effects_Count] = {};

	void passTimeMs(int ms);

	void applyEffects(Effects &other);
};


