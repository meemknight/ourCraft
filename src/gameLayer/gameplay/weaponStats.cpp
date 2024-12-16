#include <gameplay/weaponStats.h>
#include <splines.h>
#include <glm/glm.hpp>


void WeaponStats::normalize()
{


	critChance = glm::clamp(critChance, 0.f, 0.8f); // 0 -> 0.8;
	critDamage = glm::clamp(critDamage, 0.5f, 3.f); // 0.5 -> 3
	dexterity = glm::clamp(dexterity, -10.f, 20.f); // -10 -> 20;
	damage = glm::clamp(damage, 1.f, 999.f); // 1 -> 99999;
	accuracy = glm::clamp(accuracy, -10.f, 20.f); // -10 -> 20;
	range = glm::clamp(range, 1.f, 6.f); // 1 -> 6
	knockBack = glm::clamp(knockBack, 0.f, 20.f);
	speed = glm::clamp(speed, 1.f, 10.f);

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
	glm::vec2 ret = {2, 15};

	ret.x /= speed;
	ret.y /= speed;

	ret.y = std::min(ret.y, 5.f);

	return ret;
}
