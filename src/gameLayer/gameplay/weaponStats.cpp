#include <gameplay/weaponStats.h>
#include <splines.h>
#include <glm/glm.hpp>


void WeaponStats::normalize()
{


	critChance = glm::clamp(critChance, 0.f, 0.8f); // 0 -> 0.8;
	critDamage = glm::clamp(critDamage, 0.5f, 3.f); // 0.5 -> 3
	dexterity = glm::clamp(dexterity, -10.f, 20.f); // -10 -> 20;
	damage = glm::clamp(damage, 1.f, 999.f); // 1 -> 999;
	accuracy = glm::clamp(accuracy, -10.f, 20.f); // -10 -> 20;
	range = glm::clamp(range, 1.f, 6.f); // 1 -> 6
	knockBack = glm::clamp(knockBack, 0.f, 20.f);
	speed = glm::clamp(speed, -10.f, 20.f);

	//speed = 1; // todo
	//comboFrequency = 1; // todo
	//armourPenetration = 1; // todo
	
	



}

float WeaponStats::getKnockBackNormalized()
{
	return glm::clamp(knockBack, 0.f, 20.f) / 20.f;
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

float WeaponStats::getDexterityNormalized()
{
	return (dexterity + 10.f) / 30.f;
}
