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

void genericBroadcastEntityDeleteFromServerToPlayer(std::uint64_t eid, bool reliable)
{
	Packet packet;
	packet.header = headerRemoveEntity;

	Packet_RemoveEntity data;
	data.EID = eid;

	broadCast(packet, &data, sizeof(data),
		nullptr, reliable, channelEntityPositions);
}


template<class T>
bool genericCallUpdateForEntity(T &e,
	float deltaTime, ChunkData *(chunkGetter)(glm::ivec2),
	ServerChunkStorer &chunkCache, std::minstd_rand &rng, 
	std::unordered_set<std::uint64_t> &othersDeleted
	)
{
	float time = deltaTime;
	if constexpr (hasRestantTimer<decltype(e.second)>)
	{
		time = deltaTime + e.second.restantTime;
	}

	bool rez = 1;
	if (time > 0)
	{
		rez = e.second.update(time, chunkGetter, chunkCache, rng, e.first,
			othersDeleted);
	}

	if constexpr (hasRestantTimer<decltype(e.second)>)
	{
		e.second.restantTime = 0;
	}

	return rez;
};



template<class T, class U>
void genericResetEntitiesInTheirNewChunk(T &container, U memberSelector, ServerChunkStorer &chunkCache)
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
			//save entity to disk outside the thread.
			it++;
		}
	}
};


template <int... Is>
void callGenericResetEntitiesInTheirNewChunk(std::integer_sequence<int, Is...>, EntityData &c,
	ServerChunkStorer &chunkCache)
{
	(genericResetEntitiesInTheirNewChunk(*c.entityGetter<Is + 1>(),
		[](auto &entityData) { return entityData.entityGetter<Is + 1>(); },
		chunkCache), ...);
}



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




#pragma region entity updates

	std::unordered_set<std::uint64_t> othersDeleted;

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

				bool rez = genericCallUpdateForEntity(e, deltaTime, chunkGetter, chunkCache, rng, othersDeleted);
				auto newChunk = determineChunkThatIsEntityIn(e.second.getPosition());

				if (!rez)
				{

					genericBroadcastEntityDeleteFromServerToPlayer(it->first, true);

					//remove entity
					it = container.erase(it);
				}
				else
				{
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
				
			}
		};

		genericLoopOverEntities(entityData.droppedItems, orphanEntities.droppedItems,
			Packet_RecieveDroppedItemUpdate{}, headerClientRecieveDroppedItemUpdate);

		genericLoopOverEntities(entityData.zombies, orphanEntities.zombies,
			Packet_UpdateZombie{}, headerUpdateZombie);

		genericLoopOverEntities(entityData.pigs, orphanEntities.pigs,
			Packet_UpdatePig{}, headerUpdatePig);

	}

	for (auto eid : othersDeleted)
	{
		genericBroadcastEntityDeleteFromServerToPlayer(eid, true);

	}
	
	
	callGenericResetEntitiesInTheirNewChunk(std::make_integer_sequence<int, EntitiesTypesCount - 1>(),
		orphanEntities, chunkCache);


#pragma endregion




	chunkCacheGlobal = 0;
}

