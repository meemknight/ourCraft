#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <gameplay/entity.h>
#include <multyPlayer/tick.h>
#include <chunkSystem.h>
#include <iostream>
#include <multyPlayer/enetServerFunction.h>
#include <deque>


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
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding
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
			othersDeleted, pathFinding);
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

	struct PositionAndEID
	{
		glm::dvec3 pos;
		std::uint64_t eid;
	};

	std::vector<PositionAndEID> playersPosition;

	for (auto &c : chunkCache.savedChunks)
	{
		for (auto &p : c.second->entityData.players)
		{
			playersPosition.push_back({p.second.getPosition(), p.first});
		}
	}


#pragma region calculate path finding


	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> pathFinding;

	{
		std::deque<PathFindingNode> queue;
		std::unordered_map<glm::ivec3, PathFindingNode> positions;

		auto checkSides = [&](PathFindingNode node, glm::ivec3 displacement)
		{
			auto found = positions.find(node.returnPos + displacement);
			if (found == positions.end())
			{
				auto b = chunkCache.getBlockSafe(node.returnPos + displacement);
				if (b && !b->isColidable())
				{

					auto bUp = chunkCache.getBlockSafe(node.returnPos + displacement + glm::ivec3(0, 1, 0));
					if (!bUp || !bUp->isColidable())
					{
						auto bDown = chunkCache.getBlockSafe(node.returnPos + displacement + glm::ivec3(0, -1, 0));
						if (bDown && bDown->isColidable())
						{

							PathFindingNode newEntry;
							newEntry.returnPos = node.returnPos;
							newEntry.level = node.level + 1;

							positions.emplace(node.returnPos + displacement, newEntry);

							if (node.level < 40)
							{
								newEntry.returnPos = node.returnPos + displacement;
								queue.push_back(newEntry);
							}

						}

					}

				}
			}
		};


		if (!playersPosition.empty())
		{

			glm::ivec3 pos = from3DPointToBlock(playersPosition.begin()->pos);

			PathFindingNode root;
			root.returnPos = pos;
			root.level = 0;

			queue.push_back(root);
			positions.emplace(pos, root);

			while (!queue.empty())
			{
				PathFindingNode node = queue.front();
				queue.pop_front();


				checkSides(node, {1,0,0});
				checkSides(node, {-1,0,0});
				checkSides(node, {0,0,1});
				checkSides(node, {0,0,-1});


			}

			pathFinding.emplace(playersPosition.begin()->eid, std::move(positions));
		}
	
	};


#pragma endregion



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

				bool rez = genericCallUpdateForEntity(e, deltaTime, chunkGetter, chunkCache, rng, othersDeleted,
					pathFinding);
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

