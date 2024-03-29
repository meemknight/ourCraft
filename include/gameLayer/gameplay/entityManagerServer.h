#pragma once
#include <gameplay/entityManagerCommon.h>
#include <unordered_map>


struct ServerEntityManager
{

	std::unordered_map<std::uint64_t, DroppedItemNetworked> droppedItems;

	bool entityExists(std::uint64_t id);


};