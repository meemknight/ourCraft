#pragma once

#include <glm/vec3.hpp>
#include <random>
#include <gameplay/weaponStats.h>


//this is where we compute all hitting things!
//chance crit bonus is [0, 1]
template<class T>
void doHittingThings(T &e, glm::vec3 dir, glm::dvec3 attackerPosition,
	WeaponStats &weaponStats, std::uint64_t &wasKilled, std::minstd_rand &rng, std::uint64_t attackedId
	, float hitCorectness, float critChanceBonus)
{


	if constexpr (hasCanBeAttacked<decltype(e.entity)>
		&& (hasLife<decltype(e.entity)> || hasLife<decltype(e)>))
	{
		Life *life = 0;
		Armour armour = {};

	#pragma region get stuff
		//the players are stored differently
		if constexpr (std::is_same_v<T, PlayerServer>)
		{
			life = &e.newLife;
			armour = e.getArmour();
		}
		else
		{
			life = &e.entity.life;
			armour = e.entity.getArmour();
		}
	#pragma endregion

		int damage = calculateDamage(armour, weaponStats, rng,
			hitCorectness, critChanceBonus, e.isUnaware());

		//std::cout << "Damage: " << damage << "\n";

		if (damage >= life->life)
		{
			wasKilled = attackedId;
			life->life = 0;
		}
		else
		{
			life->life -= damage;
		}

		glm::vec3 hitDir = dir;
		hitDir += glm::vec3(0, 0.22, 0);
		{
			float l = glm::length(hitDir);
			if (l == 0) { hitDir = {0,-1,0}; }
			else { hitDir /= l; }
		}

		float knockBack = weaponStats.knockBack;

		//todo add knock back resistance
		//std::cout << "Attacked!\n";
		//std::cout << life->life << "\n";
		//std::cout << &life->life << "\n";

		knockBack = std::max(knockBack, 0.f);
		e.applyHitForce(hitDir * knockBack * 2.f);

		auto entityPos = e.getPosition();
		glm::dvec3 directionToAttacker = attackerPosition - entityPos;
		double l = glm::length(directionToAttacker);
		if (l) { directionToAttacker /= l; }

		e.signalHit(directionToAttacker);
	}
	else
	{
		return;
	}

}
