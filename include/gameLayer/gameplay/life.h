#pragma once
#include <gameplay/weaponStats.h>
#include <random>

struct Life
{
	Life() {};
	Life(int l) { life = l; maxLife = l; }

	short life = 0;
	short maxLife = 0;

	void sanitize() { if (life > maxLife) { life = maxLife; } }
};


struct Armour
{
	int armour = 0;
};


int calculateDamage(Armour armour, const WeaponStats &weaponStats, std::minstd_rand &rng);

