#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <gameplay/entity.h>
#include <multyPlayer/tick.h>
#include <chunkSystem.h>
#include <iostream>
#include <multyPlayer/enetServerFunction.h>
#include <deque>
#include <platformTools.h>


template <class T, class E>
void genericBroadcastEntityUpdateFromServerToPlayer(E &e, bool reliable,
	std::uint64_t currentTimer, int Packet_Type)
{

	Packet packet;
	packet.header = Packet_Type;

	T packetData;
	packetData.eid = e.first;

	if constexpr (hasGetDataToSend<decltype(e.second)>)
	{
		packetData.entity = e.second.getDataToSend();
	}else
	{
		packetData.entity = e.second.entity;
	}

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

void entityDeleteFromServerToPlayer(std::uint64_t clientToSend,
	std::uint64_t eid, bool reliable)
{

	auto client = getClientSafe(clientToSend);

	if (client)
	{
		entityDeleteFromServerToPlayer(*client, eid, reliable);
	}

}

void entityDeleteFromServerToPlayer(Client &client, 
	std::uint64_t eid, bool reliable)
{

	Packet packet;
	packet.header = headerRemoveEntity;

	Packet_RemoveEntity data;
	data.EID = eid;

	sendPacket(client.peer, packet, (const char *)&data, sizeof(data),
		reliable, channelEntityPositions);
}


template<class T>
bool genericCallUpdateForEntity(T &e,
	float deltaTime, ChunkData *(chunkGetter)(glm::ivec2),
	ServerChunkStorer &chunkCache, std::minstd_rand &rng, 
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition
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
		//todo pack things into a struct
		rez = e.second.update(time, chunkGetter, chunkCache, rng, e.first,
			othersDeleted, pathFinding, playersPosition);
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
	(genericResetEntitiesInTheirNewChunk(*c.template entityGetter<Is + 1>(),
		[](auto &entityData) { return entityData.template entityGetter<Is + 1>(); },
		chunkCache), ...);
}

//todo make sure a player can only be in only one tick
//chunkCache has only chunks that shouldn't be unloaded! And no null ptrs
void doGameTick(float deltaTime, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache, EntityData &orphanEntities,
	unsigned int seed)
{
	std::minstd_rand rng(seed);

	std::unordered_map<glm::ivec3, BlockType> modifiedBlocks;

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

	//todo also send to entity update, and use there
	std::unordered_map <std::uint64_t, PlayerServer *> allPlayers;
	std::unordered_map < std::uint64_t, Client> allClients;

#pragma region calculate all players

	for (auto &c : chunkCache.savedChunks)
	{
		auto &playersMap = c.second->entityData.players;
		for (auto &p : playersMap)
		{
			allPlayers.insert(p);
		}
	}

	{
		const auto &reff = getAllClientsReff();
		for (auto p : allPlayers)
		{
			auto found = reff.find(p.first);
			permaAssert(found != reff.end());
			allClients.insert(*found);
		}
	}

#pragma endregion


#pragma region to random tick update

	unsigned int randomTickSpeed = getRandomTickSpeed();

	for (auto &c : chunkCache.savedChunks)
	{
		for (int h = 0; h < CHUNK_HEIGHT / 16; h++)
		{
			for (int i = 0; i < randomTickSpeed; i++)
			{
				int x = rng() % 16;
				int y = rng() % 16;
				int z = rng() % 16;
				y += h * 16;

				//if the code crashes here, that means that we wrongly got a nullptr chunk!
				auto &b = c.second->chunk.unsafeGet(x,y,z);

				if (b.type == BlockTypes::dirt)
				{


				}
				else if (b.type == BlockTypes::grassBlock)
				{
					auto top = c.second->chunk.safeGet(x, y + 1, z);
					if (top && top->stopsGrassFromGrowingIfOnTop())
					{
						//update block
						b.type = BlockTypes::dirt;
						modifiedBlocks[{x + c.first.x * CHUNK_SIZE, y, z + c.first.y * CHUNK_SIZE}] = b.type;
						c.second->otherData.dirty = true;
					}

				}
			}
		}
	};




#pragma endregion


#pragma region calculate player positions
	std::unordered_map<std::uint64_t, glm::dvec3> playersPosition;

	for (auto &c : chunkCache.savedChunks)
	{
		for (auto &p : c.second->entityData.players)
		{
			playersPosition.insert({p.first, p.second->getPosition()});
		}
	}
#pragma endregion


#pragma region calculate path finding


	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> pathFinding;

	{
		std::deque<PathFindingNode> queue;
		std::unordered_map<glm::ivec3, PathFindingNode> positions;
		
		auto addNode = [&](PathFindingNode node, glm::ivec3 displacement)
		{
			PathFindingNode newEntry;
			newEntry.returnPos = node.returnPos;
			newEntry.level = node.level + 1;

			positions.emplace(node.returnPos + displacement, newEntry);

			if (node.level < 80)
			{
				newEntry.returnPos = node.returnPos + displacement;
				queue.push_back(newEntry);
			}
		};

		auto checkDown = [&](PathFindingNode node, glm::ivec3 disp) //-> bool
		{
			glm::ivec3 displacement = glm::ivec3(0, -1, 0) + disp;

			auto found = positions.find(node.returnPos + displacement);
			if (found == positions.end())
			{
				auto b = chunkCache.getBlockSafe(node.returnPos + displacement);
				if (b && !b->isColidable())
				{
					auto b2 = chunkCache.getBlockSafe(node.returnPos + glm::ivec3(0, -2, 0));
					if (b2 && b2->isColidable())
					{
						addNode(node, displacement);
					}
				}
				//else
				//{
				//	return true;
				//}
			}

			//return false;
		};

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
						auto bDown2 = chunkCache.getBlockSafe(node.returnPos + 
							displacement + glm::ivec3(0, -2, 0));

						if ((bDown && bDown->isColidable())
							|| (bDown2 && bDown2->isColidable())
							)
						{
							addNode(node, displacement);

							checkDown(node, displacement);
						}

					}

				}
			}
		};

		auto checkUp = [&](PathFindingNode node, glm::ivec3 displacement)
		{
			auto found = positions.find(node.returnPos + displacement);
			if (found == positions.end())
			{
				auto b = chunkCache.getBlockSafe(node.returnPos + displacement);
				if (b && !b->isColidable())
				{
					addNode(node, displacement);
				}
			}
		};


		if (!playersPosition.empty())
		{

			glm::ivec3 pos = from3DPointToBlock(playersPosition.begin()->second);

			//project players position down down
			for(int i=1; i<4; i++)
			{

				auto b = chunkCache.getBlockSafe(pos - glm::ivec3(0,i,0));

				if (!b) { break; }

				if (b->isColidable())
				{
					PathFindingNode root;
					root.returnPos = pos - glm::ivec3(0, i-1, 0);
					root.level = 0;

					queue.push_back(root);
					positions.emplace(pos - glm::ivec3(0, i-1, 0), root);
					break;
				}
			}

			while (!queue.empty())
			{
				PathFindingNode node = queue.front();
				queue.pop_front();

				checkSides(node, {1,0,0});
				checkSides(node, {-1,0,0});
				checkSides(node, {0,0,1});
				checkSides(node, {0,0,-1});

				auto bDown = chunkCache.getBlockSafe(node.returnPos + glm::ivec3(0,-1,0));
				if (bDown && bDown->isColidable())
				{
					checkUp(node, {0,1,0});
					//checkUp(node, {0,2,0});
					//checkUp(node, {0,3,0});
					//checkUp(node, {0,4,0});
					//checkUp(node, {0,5,0});
				}

			}

			pathFinding.emplace(playersPosition.begin()->first, std::move(positions));
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

				bool rez = genericCallUpdateForEntity(e, deltaTime, chunkGetter,
					chunkCache, rng, othersDeleted,
					pathFinding, playersPosition);
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

		genericLoopOverEntities(entityData.cats, orphanEntities.cats,
			Packet_UpdateCat{}, headerUpdateCat);

	}

	for (auto eid : othersDeleted)
	{
		genericBroadcastEntityDeleteFromServerToPlayer(eid, true);

	}
	
	
	callGenericResetEntitiesInTheirNewChunk(std::make_integer_sequence<int, EntitiesTypesCount - 1>(),
		orphanEntities, chunkCache);


#pragma endregion


#pragma region send packets


	//send new blocks
	if (!modifiedBlocks.empty())
	{

		//todo ring buffer
		Packet_PlaceBlocks *newBlocks = new Packet_PlaceBlocks[modifiedBlocks.size()];

		Packet packet;
		packet.cid = 0;
		packet.header = headerPlaceBlocks;

		int i = 0;
		for (auto &b : modifiedBlocks)
		{
			newBlocks[i].blockPos = b.first;
			newBlocks[i].blockType = b.second;
			i++;
		}

		for (auto it = allClients.begin(); it != allClients.end(); it++)
		{
			{
				sendPacket(it->second.peer, packet, (const char *)newBlocks, 
					sizeof(Packet_PlaceBlocks) *modifiedBlocks.size(), true, channelChunksAndBlocks);
			}
		}


		delete[] newBlocks;
	}



#pragma endregion



	chunkCacheGlobal = 0;
}

