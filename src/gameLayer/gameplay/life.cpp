#include <gameplay/life.h>
#include <gameplay/entity.h>

int calculateDamage(Armour armour_, const WeaponStats &weaponStats, std::minstd_rand &rng)
{
	float totalDamage = 0;

	bool crit = getRandomChance(rng, weaponStats.critChance);

	if (crit)
	{
		totalDamage += weaponStats.damage * weaponStats.critDamage;
	}
	else
	{
		totalDamage += weaponStats.damage;
	}
	
	float armour = armour_.armour;
	armour -= weaponStats.armourPenetration;
	armour = std::max(armour, 0.f);
	
	armour *= 2;

	totalDamage -= armour;

	totalDamage = std::max(totalDamage, 0.f);

	return totalDamage;
}
