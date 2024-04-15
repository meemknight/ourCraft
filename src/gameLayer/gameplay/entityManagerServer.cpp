
#include <gameplay/entityManagerServer.h>


bool ServerEntityManager::entityExists(std::uint64_t id)
{

	if (droppedItems.find(id) != droppedItems.end()) { return true; }
	if (zombies.find(id) != zombies.end()) { return true; }

	return 0;
}
