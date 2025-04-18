#include <gameplay/life.h>
#include <gameplay/entity.h>

//hit
int calculateDamage(Armour armour_, const WeaponStats &weaponStats, std::minstd_rand &rng
	, float hitCorectness, float critChanceBonus, bool unaware)
{
	float totalDamage = 0;

	float critChance = weaponStats.critChance;
	critChance += critChanceBonus * 0.2f;

	if (weaponStats.accuracy < 0)
	{
		critChance += weaponStats.getAccuracyNormalizedNegative(); //we heavily decrease crit chance
	}
	else
	{
		critChance += weaponStats.getAccuracyNormalizedNegative() / 5.f;
	}

	if (weaponStats.accuracy < 0)
	{
		critChance = glm::clamp(critChance, 0.f, 0.8f);
	}
	else
	{
		critChance = glm::clamp(critChance, 0.f, 0.9f); //never 100% chance
	}


	bool crit = getRandomChance(rng, critChance);

	if (unaware)
	{
		totalDamage += weaponStats.surprizeDamage;
		std::cout << "Surprize! ";
	}else if (crit)
	{
		totalDamage += weaponStats.critDamage;
		std::cout << "Crit! ";
	}
	else
	{
		totalDamage += weaponStats.damage;
		std::cout << "Normal! ";
	}
	
	float armour = armour_.armour;
	armour -= weaponStats.armourPenetration;
	armour = std::max(armour, 0.f);

	totalDamage -= armour;

	totalDamage *= 50.f/(50.f+armour);

	totalDamage = std::max(totalDamage, 0.f);

	return totalDamage * hitCorectness;
}
