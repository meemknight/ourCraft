#pragma once



struct WeaponStats
{

	float critChance = 0.1; // 0 -> 0.8;
	float critDamage = 1.5; // 0.5 -> 3
	
	float dexterity = 1; // -10 -> 20;	//X
	float damage = 1; // 1 -> 99999;
	float speed = 1; // todo			//X
	float comboFrequency = 1; // todo	//X
	float armourPenetration = 1; // todo
	float accuracy = 0; // -10 -> 20;	//X
	float range = 3; // 1 -> 6			
	float knockBack = 3; // 0 -> 20

	void normalize();
	
	float getKnockBackNormalized();


};