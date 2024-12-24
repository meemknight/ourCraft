#include <gameplay/effects.h>
#include "tickTimer.h"




float Effect::getTimerInSecconds()
{
	return timerMs/1000.f;
}


void Effects::passTimeMs(int ms)
{
	for (int i = 0; i < Effects_Count; i++)
	{
		allEffects[i].timerMs -= ms;

		if (allEffects[i].timerMs < 0) { allEffects[i].timerMs = 0; }
	}
}

void Effects::applyEffects(Effects &other)
{

	for (int i = 0; i < Effects_Count; i++)
	{
		if (other.allEffects[i].timerMs > allEffects[i].timerMs)
		{
			allEffects[i].timerMs = other.allEffects[i].timerMs;
		}
	}

}
