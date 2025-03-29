#pragma once
#include <glm/vec2.hpp>


struct WeaponStats
{

	float critChance = 0.1; // 0 -> 0.7;
	float critDamage = 15; // 1 -> 999;
	float surprizeDamage = 30; // 1 -> 999;
	
	//float dexterity = 1; // -10 -> 20;
	float damage = 10; // 1 -> 999;
	float speed = 1; // -10 -> 20
	//float comboFrequency = 1; // // -10 -> 20; //
	float armourPenetration = 1; // 0 -> 999;
	float accuracy = 0; // -10 -> 20; //increase enemy hit box + more chances to hit corectly
	float range = 2; // 1 -> 6			
	float knockBack = 4; // 0 -> 30

	void normalize();
	
	float getKnockBackNormalized();

	//returns the culldown timer for the weapon
	//based on speed
	glm::vec2 getTimerCulldownRangeForAttacks();

	//the spped that the game moves
	float getUIMoveSpeed();

	float getDexterityNormalized();

	float getComboFrequencyChance();

	//between -0.25 and 0.25, used for collisions!
	float getAccuracyAdjusted();

	float getAccuracyNormalized() const;

	float getAccuracyNormalizedNegative() const;

};