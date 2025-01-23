#include "easing.h"


float lerp(float a, float b, float r)
{
	return a * (1 - r) + b * r;
}

float linearRemap(float val, float fromMin, float fromMax, float toMin, float toMax)
{
	float rez = val;
	rez -= fromMin;

	rez /= (fromMax - fromMin);
	//rez is between 0 and 1 now

	rez *= (toMax - toMin);
	rez += toMin;

	return rez;
}