#pragma once
#include <string>


//also shared for items!
struct EntityStats
{

	float runningSpeed = 0;


	//defence
	short armour = 0;
	short knockBackResistance = 0; //percentage
	short thorns = 0; //percentage up to 300

	//attack
	short meleDamage = 0;
	short meleAttackSpeed = 0;
	short critChance = 0;

	//special
	short stealthSound = 0;
	short stealthVisibility = 0;

	//other
	short luck = 0;

	short improvedMiningPower = 0;

	std::string formatDataToString();


	void add(EntityStats &other);
	void normalize();

};




