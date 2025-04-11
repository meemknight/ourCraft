#include <gameplay/weaponStats.h>
#include <splines.h>
#include <glm/glm.hpp>
#include <iomanip>
#include <sstream>

void WeaponStats::normalize()
{


	critChance = glm::clamp(critChance, 0.f, 0.7f); // 0 -> 0.7;
	critDamage = glm::clamp(critDamage, 0.f, 999.f); // 
	surprizeDamage = glm::clamp(surprizeDamage, 0.f, 999.f); // 
	//dexterity = glm::clamp(dexterity, -10.f, 20.f); // -10 -> 20;
	damage = glm::clamp(damage, 1.f, 999.f); // 1 -> 999;
	accuracy = glm::clamp(accuracy, -10.f, 20.f); // -10 -> 20;
	range = glm::clamp(range, 1.f, 6.f); // 1 -> 6
	knockBack = glm::clamp(knockBack, 0.f, 30.f);
	speed = glm::clamp(speed, -10.f, 20.f);
	drawSpeed = glm::clamp(drawSpeed, -10.f, 20.f);
	//comboFrequency = glm::clamp(comboFrequency, -10.f, 20.f);
	armourPenetration = glm::clamp(armourPenetration, 0.f, 999.f);
	

}

float WeaponStats::getKnockBackNormalized()
{
	return glm::clamp(knockBack, 0.f, 30.f) / 30.f;
}

glm::vec2 WeaponStats
::getTimerCulldownRangeForAttacks()
{
	glm::vec2 ret = {0.8, 2.0};

	float normalizedSpeed = (speed + 10) / 30.f;
	normalizedSpeed = powf(normalizedSpeed, 2.f);
	normalizedSpeed = 1.2 + normalizedSpeed * 3;

	ret.x /= normalizedSpeed;
	ret.y /= normalizedSpeed;


	//ret.y = std::min(ret.y, 3.f);

	return ret;
}

float WeaponStats::getUIMoveSpeed()
{
	float s = (speed + 10) / 30.f;
	s = powf(speed, 0.5f);
	s *= 0.7f;
	return s + 0.6f;
}

float WeaponStats::getSpeedNormalizedInSecconds() const
{
	float normalizedSpeed = (speed + 10) / 30.f;
	normalizedSpeed = 1 - normalizedSpeed;
	normalizedSpeed = powf(normalizedSpeed, 2.f);
	normalizedSpeed = 0.1 + normalizedSpeed * 2.0;
	return normalizedSpeed;
}

float WeaponStats::getDrawSpeedNormalizedInSecconds() const
{
	float normalizedSpeed = (drawSpeed + 10) / 30.f;
	normalizedSpeed = 1 - normalizedSpeed;
	normalizedSpeed = powf(normalizedSpeed, 2.f);
	normalizedSpeed = 0.1 + normalizedSpeed * 2.0;
	return normalizedSpeed;
}

//float WeaponStats::getDexterityNormalized()
//{
//	return (dexterity + 10.f) / 30.f;
//}
//
//float WeaponStats::getComboFrequencyChance()
//{
//	float normalized = (comboFrequency + 10.f) / 30.f;
//	return normalized * 0.25f;
//}

float WeaponStats::getAccuracyAdjusted()
{
	float accuracyNormalized = (accuracy + 10.f) / 30.f;

	accuracyNormalized *= 1.0f;
	accuracyNormalized -= 0.5f;

	accuracyNormalized *= 0.5;

	return accuracyNormalized;
}

float WeaponStats::getAccuracyNormalized() const
{
	return (accuracy + 10.f) / 30.f;
}

float WeaponStats::getAccuracyNormalizedNegative() const
{
	return (accuracy) / 20.f;
}

std::string WeaponStats::formatDataToString() const
{

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(1);

	oss << "\nMele damage: " << (int)damage;
	oss << "\nCritical damage: " << (int)critDamage;
	oss << "\nCritical chance: " << ((int)(critChance * 100)) << "%";
	oss << "\nSurprise damage: " << (int)surprizeDamage;
	oss << "\nSpeed: " << getSpeedNormalizedInSecconds() << " secconds.";
	oss << "\nDraw speed: " << getDrawSpeedNormalizedInSecconds() << " secconds.";
	oss << "\nArmour penetration: " << (int)armourPenetration;
	oss << "\nKnockback: " << knockBack;
	oss << "\nRange: " << range;
	oss << "\nAccuracy: " << ((int)(getAccuracyNormalizedNegative() * 100)) << "%";


	return oss.str();
}

