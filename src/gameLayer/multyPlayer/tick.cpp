#include <multyPlayer/tick.h>
#include <chunkSystem.h>
#include <iostream>
#include <multyPlayer/enetServerFunction.h>


template <class T, class E>
void genericBroadcastEntityUpdateFromServerToPlayer(E &e, bool reliable,
	std::uint64_t currentTimer, int Packet_Type)
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
	ServerChunkStorer &chunkCache, std::minstd_rand &rng)
{
	float time = deltaTime;
	if constexpr (hasRestantTimer<decltype(e.second)>)
	{
		time = deltaTime + e.second.restantTime;
	}

	if (time > 0)
	{
		e.second.update(time, chunkGetter, chunkCache, rng);
	}

	if constexpr (hasRestantTimer<decltype(e.second)>)
	{
		e.second.restantTime = 0;
	}
};



void doGameTick(float deltaTime, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache, EntityData &orphanEntities, unsigned int seed)
{
	std::minstd_rand rng(seed);


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

		auto genericLoopOverEntities = [&](auto &container, 
			auto &orphanContainer, auto packetType, auto packetId)
		{
			for (auto it = container.begin(); it != container.end(); )
			{
				auto &e = *it;

				genericCallUpdateForEntity(e, deltaTime, chunkGetter, chunkCache, rng);
				auto newChunk = determineChunkThatIsEntityIn(e.second.getPosition());

				//todo this should take into acount if that player should recieve it
				genericBroadcastEntityUpdateFromServerToPlayer
					< decltype(packetType)>(e, false, currentTimer, packetId);

				if (initialChunk != newChunk)
				{
					orphanContainer.insert(
						{e.first, e.second});

					it = container.erase(it);
				}
				else
				{
					++it;
				}
			}
		};

		genericLoopOverEntities(entityData.droppedItems, orphanEntities.droppedItems,
			Packet_RecieveDroppedItemUpdate{}, headerClientRecieveDroppedItemUpdate);

		genericLoopOverEntities(entityData.zombies, orphanEntities.zombies,
			Packet_UpdateZombie{}, headerUpdateZombie);

		genericLoopOverEntities(entityData.pigs, orphanEntities.pigs,
			Packet_UpdatePig{}, headerUpdatePig);
	

	}

	
	auto resetEntitiesInTheirNewChunk = [&](auto &container, auto memberSelector)
	{

		for (auto it = container.begin();
			it != container.end();)
		{
			auto &e = *it;

			auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
			auto chunk = chunkCache.getChunkOrGetNull(pos.x, pos.y);

			if (chunk)
			{
				auto member = memberSelector(chunk->entityData);
				member->insert({e.first, e.second});
				//chunk->entityData.droppedItems.insert({e.first, e.second});
				it = container.erase(it);
			}
			else
			{
				//save entity to disk!
				it++;
			}
		}


	};
	
	
	resetEntitiesInTheirNewChunk(orphanEntities.droppedItems, 
		[](auto &entityData) { return &entityData.droppedItems; });

	resetEntitiesInTheirNewChunk(orphanEntities.zombies,
		[](auto &entityData) { return &entityData.zombies; });
	
	resetEntitiesInTheirNewChunk(orphanEntities.pigs,
		[](auto &entityData) { return &entityData.pigs; });

	//re set entities in their new chunk
	//for (auto it = orphanEntities.droppedItems.begin();
	//	it != orphanEntities.droppedItems.end();)
	//{
	//	auto &e = *it;
	//
	//	auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
	//	auto chunk = chunkCache.getChunkOrGetNull(pos.x, pos.y);
	//
	//	if (chunk)
	//	{
	//		chunk->entityData.droppedItems.insert({e.first, e.second});
	//		it = orphanEntities.droppedItems.erase(it);
	//	}
	//	else
	//	{
	//		//save entity to disk!
	//		it++;
	//	}
	//}


	//for (auto it = orphanEntities.zombies.begin();
	//	it != orphanEntities.zombies.end();)
	//{
	//	auto &e = *it;
	//
	//	auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
	//	auto chunk = chunkCache.getChunkOrGetNull(pos.x, pos.y);
	//
	//	if (chunk)
	//	{
	//		chunk->entityData.zombies.insert({e.first, e.second});
	//		it = orphanEntities.zombies.erase(it);
	//	}
	//	else
	//	{
	//		//save entity to disk!
	//		it++;
	//	}
	//}
	//
	//for (auto it = orphanEntities.pigs.begin();
	//	it != orphanEntities.pigs.end();)
	//{
	//	auto &e = *it;
	//
	//	auto pos = determineChunkThatIsEntityIn(e.second.getPosition());
	//	auto chunk = chunkCache.getChunkOrGetNull(pos.x, pos.y);
	//	if (chunk)
	//	{
	//		chunk->entityData.pigs.insert({e.first, e.second});
	//		it = orphanEntities.pigs.erase(it);
	//	}
	//	else
	//	{
	//		//save entity to disk!
	//		it++;
	//	}
	//}


#pragma endregion




	chunkCacheGlobal = 0;
}

