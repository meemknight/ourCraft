#pragma once
#include <gameplay/entity.h>
#include <gameplay/droppedItem.h>
#include <gameplay/zombie.h>
#include <unordered_map>


struct ServerEntityManager
{

	std::unordered_map<std::uint64_t, DroppedItemServer> droppedItems;
	std::unordered_map<std::uint64_t, ZombieServer> zombies;

	bool entityExists(std::uint64_t id);

};