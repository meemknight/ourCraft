#include <multyPlayer/tick.h>
#include <chunkSystem.h>
#include <iostream>
#include <multyPlayer/enetServerFunction.h>


template <class T, int Packet_Type, class E>
void genericBroadcastEntityUpdateFromServerToPlayer(E &e, bool reliable,
	std::uint64_t currentTimer)
{

	Packet packet;
	packet.header = Packet_Type;

	T packetData;
	packetData.eid = e.first;
	packetData.entity = e.second.entity;
	packetData.timer = currentTimer;

	broadCast(packet, &packetData, sizeof(packetData),
		nullptr, reliable, channelEntityPositions);

}


template<class T>
void genericCallUpdateForEntity(T &e,
	float deltaTime, ChunkData *(chunkGetter)(glm::ivec2),
	ServerChunkStorer &chunkCache)
{
	float time = deltaTime;
	if constexpr (hasRestantTimer<decltype(e.second)>)
	{
		time = deltaTime + e.second.restantTime;
	}

	if (time > 0)
	{
		e.second.update(time, chunkGetter, chunkCache);
	}

	if constexpr (hasRestantTimer<decltype(e.second)>)
	{
		e.second.restantTime = 0;
	}
};



void doGameTick(float deltaTime, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache, EntityData &orphanEntities)
{

	static thread_local ServerChunkStorer *chunkCacheGlobal = 0;

	chunkCacheGlobal = &chunkCache;

	auto chunkGetter = [](glm::ivec2 pos) -> ChunkData *
	{
		auto c = chunkCacheGlobal->getChunkOrGetNull(pos.x, pos.y);
		if (c)
		{
			return &c->chunk;
		}
		else
		{
			return nullptr;
		}
	};

	//todo when a player joins, send him all of the entities


#pragma region entity updates



	for (auto &c : chunkCache.savedChunks)
	{	
		if (c.second->otherData.shouldUnload) { continue; }

		auto &entityData = c.second->entityData;

		auto initialChunk = c.first;

		for (auto it = entityData.droppedItems.begin(); it != entityData.droppedItems.end(); )
		{
			auto &e = *it;

			genericCallUpdateForEntity(e, deltaTime, chunkGetter, chunkCache);
			auto newChunk = determineChunkThatIsEntityIn(e.second.getPosition());

			//todo this should take into acount if that player should recieve it
			genericBroadcastEntityUpdateFromServerToPlayer
				< Packet_RecieveDroppedItemUpdate,
				headerClientRecieveDroppedItemUpdate>(e, false, currentTimer);

			if (initialChunk != newChunk)
			{
				orphanEntities.droppedItems.insert(
					{e.first, e.second});

				it = entityData.droppedItems.erase(it);
			}
			else
			{
				++it;
			}

		}

		for (auto it = entityData.zombies.begin(); it != entityData.zombies.end(); )
		{
			auto &e = *it;

			genericCallUpdateForEntity(e, deltaTime, chunkGetter, chunkCache);
			auto newChunk = determineChunkThatIsEntityIn(e.second.getPosition());

			//todo this should take into acount if that player should recieve it
			genericBroadcastEntityUpdateFromServerToPlayer
				< Packet_UpdateZombie,
				headerUpdateZombie>(e, false, currentTimer);

			if (initialChunk != newChunk)
			{
				orphanEntities.zombies.insert(
					{e.first, e.second});

				it = entityData.zombies.erase(it);

				//std::cout << "Orphaned!\n";
			}
			else
			{
				++it;
			}
		}

	}


	//re set entities in their new chunk
	for (auto it = orphanEntities.droppedItems.begin();
		it != orphanEntities.droppedItems.end();)
	{
		auto &e = *it;

		auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
		auto chunk = chunkCache.getChunkOrGetNull(pos.x, pos.y);

		if (chunk)
		{
			chunk->entityData.droppedItems.insert({e.first, e.second});
			it = orphanEntities.droppedItems.erase(it);
		}
		else
		{
			//save entity to disk!
			it++;
		}
	}


	for (auto it = orphanEntities.zombies.begin();
		it != orphanEntities.zombies.end();)
	{
		auto &e = *it;

		auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
		auto chunk = chunkCache.getChunkOrGetNull(pos.x, pos.y);

		if (chunk)
		{
			chunk->entityData.zombies.insert({e.first, e.second});
			it = orphanEntities.zombies.erase(it);
		}
		else
		{
			//save entity to disk!
			it++;
		}
	}


#pragma endregion




	chunkCacheGlobal = 0;
}

glm::ivec2 determineChunkThatIsEntityIn(glm::dvec3 position)
{
	return {divideChunk(position.x), divideChunk(position.z)};
}
