#pragma once


struct Life
{
	Life() {};
	Life(int l) { life = l; maxLife = l; }

	short life = 0;
	short maxLife = 0;

	void sanitize() { if (life > maxLife) { life = maxLife; } }
};