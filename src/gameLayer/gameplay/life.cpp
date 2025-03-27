#include <gameplay/life.h>
#include <gameplay/entity.h>

//hit
int calculateDamage(Armour armour_, const WeaponStats &weaponStats, std::minstd_rand &rng
	, float hitCorectness, float critChanceBonus, bool unaware)
{
	float totalDamage = 0;

	float critChance = weaponStats.critChance;
	critChance += critChanceBonus * 0.2f;
	critChance = glm::clamp(critChance, 0.f, 0.9f); //never 100% chance

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

	totalDamage *= 100.f/(100.f+armour);

	totalDamage = std::max(totalDamage, 0.f);

	return totalDamage * hitCorectness;
}
