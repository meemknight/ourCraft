#pragma once
#include <glm/vec2.hpp>


struct WeaponStats
{

	float critChance = 0.1; // 0 -> 0.8;
	float critDamage = 1.5; // 0.5 -> 3
	
	float dexterity = 1; // -10 -> 20;	//X
	float damage = 1; // 1 -> 99999;
	float speed = 5; // 1 -> 10
	float comboFrequency = 1; // todo	//X
	float armourPenetration = 1; // todo
	float accuracy = 0; // -10 -> 20;	//X
	float range = 3; // 1 -> 6			
	float knockBack = 3; // 0 -> 20

	void normalize();
	
	float getKnockBackNormalized();

	//returns the culldown timer for the weapon
	//based on speed
	glm::vec2 getTimerCulldownRangeForAttacks();
};