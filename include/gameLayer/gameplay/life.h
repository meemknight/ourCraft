#pragma once


struct Life
{
	Life() {};
	Life(int l) { life = l; maxLife = l; }

	unsigned short life = 0;
	unsigned short maxLife = 0;

	void sanitize() { if (life > maxLife) { life = maxLife; } }
};