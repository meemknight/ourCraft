#pragma once
#include <gameplay/zombie.h>
#include <gameplay/droppedItem.h>
#include <gameplay/pig.h>
#include <gameplay/player.h>
#include <gameplay/cat.h>
#include <gameplay/goblin.h>
#include <gameplay/trainingDummy.h>
#include <gameplay/scareCrow.h>
#include "repeat.h"


//!!!!!!!!!!! DONT FORGET TO ALSO UPDATE THIS ONE
#define EntitiesTypesCountMACRO 8
constexpr static unsigned int EntitiesTypesCount = EntitiesTypesCountMACRO;
#define REPEAT_FOR_ALL_ENTITIES(FN) REPEAT_8(FN)
//!!!!!!!!!!! ^ ALSO THIS ONE            ^^^^^
#define REPEAT_FOR_ALL_ENTITIES_NO_PLAYERS(FN) REPEAT_NO_0_8(FN)
//!!!!!!!!!!! ^ ALSO THIS ONE!							^^^^^

//CHECK ALL OF THIS FILE FOR CHANGES

//!!!!!!!!!!! v ALSO THIS ONE!							vvvvv
namespace EntityType
{
	enum
	{
		player = 0,
		droppedItems,
		zombies,
		pigs,
		cats,
		goblins,
		trainingDummy,
		scareCrow,
	};
};

inline unsigned char getEntityTypeFromEID(std::uint64_t eid)
{
	return (eid >> 56);
}

inline unsigned char getOnlyIdFromEID(std::uint64_t eid)
{
	return ((eid << 6) >> 6);
}

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
		else if constexpr (I == 5)
		{
			return &baseClass->goblins;
		}
		else if constexpr (I == 6)
		{
			return &baseClass->trainingDummy;
		}
		else if constexpr (I == 7)
		{
			return &baseClass->scareCrows;
		}

		static_assert(I >= 0 && I <= EntitiesTypesCount);
	}

	template<unsigned int I>
	bool removeEntityBasedOnBlockPosition(int x, unsigned char y, int z)
	{
		auto &container = *entityGetter<I>();
		auto id = fromBlockPosToEntityID(x, y, z, I);
		
		auto found = container.find(id);
		if (found != container.end())
		{
			container.erase(found);
			return true;
		}

		return false;
	};

	template<unsigned int I>
	void addEmptyEntityBasedOnBlockPosition(int x, unsigned char y, int z)
	{
		auto &container = *entityGetter<I>();
		auto id = fromBlockPosToEntityID(x, y, z, I);

		container[id] = {};
	};

};

//server
struct EntityData: public EntityGetter<EntityData>
{
	std::unordered_map <std::uint64_t, PlayerServer*> players;

	std::unordered_map<std::uint64_t, DroppedItemServer> droppedItems;
	std::unordered_map<std::uint64_t, ZombieServer> zombies;
	std::unordered_map<std::uint64_t, PigServer> pigs;
	std::unordered_map<std::uint64_t, CatServer> cats;
	std::unordered_map<std::uint64_t, GoblinServer> goblins;
	std::unordered_map<std::uint64_t, TrainingDummyServer> trainingDummy;
	std::unordered_map<std::uint64_t, ScareCrowServer> scareCrows;

};


struct EntityDataClient : public EntityGetter<EntityDataClient>
{
	std::unordered_map<std::uint64_t, PlayerClient> players;

	std::unordered_map<std::uint64_t, DroppedItemClient> droppedItems;
	std::unordered_map<std::uint64_t, ZombieClient> zombies;
	std::unordered_map<std::uint64_t, PigClient> pigs;
	std::unordered_map<std::uint64_t, CatClient> cats;
	std::unordered_map<std::uint64_t, GoblinClient> goblins;
	std::unordered_map<std::uint64_t, TrainingDummyClient> trainingDummy;
	std::unordered_map<std::uint64_t, ScareCrowClient> scareCrows;

};

bool canEntityBeHit(unsigned char entityType);