#include <gameplay/items.h>



WeaponStats Item::getWeaponStats()
{

	WeaponStats stats{};

	auto basicSword = [&](int damage)
	{
		stats = {};
		stats.damage = damage;
		stats.critDamage = damage * 1.5f;
		stats.surprizeDamage = damage * 2.f;
		stats.speed = 7;
		stats.drawSpeed = 3;
	};

	auto basicKnife = [&](int damage)
	{
		stats = {};
		stats.damage = damage;
		stats.critDamage = damage * 2.f;
		stats.surprizeDamage = std::max(damage * 4, 20);

		stats.range = 1;
		stats.knockBack = 1;
		stats.speed = 15;
		stats.drawSpeed = 15;
		stats.critChance = 0.2;

		stats.accuracy = 10;
	};

	auto basicScythe = [&](int damage)
	{
		stats = {};
		stats.damage = damage;
		stats.critDamage = damage * 2.f;
		stats.surprizeDamage = damage * 2.2f;
		stats.critChance = 0.15;

		stats.accuracy = 1;
		stats.armourPenetration = 0;

		stats.speed = 3;
		stats.drawSpeed = 1;
		stats.knockBack = 6;
		stats.range = 2.3;
	};

	auto basicSpear = [&](int damage)
	{
		stats = {};
		stats.damage = damage;
		stats.critDamage = damage * 1.5f;
		stats.surprizeDamage = damage * 2.f;
		stats.range = 3.3;
		stats.speed = 1;
		stats.drawSpeed = 0;
		stats.accuracy = 0;
		stats.knockBack = 6;
	};

	auto basicHammer = [&](int damage)
	{
		stats = {};
		stats.damage = damage;
		stats.critDamage = damage * 1.8f;
		stats.surprizeDamage = damage * 2.f;
		stats.range = 2;
		stats.speed = -4;
		stats.drawSpeed = -6;
		stats.accuracy = 1;
		stats.knockBack = 12;
		stats.range = 1.4;
		stats.armourPenetration = std::max(damage / 3, 5);
	};

	auto basicAxe = [&](int damage)
	{
		stats = {};
		stats.damage = damage;
		stats.critDamage = damage * 1.8f;
		stats.critChance = 0.12;
		stats.surprizeDamage = damage * 2.f;
		stats.range = 1.6;
		stats.speed = 0;
		stats.drawSpeed = -2;
		stats.accuracy = 3;
		stats.knockBack = 8;
		stats.armourPenetration = std::max(damage / 5, 2);
	};

	switch (type)
	{
	case trainingScythe:
	{
		basicScythe(7);
	}
	break;

	case trainingKnife: { basicKnife(3); }break;
	case copperKnife: { basicKnife(5); } break;
	case leadKnife: { basicKnife(6); } break;
	case ironKnife: { basicKnife(7); } break;
	case silverKnife: { basicKnife(8); } break;
	case goldKnife: { basicKnife(9); } break;


	case trainingSword: { basicSword(5); } break;
	case copperSword: { basicSword(7); } break;
	case leadSword: { basicSword(9); } break;
	case ironSword: { basicSword(12); } break;
	case silverSword: { basicSword(14); } break;
	case goldSword: { basicSword(16); } break;


	case trainingWarHammer: { basicHammer(8); } break;
	case copperWarHammer: { basicHammer(10); } break;
	case leadWarHammer: { basicHammer(12); } break;
	case ironWarHammer: { basicHammer(15); } break;
	case silverWarHammer: { basicHammer(17); } break;
	case goldWarHammer: { basicHammer(20); } break;

	case trainingSpear: { basicSpear(4); } break;
	case copperSpear: { basicSpear(6); } break;
	case leadSpear: { basicSpear(8); } break;
	case ironSpear: { basicSpear(10); } break;
	case silverSpear: { basicSpear(12); } break;
	case goldSpear: { basicSpear(14); } break;


	case trainingBattleAxe: { basicAxe(6); } break;
	case copperBattleAxe: { basicAxe(8); } break;
	case leadBattleAxe: { basicAxe(10); } break;
	case ironBattleAxe: { basicAxe(13); } break;
	case silverBattleAxe: { basicAxe(15); } break;
	case goldBattleAxe: { basicAxe(17); } break;

		//todo more damage, get to like 30 ish for swords!

	break;
	}

	stats.normalize();


	return stats;
}