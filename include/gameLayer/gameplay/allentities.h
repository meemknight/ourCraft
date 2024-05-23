#pragma once
#include <gameplay/zombie.h>
#include <gameplay/droppedItem.h>
#include <gameplay/pig.h>
#include <gameplay/player.h>
#include <gameplay/cat.h>


constexpr static unsigned int EntitiesTypesCount = 5;


template<typename B>
struct EntityGetter
{

	template<unsigned int I>
	auto entityGetter()
	{
		B *baseClass = (B*)this;

		if constexpr (I == 0)
		{
			return &baseClass->players;
		}
		else if constexpr (I == 1)
		{
			return &baseClass->droppedItems;
		}
		else if constexpr (I == 2)
		{
			return &baseClass->zombies;
		}
		else if constexpr (I == 3)
		{
			return &baseClass->pigs;
		}
		else if constexpr (I == 4)
		{
			return &baseClass->cats;
		}

		static_assert(I >= 0 && I <= EntitiesTypesCount);
	}

};


struct EntityData: public EntityGetter<EntityData>
{
	std::unordered_map <std::uint64_t, PlayerServer*> players;

	std::unordered_map<std::uint64_t, DroppedItemServer> droppedItems;
	std::unordered_map<std::uint64_t, ZombieServer> zombies;
	std::unordered_map<std::uint64_t, PigServer> pigs;
	std::unordered_map<std::uint64_t, CatServer> cats;

};


struct EntityDataClient : public EntityGetter<EntityDataClient>
{
	std::unordered_map<std::uint64_t, PlayerClient> players;

	std::unordered_map<std::uint64_t, DroppedItemClient> droppedItems;
	std::unordered_map<std::uint64_t, ZombieClient> zombies;
	std::unordered_map<std::uint64_t, PigClient> pigs;
	std::unordered_map<std::uint64_t, CatClient> cats;


};