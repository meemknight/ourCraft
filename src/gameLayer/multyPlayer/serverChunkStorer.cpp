#include "multyPlayer/serverChunkStorer.h"
#include <iostream>
#include <chunkSystem.h>
#include <gameplay/physics.h>

#include <platformTools.h>
#include <repeat.h>
#include <gameplay/blocks/blocksWithData.h>


template<class T>
void genericCheckEntitiesForCollisions(T &container, std::vector<ColidableEntry> &ret, glm::dvec3 position,
	glm::vec3 collider, std::uint64_t eidToIgnore)
{
	if constexpr (hasCanPushOthers<decltype(container[0].entity)>)
	{
		for (auto &e : container)
		{

			if (boxColide(position, collider, e.second.getPosition(), e.second.entity.getColliderSize())
				&& eidToIgnore != e.first
				)
			{
				ColidableEntry other;
				other.eid = e.first;
				other.position = e.second.getPosition();
				other.collider = e.second.entity.getColliderSize();

				if constexpr (hasGetColliderOffset<decltype(e.second.entity)>) { other.position += e.second.entity.getColliderOffset(); }


				ret.push_back(other);
			}

		}
	}
};

template <int... Is>
void callGenericCheckEntitiesForCollisions(std::integer_sequence<int, Is...>, 
	EntityData &entityData, std::vector<ColidableEntry> &ret, glm::dvec3 position,
	glm::vec3 collider, std::uint64_t eidToIgnore)
{

	(genericCheckEntitiesForCollisions(*entityData.template entityGetter<Is+1>(), ret, 
		position, collider, eidToIgnore), ...);
}



std::vector<ColidableEntry> ServerChunkStorer::getCollisionsListThatCanPush(glm::dvec3 position, glm::vec3 colider
	, std::uint64_t eidToIgnore)
{
	std::vector<ColidableEntry> ret;
	std::vector<SavedChunk *> chunks;
	chunks.reserve(9);
	ret.reserve(4);

	//todo optimize, make this function not check chunks and lists that can't possibly colide
	auto chunkPosition = determineChunkThatIsEntityIn(position);
	for (auto offset : *getChunkNeighboursOffsets())
	{
		glm::ivec2 pos = chunkPosition + offset;
		auto c = getChunkOrGetNull(pos.x, pos.y);
		if (c)
		{
			chunks.push_back(c);
		}
	}
	

	auto checkEntities = [&](auto container)
	{
		if constexpr (hasCanPushOthers<decltype(container[0].entity)>)
		{
			for (auto &e : container)
			{

				if (boxColide(position, colider, e.second.getPosition(), e.second.entity.getColliderSize())
					&& eidToIgnore != e.first
					)
				{
					ColidableEntry other;
					other.eid = e.first;
					other.position = e.second.getPosition();
					other.collider = e.second.entity.getColliderSize();
					if constexpr (hasGetColliderOffset<decltype(e.second.entity)>) { other.position += e.second.entity.getColliderOffset(); }
					ret.push_back(other);
				}

			}
		}
	};


	for (auto &c : chunks)
	{

		if constexpr (hasCanPushOthers<decltype(c->entityData.players[0]->entity)>)
		{
			for (auto &e : c->entityData.players)
			{

				if (boxColide(position, colider, e.second->getPosition(), e.second->entity.getColliderSize())
					&& eidToIgnore != e.first
					)
				{
					ColidableEntry other;
					other.eid = e.first;
					other.position = e.second->getPosition();
					other.collider = e.second->entity.getColliderSize();
					//if constexpr (hasGetColliderOffset<decltype(e.second->entity)>) { other.position += e.second->entity.getColliderOffset(); }
					ret.push_back(other);
				}

			}
		};


		callGenericCheckEntitiesForCollisions(std::make_integer_sequence<int, EntitiesTypesCount-1>(),
			c->entityData, ret, position, colider, eidToIgnore);


	}

	return ret;
}

#define ENTITY_ITERATE(X) genericIterate(*chunk.entityData.entityGetter<X>());
void ServerChunkStorer::removeEntityChunkPositionsForChunk(SavedChunk &chunk)
{

	auto genericIterate = [&](auto &container)
	{
		for (auto &p : container)
		{
			auto found = entityChunkPositions.find(p.first);

			if (found != entityChunkPositions.end())
			{
				entityChunkPositions.erase(found);
			}
		}
	};
	
	REPEAT_FOR_ALL_ENTITIES(ENTITY_ITERATE);

}
#undef ENTITY_ITERATE

SavedChunk *ServerChunkStorer::getChunkOrGetNull(int posX, int posZ)
{
	glm::ivec2 pos = {posX, posZ};
	auto foundPos = savedChunks.find(pos);

	if (foundPos != savedChunks.end())
	{
		return ((*foundPos).second);
	}
	else
	{
		return nullptr;
	}
}

//old version!
SavedChunk *ServerChunkStorer::getOrCreateChunk(int posX, int posZ,
	WorldGenerator &wg,
	StructuresManager &structureManager,
	BiomesManager &biomesManager, 
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	bool generateGhostAndStructures,
	std::vector<StructureToGenerate> *newStructuresToAdd, 
	WorldSaver &worldSaver, bool *wasGenerated, bool *wasLoaded)
{
	if (wasGenerated) { *wasGenerated = false; }
	if (wasLoaded) { *wasLoaded = false; }

	glm::ivec2 pos = {posX, posZ};
	auto foundPos = savedChunks.find(pos);
	if (foundPos != savedChunks.end())
	{

		permaAssertComment(foundPos->second, "chunk should not be nullpointer here!");
	
		return foundPos->second;
	
	}
	else
	{

		std::vector<StructureToGenerate> newStructures;
		newStructures.reserve(10); //todo cache up

		SavedChunk *rez = new SavedChunk;

		rez->chunk.x = posX;
		rez->chunk.z = posZ;

		PL::Profiler profiler;
		profiler.start();

		if (!worldSaver.loadChunk(rez->chunk))
		{
			//create new chunk!
			if (wasGenerated) { *wasGenerated = true; }

			generateChunk(rez->chunk, wg, structureManager, biomesManager, newStructures);
			rez->otherData.dirty = true;
		}
		else
		{
			profiler.end();

			//0.3 ms! for non zipped!
			//std::cout << "Loaded Chunk ms: " << profiler.rezult.timeSeconds * 1000 << "\n";

			if (wasLoaded) { *wasLoaded = true; }

			//we loaded a chunk so no need to dirty here.
			rez->otherData.dirty = false;
			rez->otherData.dirtyBlockData = false;
			//std::cout << "Loaded!\n";

			worldSaver.loadBlockData(*rez);
			if (rez->normalize())
			{
				rez->otherData.dirty = true;
				rez->otherData.dirtyBlockData = true;
			}

		}

		worldSaver.loadEntityData(rez->entityData, {posX, posZ});

		savedChunks[pos] = rez;


		//todo this part should be moved in world generator if possible...

		//generate big structures
		//todo this shouldn't contain loaded chunks
		std::vector<glm::ivec2> newCreatedChunks;
		newCreatedChunks.push_back({posX, posZ});

		//generateGhostAndStructures = false;

		//strucutres todo remove or something or remake...
		/*
		if (generateGhostAndStructures)
		{
			int metaChunkX = divideMetaChunk(posX);
			int metaChunkZ = divideMetaChunk(posZ);

			auto randValues = wg.whiteNoise->GetNoiseSet(metaChunkX, 0, metaChunkZ, 8, 1, 8);
			for (int i = 0; i < 8 * 8; i++)
			{
				randValues[i] += 1;
				randValues[i] /= 2;
			}
			int randomIndex = 0;

			auto generateTreeHouse = [&](glm::ivec2 rootChunk, glm::ivec2 inChunkPos,
				std::vector<glm::ivec3> *controllBlocks) -> bool
			{
				getOrCreateChunk(rootChunk.x + 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
				getOrCreateChunk(rootChunk.x - 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
				getOrCreateChunk(rootChunk.x + 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
				getOrCreateChunk(rootChunk.x - 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
				getOrCreateChunk(rootChunk.x + 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
				getOrCreateChunk(rootChunk.x - 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
				getOrCreateChunk(rootChunk.x, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
				getOrCreateChunk(rootChunk.x, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);

				newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y + 1});
				newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y - 1});
				newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y - 1});
				newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y + 1});
				newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y});
				newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y});
				newCreatedChunks.push_back({rootChunk.x, rootChunk.y + 1});
				newCreatedChunks.push_back({rootChunk.x, rootChunk.y - 1});



				//std::cout << "Generated root chunk: " << rootChunk.x << " " << rootChunk.y << "\n";

				auto newC = getOrCreateChunk(rootChunk.x, rootChunk.y, wg,
					structureManager, biomesManager, sendNewBlocksToPlayers, false, &newStructures, worldSaver);

				if (newC)
				{
					int y;
					for (y = CHUNK_HEIGHT - 20; y > 40; y--)
					{
						auto b = newC->chunk.unsafeGet(inChunkPos.x, y, inChunkPos.y);

						if (b.getType() == BlockTypes::grassBlock)
						{
							y++;
							break;
						}
						else if (b.getType() == BlockTypes::air || b.getType() == BlockTypes::grass)
						{

						}
						else
						{
							y = 0;
							break;
						}
					}

					if (y > 40)
					{
						//todo fill this
						std::unordered_set<glm::ivec2> newCreatedChunksSet;
						newCreatedChunksSet.reserve(30);


						newCreatedChunksSet.insert({});

						StructureToGenerate s;
						s.pos.x = rootChunk.x * CHUNK_SIZE + inChunkPos.x;
						s.pos.z = rootChunk.y * CHUNK_SIZE + inChunkPos.y;
						s.pos.y = y;
						s.randomNumber1 = randValues[randomIndex++]; //todo
						s.randomNumber2 = randValues[randomIndex++]; //todo
						s.randomNumber3 = randValues[randomIndex++]; //todo
						s.randomNumber4 = randValues[randomIndex++]; //todo
						s.replaceBlocks = true;
						s.type = Structure_TreeHouse;


						//if (generateStructure(s,
						//	structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, controllBlocks))
						//{
						//	std::cout << "Generated a jungle tree! : " << s.pos.x << " " << s.pos.z << "\n";
						//	return true;
						//}

					}
				}

				return 0;
			};

			//todo random chance
			if (true)
			{

				glm::ivec2 offset{2};

				glm::ivec2 rootChunk = glm::ivec2(metaChunkX, metaChunkZ) * META_CHUNK_SIZE;
				rootChunk += offset;

				//todo or load it
				auto *c = getChunkOrGetNull(rootChunk.x, rootChunk.y);


				if (c)
				{
					//no need to generate structures, they were already generated.
				}
				else
				{
					auto tryGenerateTreeHouseChild = [&](glm::ivec2 rootChunk, std::vector<glm::ivec3> *controllBlocks) -> bool
					{
						glm::ivec2 inChunkPos = {8,8};

						auto newC = getOrCreateChunk(rootChunk.x, rootChunk.y, wg,
							structureManager, biomesManager, sendNewBlocksToPlayers, false, &newStructures, worldSaver);
						newCreatedChunks.push_back(rootChunk);

						if (newC->chunk.unsafeGetCachedBiome(inChunkPos.x, inChunkPos.y) == 5)
						{

							if (generateTreeHouse(rootChunk, inChunkPos, controllBlocks))
							{



								return true;
							}
						}

						return 0;
					};

					glm::ivec2 inChunkPos = {8,8};

					auto newC = getOrCreateChunk(rootChunk.x, rootChunk.y, wg,
						structureManager, biomesManager, sendNewBlocksToPlayers, false, &newStructures, worldSaver);
					newCreatedChunks.push_back(rootChunk);

					auto fillControllBlocksWithLeaves = [&](glm::ivec3 pos)
					{
						auto b = tryGetBlockIfChunkExistsNoChecks(pos);
						assert(b);

						if (isControlBlock(b->getType()))
						{
							b->setType(BlockTypes::jungle_leaves);

							auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, 0));

							bUp->setType(BlockTypes::jungle_leaves);

							auto b2 = tryGetBlockIfChunkExistsNoChecks({pos.x,pos.y,pos.z - 1});
							auto b3 = tryGetBlockIfChunkExistsNoChecks({pos.x,pos.y,pos.z + 1});
							auto b4 = tryGetBlockIfChunkExistsNoChecks({pos.x - 1,pos.y,pos.z});
							auto b5 = tryGetBlockIfChunkExistsNoChecks({pos.x + 1,pos.y,pos.z});

							if (b2 && isControlBlock(b2->getType()))
							{
								b2->setType(BlockTypes::jungle_leaves);
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, -1));
								bUp->setType(BlockTypes::jungle_leaves);

							}
							else
							if (b3 && isControlBlock(b3->getType()))
							{
								b3->setType(BlockTypes::jungle_leaves);
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, +1));
								bUp->setType(BlockTypes::jungle_leaves);
							}
							else
							if (b4 && isControlBlock(b4->getType()))
							{
								b4->setType(BlockTypes::jungle_leaves);
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(-1, 1, 0));
								bUp->setType(BlockTypes::jungle_leaves);
							}
							else
							if (b5 && isControlBlock(b5->getType()))
							{
								b5->setType(BlockTypes::jungle_leaves);
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(+1, 1, 0));
								bUp->setType(BlockTypes::jungle_leaves);
							}
						}
					};

					auto getSides = [&](std::vector<glm::ivec3> &rootControllBlocks,
						glm::ivec3 &front, glm::ivec3 &back, glm::ivec3 &left, glm::ivec3 &right)
					{
						front = rootControllBlocks[0];
						back = rootControllBlocks[0];
						left = rootControllBlocks[0];
						right = rootControllBlocks[0];

						for (auto c : rootControllBlocks)
						{
							if (c.x > front.x)
							{
								front = c;
							}
							if (c.x < back.x)
							{
								back = c;
							}
							if (c.z > right.z)
							{
								right = c;
							}
							if (c.z < left.z)
							{
								left = c;
							}
						}
					};

					auto drawBridge = [&](glm::ivec3 to, glm::ivec3 from)
					{
						glm::dvec3 pos = to;

						int steps = 100;

						glm::dvec3 delta = glm::dvec3(from - to) * (1.0 / steps);

						for (int i = 0; i < steps; i++)
						{
							auto b = tryGetBlockIfChunkExistsNoChecks(pos);
							if (b)
							{
								b->setType(BlockTypes::jungle_planks);
							}
							pos += delta;
						}

						auto b = tryGetBlockIfChunkExistsNoChecks(from);
						if (b)
						{
							b->setType(BlockTypes::jungle_planks);
						}
					};

					auto drawDoubleBridge = [&](glm::ivec3 to, glm::ivec3 from)
					{

						glm::ivec3 neighbour1 = to;
						glm::ivec3 neighbour2 = from;

						auto tryNeighbour = [this](glm::ivec3 pos)
						{
							auto b = tryGetBlockIfChunkExistsNoChecks(pos);
							if (b && isControlBlock(b->getType()))
							{
								return true;
							}

							return false;
						};

						if (tryNeighbour(to + glm::ivec3(0, 0, 1))) { neighbour1 = to + glm::ivec3(0, 0, 1); }
						if (tryNeighbour(to + glm::ivec3(0, 0, -1))) { neighbour1 = to + glm::ivec3(0, 0, -1); }
						if (tryNeighbour(to + glm::ivec3(1, 0, 0))) { neighbour1 = to + glm::ivec3(1, 0, 0); }
						if (tryNeighbour(to + glm::ivec3(-1, 0, 0))) { neighbour1 = to + glm::ivec3(-1, 0, 0); }

						if (tryNeighbour(from + glm::ivec3(0, 0, 1))) { neighbour2 = from + glm::ivec3(0, 0, 1); }
						if (tryNeighbour(from + glm::ivec3(0, 0, -1))) { neighbour2 = from + glm::ivec3(0, 0, -1); }
						if (tryNeighbour(from + glm::ivec3(1, 0, 0))) { neighbour2 = from + glm::ivec3(1, 0, 0); }
						if (tryNeighbour(from + glm::ivec3(-1, 0, 0))) { neighbour2 = from + glm::ivec3(-1, 0, 0); }

						drawBridge(to, from);
						drawBridge(neighbour1, neighbour2);
					};

					auto biome = newC->chunk.unsafeGetCachedBiome(inChunkPos.x, inChunkPos.y);
					if (biome == 5)
					{
						std::vector<glm::ivec3> rootControllBlocks;

						if (generateTreeHouse(rootChunk, inChunkPos, &rootControllBlocks))
						{
							if (rootControllBlocks.size())
							{
								glm::ivec3 front = {};
								glm::ivec3 back = {};
								glm::ivec3 left = {};
								glm::ivec3 right = {};

								getSides(rootControllBlocks, front, back, left, right);

								{
									std::vector<glm::ivec3> childControlBlocks;
									if (tryGenerateTreeHouseChild({rootChunk.x - 2, rootChunk.y}, &childControlBlocks))
									{
										glm::ivec3 cfront = {};
										glm::ivec3 cback = {};
										glm::ivec3 cleft = {};
										glm::ivec3 cright = {};

										getSides(childControlBlocks, cfront, cback, cleft, cright);

										//front
										fillControllBlocksWithLeaves(cback);
										fillControllBlocksWithLeaves(cleft);
										fillControllBlocksWithLeaves(cright);

										drawDoubleBridge(cfront, back);
									}
									else
									{
										fillControllBlocksWithLeaves(back);
									}
								}

								{
									std::vector<glm::ivec3> childControlBlocks;
									if (tryGenerateTreeHouseChild({rootChunk.x + 2, rootChunk.y}, &childControlBlocks))
									{
										glm::ivec3 cfront = {};
										glm::ivec3 cback = {};
										glm::ivec3 cleft = {};
										glm::ivec3 cright = {};

										getSides(childControlBlocks, cfront, cback, cleft, cright);

										//back
										fillControllBlocksWithLeaves(cfront);
										fillControllBlocksWithLeaves(cleft);
										fillControllBlocksWithLeaves(cright);

										drawDoubleBridge(front, cback);
									}
									else
									{
										fillControllBlocksWithLeaves(front);
									}
								}

								{
									std::vector<glm::ivec3> childControlBlocks;
									if (tryGenerateTreeHouseChild({rootChunk.x, rootChunk.y - 2}, &childControlBlocks))
									{
										glm::ivec3 cfront = {};
										glm::ivec3 cback = {};
										glm::ivec3 cleft = {};
										glm::ivec3 cright = {};

										getSides(childControlBlocks, cfront, cback, cleft, cright);

										//right
										fillControllBlocksWithLeaves(cfront);
										fillControllBlocksWithLeaves(cleft);
										fillControllBlocksWithLeaves(cback);

										drawDoubleBridge(left, cright);
									}
									else
									{
										fillControllBlocksWithLeaves(left);
									}
								}

								{
									std::vector<glm::ivec3> childControlBlocks;
									if (tryGenerateTreeHouseChild({rootChunk.x, rootChunk.y + 2}, &childControlBlocks))
									{
										glm::ivec3 cfront = {};
										glm::ivec3 cback = {};
										glm::ivec3 cleft = {};
										glm::ivec3 cright = {};

										getSides(childControlBlocks, cfront, cback, cleft, cright);

										//left
										fillControllBlocksWithLeaves(cfront);
										fillControllBlocksWithLeaves(cright);
										fillControllBlocksWithLeaves(cback);

										drawDoubleBridge(cleft, right);
									}
									else
									{
										fillControllBlocksWithLeaves(right);
									}
								}
							}



							std::cout << "Generated village" << rootChunk.x * CHUNK_SIZE << " " << rootChunk.y * CHUNK_SIZE << "\n";
						}


					}
					else if (biome == 2 || biome == 1)
					{
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);

						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y - 1});
						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y - 1});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y});
						newCreatedChunks.push_back({rootChunk.x, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x, rootChunk.y - 1});

						int y;
						for (y = CHUNK_HEIGHT - 20; y > 40; y--)
						{
							auto b = newC->chunk.unsafeGet(inChunkPos.x, y, inChunkPos.y);

							if (b.getType() == BlockTypes::sand || b.getType() == BlockTypes::sand_stone)
							{
								y -= 6 * randValues[randomIndex++];
								break;
							}
							else if (b.getType() == BlockTypes::air || b.getType() == BlockTypes::grass)
							{

							}
							else
							{
								y = 0;
								break;
							}
						}

						if (y > 40)
						{
							//todo fill this
							std::unordered_set<glm::ivec2, Ivec2Hash> newCreatedChunksSet;
							newCreatedChunksSet.reserve(30);

							StructureToGenerate s;
							s.pos.x = rootChunk.x * CHUNK_SIZE + inChunkPos.x;
							s.pos.z = rootChunk.y * CHUNK_SIZE + inChunkPos.y;
							s.pos.y = y;
							s.randomNumber1 = randValues[randomIndex++];
							s.randomNumber2 = randValues[randomIndex++];
							s.randomNumber3 = randValues[randomIndex++];
							s.randomNumber4 = randValues[randomIndex++];
							s.replaceBlocks = true;
							s.type = Structure_Pyramid;

							if (generateStructure(s,
								structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, 0))
							{
								std::cout << "Generated a pyrmid: " << s.pos.x << " " << s.pos.z << "\n";
							}


						}

					}
					else if (biome == 8 || biome == 9)
					{

						getOrCreateChunk(rootChunk.x + 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x + 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x - 1, rootChunk.y, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x, rootChunk.y + 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);
						getOrCreateChunk(rootChunk.x, rootChunk.y - 1, wg, structureManager, biomesManager, sendNewBlocksToPlayers, 0, &newStructures, worldSaver);

						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y - 1});
						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y - 1});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x + 1, rootChunk.y});
						newCreatedChunks.push_back({rootChunk.x - 1, rootChunk.y});
						newCreatedChunks.push_back({rootChunk.x, rootChunk.y + 1});
						newCreatedChunks.push_back({rootChunk.x, rootChunk.y - 1});

						int y;
						for (y = CHUNK_HEIGHT - 20; y > 40; y--)
						{
							auto b = newC->chunk.unsafeGet(inChunkPos.x, y, inChunkPos.y);

							if (b.getType() == BlockTypes::snow_block)
							{
								break;
							}
							else if (b.getType() == BlockTypes::air || b.getType() == BlockTypes::grass)
							{

							}
							else
							{
								y = 0;
								break;
							}
						}

						if (y > 40)
						{
							//todo fill this
							std::unordered_set<glm::ivec2, Ivec2Hash> newCreatedChunksSet;
							newCreatedChunksSet.reserve(30);

							StructureToGenerate s;
							s.pos.x = rootChunk.x * CHUNK_SIZE + inChunkPos.x;
							s.pos.z = rootChunk.y * CHUNK_SIZE + inChunkPos.y;
							s.pos.y = y;
							s.randomNumber1 = randValues[randomIndex++];
							s.randomNumber2 = randValues[randomIndex++];
							s.randomNumber3 = randValues[randomIndex++];
							s.randomNumber4 = randValues[randomIndex++];
							s.replaceBlocks = true;
							s.type = Structure_Igloo;

							if (generateStructure(s,
								structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, 0))
							{
								std::cout << "Generated an igloo: " << s.pos.x << " " << s.pos.z << "\n";
							}


						}

					}
				}

			}

			FastNoiseSIMD::FreeNoiseSet(randValues);

		}

		*/

		if (generateGhostAndStructures)
		{
			std::unordered_set<glm::ivec2, Ivec2Hash> newCreatedChunksSet;
			newCreatedChunksSet.reserve(30);

			//ghost blocks
			for (auto &cp : newCreatedChunks)
			{
				auto c = getChunkOrGetNull(cp.x, cp.y);

				assert(c);

				if (c)
				{
					auto &d = c->chunk;
					if (placeGhostBlocksForChunk(d.x, d.z, d))
					{
						c->otherData.dirty = true;
					}
					newCreatedChunksSet.insert({d.x, d.z});
				}
			}

			//trees
			for (auto &s : newStructures)
			{
				//generateStructure(s, structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, 0);
			}

		}
		else
		{
			if (newStructuresToAdd)
			{
				for (auto &s : newStructures)
				{
					newStructuresToAdd->push_back(s);
				}
			}
		}

	

		return rez;

	}


	return nullptr;
}

SavedChunk *ServerChunkStorer::getOrCreateChunk(int posX, int posZ, 
	WorldGenerator &wg, StructuresManager &structureManager, 
	BiomesManager &biomesManager, std::vector<SendBlocksBack> &sendNewBlocksToPlayers, 
	WorldSaver &worldSaver, bool *wasGenerated, bool *wasLoaded)
{

	if (wasGenerated) { *wasGenerated = 0; }
	if (wasLoaded) { *wasLoaded = 0; }

#pragma region check if already generated
	glm::ivec2 pos = {posX, posZ};
	auto foundPos = savedChunks.find(pos);
	if (foundPos != savedChunks.end())
	{
		permaAssertCommentDevelopement(foundPos->second, "chunk should not be nullpointer here!");
		return foundPos->second;
	}
#pragma endregion

#pragma region check if you can load it from a file and if not create the geometry!

	SavedChunk *rez = new SavedChunk;

	rez->chunk.x = posX;
	rez->chunk.z = posZ;

	std::vector<StructureToGenerate> newStructures;
	newStructures.reserve(10); 

	//this are chunks that have been just generated, so they need structures!
	std::unordered_map<glm::ivec2, SavedChunk*, Ivec2Hash> newCreatedChunks;
	std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> newCreatedOrLoadedChunks;
	newCreatedChunks.reserve(10);
	newCreatedOrLoadedChunks.reserve(10);

	if (worldSaver.loadChunk(rez->chunk))
	{
		//we managed to load the chunk
		if (wasLoaded) { *wasLoaded = true; }

		//we loaded a chunk so no need to dirty here.
		rez->otherData.dirty = false;
		rez->otherData.dirtyBlockData = false;
		//std::cout << "Loaded!\n";

		worldSaver.loadBlockData(*rez);
		if (rez->normalize())
		{
			rez->otherData.dirty = true;
			rez->otherData.dirtyBlockData = true;
		}
	}
	else
	{
		//create new chunk!
		if (wasGenerated) { *wasGenerated = true; }

		generateChunk(rez->chunk, wg, structureManager, biomesManager, newStructures);
		rez->otherData.dirty = true;

		newCreatedChunks[glm::ivec2{posX, posZ}] = rez;
	}

	newCreatedOrLoadedChunks[{posX, posZ}] =  rez;


#pragma endregion


#pragma region place previous ghost blocks

	for (auto &c : newCreatedOrLoadedChunks)
	{
		auto &d = c.second->chunk;
		if (placeGhostBlocksForChunk(d.x, d.z, d))
		{
			c.second->otherData.dirty = true;
			c.second->otherData.dirtyBlockData = true; //probably not needed but just to be safe!
		}
	}

#pragma endregion

#pragma region place small structures

	for (auto &s : newStructures)
	{
		generateStructure(s, structureManager, newCreatedOrLoadedChunks, sendNewBlocksToPlayers, 0);
	}

#pragma endregion



#pragma region load entities

	for (auto &c : newCreatedChunks)
	{
		worldSaver.loadEntityData(c.second->entityData, c.first);
	}

#pragma endregion


#pragma region set newly loaded chunls

	for (auto &c : newCreatedOrLoadedChunks)
	{
		savedChunks[c.first] = c.second;
	}

#pragma endregion


	return rez;
}

bool ServerChunkStorer::generateStructure(StructureToGenerate s,
	StructureDataAndFlags &structure, int rotation,
	std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> &newCreatedOrLoadedChunks,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers, 
	std::vector<glm::ivec3> *controlBlocks, bool replace,
	BlockType from, BlockType to)
{
	auto size = structure.data->size;

	//the user can replace a block with another
	auto replaceB = [&](Block &b)
	{
		if (replace)
		{
			if (b.getType() == from)
			{
				b.setType(to);
			}
		}

		if (s.replaceLeavesWith && isAnyLeaves(b.getType()))
		{
			b.setType(s.replaceLeavesWith);
		}

		if (s.replaceLogWith && isAnyWoddenLOG(b.getType()))
		{
			b.setType(s.replaceLogWith);
		}
	};

	auto chooseRandomElement = [](float randVal, int elementCount)
	{
		return int(floor(randVal * elementCount));
	};


	int bonusRandomHeightTrees = 0;
	if (s.addRandomTreeHeight)
	{
		bonusRandomHeightTrees = chooseRandomElement(s.randomNumber4, 6) + 2;
	}

	auto placeOneBlockLogic = [&](Block oldBlock, Block &newBlock, int x, int y, int z, unsigned char replaceAnything,
		unsigned char oldBlockReplaceAnything,
		bool canPlaceAir) -> bool
	{

		if (replaceAnything < oldBlockReplaceAnything) { return false; }

		if (replaceAnything && canPlaceAir && newBlock.getType() == BlockTypes::air)
		{
			return true;
		}

		if (oldBlock.getType() == BlockTypes::air || replaceAnything)
		{
		
			//we replace the new block with a variation if needed.
			replaceB(newBlock);

			//basic paint logic

			if (s.paintLogicStuff)
			{
				if (newBlock.getType() == BlockTypes::birch_leaves)
				{

					int colors[] = 
					{	0,
						BlockColor::yellow,
						BlockColor::brown,
						BlockColor::brown,
						0,
						BlockColor::orange,
						BlockColor::orange,
						0,
						0,
						BlockColor::brown,
						0,
					};

					int choice = getRandomNumberFloat(x, y, z, 0, 0.99) * 
						(sizeof(colors)/sizeof(colors[0]));

					newBlock.setColor(colors[choice]);
				}
				else
				if (newBlock.getType() == BlockTypes::spruce_leaves_red)
				{

					int colors[] =
					{	0,
						BlockColor::brown,
						0,
						BlockColor::red,
						0,
						BlockColor::red,
						0,
						BlockColor::red,
						0,
					};

					int choice = getRandomNumberFloat(x, y, z, 0, 0.99) *
						(sizeof(colors) / sizeof(colors[0]));

					newBlock.setColor(colors[choice]);
				}

			};


			if (newBlock.getType() != BlockTypes::air)
			{

				//todo paint if needed.

				if (controlBlocks)
				{
					if (isControlBlock(newBlock.getType()))
					{
						//controlBlocks->push_back({x,y,z});
					}
				}

				return true;
			}

		}

		return false;
	};


	//todo re add this!
	bonusRandomHeightTrees = 0;
	
	
	if (s.pos.y + size.y + bonusRandomHeightTrees <= CHUNK_HEIGHT)
	{


		glm::ivec3 startPos = s.pos;

		if(bonusRandomHeightTrees)
		{

			int x = startPos.x;
			int z = startPos.z;

			int chunkX = divideChunk(x);
			int chunkZ = divideChunk(z);

			auto c = getChunkOrGetNull(chunkX, chunkZ);

			int inChunkX = modBlockToChunk(x);
			int inChunkZ = modBlockToChunk(z);

			if (c)
			{
				c->otherData.dirty = true;

				for (int y = s.pos.y; y < s.pos.y + bonusRandomHeightTrees; y++)
				{
					auto &b = c->chunk.unsafeGet(inChunkX, y, inChunkZ);
					if (b.getType() == BlockTypes::air)
					{
						b.setType(s.replaceLogWith);
					}
				}
			}
			else {} //we can assume that this chunk is loaded so no need for ghost blocks...

			startPos.y += bonusRandomHeightTrees;
		}

		startPos.x -= size.x / 2;
		startPos.z -= size.z / 2;
		glm::ivec3 endPos = startPos + size;

		for (int x = startPos.x; x < endPos.x; x++)
			for (int z = startPos.z; z < endPos.z; z++)
			{

				int chunkX = divideChunk(x);
				int chunkZ = divideChunk(z);

				int inChunkX = modBlockToChunk(x);
				int inChunkZ = modBlockToChunk(z);

				int replaceAnything = s.replaceOverAnything;
				bool sendDataToPlayers = 0;
				SavedChunk *c = nullptr;

				auto &collumFlags = structure.getPerCollomFlagsUnsafeRotated(x - startPos.x, z - startPos.z, rotation, structure.data->size);

				//CASE 1: we place the block in one of the newly created or loaded CHUNKS!
				auto it = newCreatedOrLoadedChunks.find({chunkX, chunkZ});
				if (it != newCreatedOrLoadedChunks.end())
				{
					//this is a new chunk so we know for sure no player could already have had this chunk!
					sendDataToPlayers = false;
					c = it->second;
				}
				else
				{
					sendDataToPlayers = true;
					c = getChunkOrGetNull(chunkX, chunkZ);
				}

				auto shouldReplaceAir = [&](int y)
				{
					bool replaceAir = 0;
					if (s.replaceEnclosedColumsWithAir && collumFlags.hasBlocks)
					{
						if (y - startPos.y > collumFlags.minMax.x && y - startPos.y < collumFlags.minMax.y)
						{
							replaceAir = true;
						}
					}
					//replaceAir = true; //TODO DEBUG TEST RREMOVE
					return replaceAir;
				};

				// IF we have a chunk, we start placing blocks.
				if (c)
				{
					c->otherData.dirty = true;


					for (int y = startPos.y; y < endPos.y; y++)
					{

						bool replaceAir = shouldReplaceAir(y);


						auto &b = c->chunk.unsafeGet(inChunkX, y, inChunkZ);

						auto oldBlock = b;
						auto newBlock = structure.data->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z, rotation);
						newBlock.rotate(rotation);


						if (placeOneBlockLogic(oldBlock, newBlock, x, y, z, replaceAnything, 0, replaceAir))
						{
							c->removeBlockWithData({inChunkX, y, inChunkZ}, oldBlock.getType());
							b = newBlock; //we set the new block!

							if (sendDataToPlayers)
							{
								SendBlocksBack sendB;
								sendB.pos = {x,y,z};
								sendB.blockInfo = b;
								sendNewBlocksToPlayers.push_back(sendB);
							}
							
						};

					}
				}
				else
				{

					//CASE 3: we don't have this chunk, so we add some ghost blocks!
					auto it = ghostBlocks.find({chunkX, chunkZ});

					if (it == ghostBlocks.end())
					{

						std::unordered_map<BlockInChunkPos, GhostBlock, BlockInChunkHash> rez;

						for (int y = startPos.y; y < endPos.y; y++)
						{
							bool replaceAir = shouldReplaceAir(y);

							auto oldBlock = Block{};
							auto newBlock = structure.data->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z, rotation);
							newBlock.rotate(rotation);

							if (newBlock.air() && replaceAnything)
							{
								int a = 0;
							}

							if (placeOneBlockLogic(oldBlock, newBlock, x, y, z, replaceAnything, 0, replaceAir))
							{
								GhostBlock ghostBlock;
								ghostBlock.block = newBlock;
								ghostBlock.replaceAnything = replaceAnything;

								if (newBlock.air())
								{
									int a = 0;
								}

								rez[{inChunkX, y, inChunkZ}] = ghostBlock;
							}

						}

						ghostBlocks[{chunkX, chunkZ}] = rez;

					}
					else
					{
						for (int y = startPos.y; y < endPos.y; y++)
						{

							bool replaceAir = shouldReplaceAir(y);

							int oldBlockReplaceAnything = 0;
							auto oldBlock = Block{};
							auto newBlock = structure.data->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z, rotation);
							newBlock.rotate(rotation);

							auto blockIt = it->second.find({inChunkX, y, inChunkZ});
							
							if (blockIt != it->second.end())
							{
								oldBlock = blockIt->second.block;
								oldBlockReplaceAnything = blockIt->second.replaceAnything;
							}

							bool allowed = 1;
							if (blockIt != it->second.end())
							{
								if (replaceAnything > blockIt->second.replaceAnything)
								{
									//good we can replace the block
								}
								else
								{
									allowed = 0;
								}
							}

							if(allowed)
							if (placeOneBlockLogic(oldBlock, newBlock, x, y, z, replaceAnything, oldBlockReplaceAnything, replaceAir))
							{

								GhostBlock ghostBlock;
								ghostBlock.block = newBlock;
								ghostBlock.replaceAnything = replaceAnything;

								if (newBlock.air())
								{
									int a = 0;
								}

								if (blockIt == it->second.end())
								{
									it->second[{inChunkX, y, inChunkZ}] = ghostBlock;
								}
								else
								{
									blockIt->second = ghostBlock;
								}

							}

						}
					}


				}

			}

		return true;
	}

	return 0;
}

bool ServerChunkStorer::generateStructure(StructureToGenerate s,
	StructuresManager &structureManager,
	std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> &newCreatedOrLoadedChunks,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers, 
	std::vector<glm::ivec3> *controlBlocks)
{
	auto chooseRandomElement = [](float randVal, int elementCount)
	{
		randVal = std::min(randVal, 0.999f);
		return int(floor(randVal * elementCount));
	};

	auto defaultGenerate = [&](std::vector<StructureDataAndFlags> &structure)
	{
		auto a = structure
			[chooseRandomElement(s.randomNumber1, structure.size())];
		return generateStructure(s, a, chooseRandomElement(s.randomNumber2, 4),
			newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks);
	};

	if (s.type == Structure_Tree)
	{

		auto tree = structureManager.trees
			[chooseRandomElement(s.randomNumber1, structureManager.trees.size())];

		return generateStructure(s, tree,
			chooseRandomElement(s.randomNumber2, 4), newCreatedOrLoadedChunks, sendNewBlocksToPlayers,
			controlBlocks);
	}
	else
	if (s.type == Structure_JungleTree)
	{

		auto tree = structureManager.jungleTrees
			[chooseRandomElement(s.randomNumber1, structureManager.jungleTrees.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks);
	}else if (s.type == Structure_PalmTree)
	{

		auto tree = structureManager.palmTrees
			[chooseRandomElement(s.randomNumber1, structureManager.palmTrees.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedOrLoadedChunks,
			sendNewBlocksToPlayers, controlBlocks);

	}else if (s.type == Structure_TreeHouse)
	{

		auto tree = structureManager.treeHouses
			[chooseRandomElement(s.randomNumber1, structureManager.treeHouses.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedOrLoadedChunks,
			sendNewBlocksToPlayers, controlBlocks);

	}else if (s.type == Structure_Pyramid)
	{

		auto tree = structureManager.smallPyramids
			[chooseRandomElement(s.randomNumber1, structureManager.smallPyramids.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4), newCreatedOrLoadedChunks,
			sendNewBlocksToPlayers, controlBlocks);

	}else if (s.type == Structure_BirchTree)
	{
		auto tree = structureManager.birchTrees
			[chooseRandomElement(s.randomNumber1, structureManager.birchTrees.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
			newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks);

	}else if (s.type == Structure_Igloo)
	{
		auto tree = structureManager.igloos
			[chooseRandomElement(s.randomNumber1, structureManager.igloos.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
			newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks);

	}
	else
	if (s.type == Structure_Spruce)
	{
		auto tree = structureManager.spruceTrees
			[chooseRandomElement(s.randomNumber1, structureManager.spruceTrees.size())];

		if (s.randomNumber3 > 0.5)
		{
			return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
				newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks, true,
				BlockTypes::spruce_leaves, BlockTypes::spruce_leaves_red);
		}
		else
		{
			return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
				newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks);
		}
	}else if (s.type == Structure_SpruceSlim)
	{
		auto tree = structureManager.spruceTreesSlim
			[chooseRandomElement(s.randomNumber1, structureManager.spruceTreesSlim.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
			newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks);
	}
	else if (s.type == Structure_SmallStone)
	{
		auto stone = structureManager.smallStones
			[chooseRandomElement(s.randomNumber1, structureManager.smallStones.size())];

		return generateStructure(s, stone, chooseRandomElement(s.randomNumber2, 4),
			newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks);
	}
	else if (s.type == Structure_TallSlimTree)
	{
		auto tree = structureManager.tallTreesSlim
			[chooseRandomElement(s.randomNumber1, structureManager.tallTreesSlim.size())];

		return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
			newCreatedOrLoadedChunks, sendNewBlocksToPlayers, controlBlocks);
	}
	else if (s.type == Structure_AbandonedHouse)
	{
		defaultGenerate(structureManager.abandonedHouse);
	}
	else if (s.type == Structure_GoblinTower)
	{
		defaultGenerate(structureManager.goblinTower);
	}
	else if (s.type == Structure_AbandonedTrainingCamp)
	{
		defaultGenerate(structureManager.trainingCamp);
	}


	return 0;
}

Block *ServerChunkStorer::getBlockSafe(glm::ivec3 pos)
{
	auto c = getChunkOrGetNull(divideChunk(pos.x), divideChunk(pos.z));

	if (c)
	{
		if (pos.y > 0 && pos.y < CHUNK_HEIGHT)
		{
			return &c->chunk.unsafeGet(modBlockToChunk(pos.x), pos.y, modBlockToChunk(pos.z));
		}
	}

	return nullptr;
}

Block *ServerChunkStorer::getBlockSafeAndChunk(glm::ivec3 pos, SavedChunk *&c)
{
	c = getChunkOrGetNull(divideChunk(pos.x), divideChunk(pos.z));

	if (c)
	{
		if (pos.y > 0 && pos.y < CHUNK_HEIGHT)
		{
			return &c->chunk.unsafeGet(modBlockToChunk(pos.x), pos.y, modBlockToChunk(pos.z));
		}
	}

	return nullptr;
}


Block *ServerChunkStorer::tryGetBlockIfChunkExistsNoChecks(glm::ivec3 pos)
{
	auto c = getChunkOrGetNull(divideChunk(pos.x), divideChunk(pos.z));

	if (c)
	{
		return &c->chunk.unsafeGet(modBlockToChunk(pos.x), pos.y, modBlockToChunk(pos.z));
	}

	return nullptr;
}

//given a chunk, we check if we have ghost blocks for it to be placed there
bool ServerChunkStorer::placeGhostBlocksForChunk(int posX, int posZ, ChunkData &c)
{
	bool placed = 0;

	auto iter = ghostBlocks.find({posX, posZ});

	if (iter != ghostBlocks.end())
	{

		for (auto &b : iter->second)
		{
			//the pos is in chunk space
			auto pos = b.first;

			auto &block = c.unsafeGet(pos.x, pos.y, pos.z);

			if (b.second.block.air())
			{
				int a = 0;
			}

			if (b.second.replaceAnything || block.getType() == BlockTypes::air)
			{
				//todo rotation and stuff like that
				//todo placed by server stuff
				block.typeAndFlags = b.second.block.typeAndFlags;
				block.colorAndOtherFlags = b.second.block.colorAndOtherFlags;
				
				placed = true;
			}
		}

		ghostBlocks.erase(iter);
	}

	return placed;
}



bool operator==(const BlockInChunkPos &a, const BlockInChunkPos &b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

std::array<glm::ivec2, 9> *getChunkNeighboursOffsets()
{
	static std::array<glm::ivec2, 9> checkOffsets = {
		glm::ivec2(0,0),
		glm::ivec2(1,0),
		glm::ivec2(-1,0),
		glm::ivec2(0,1),
		glm::ivec2(0,-1),
		glm::ivec2(1,-1),
		glm::ivec2(-1,-1),
		glm::ivec2(1,1),
		glm::ivec2(-1,1),
	};

	return &checkOffsets;
}



void ServerChunkStorer::cleanup()
{
	for (auto &c : savedChunks)
	{
		delete c.second;
	}
	*this = {};
}


bool ServerChunkStorer::saveNextChunk(WorldSaver &worldSaver, int count, int entitySaver)
{
	int blockDataCounter = count + 1; //we can afford to save a little more blockData stuff

	bool succeeded = 0;
	for (auto &c : savedChunks)
	{
		if (c.second->otherData.dirty && count > 0)
		{
			//std::cout << "Saved next chunk!\n";
			saveChunk(worldSaver, c.second);

			if (c.second->otherData.dirtyBlockData)
			{
				saveChunkBlockData(worldSaver, c.second);
				blockDataCounter--;
			}

			succeeded = true;

			count--;
		}
		else if (c.second->otherData.dirtyEntity && entitySaver > 0)
		{
			worldSaver.saveEntitiesForChunk(*c.second);
			c.second->otherData.dirtyEntity = 0;
			succeeded = true;

			entitySaver--;
		}
		else if (c.second->otherData.dirtyBlockData && blockDataCounter > 0)
		{
			saveChunkBlockData(worldSaver, c.second);
			blockDataCounter--;
			succeeded = true;
		}
		else if(count <= 0 && entitySaver <= 0 && blockDataCounter <= 0)
		{ break; }
	}

	return succeeded;
}



void ServerChunkStorer::saveChunk(WorldSaver &worldSaver, SavedChunk *savedChunks)
{
	worldSaver.saveChunk(savedChunks->chunk);
	worldSaver.saveEntitiesForChunk(*savedChunks);
	savedChunks->otherData.dirty = false;
	savedChunks->otherData.dirtyEntity = false;
}

void ServerChunkStorer::saveChunkBlockData(WorldSaver &worldSaver, SavedChunk *savedChunks)
{
	worldSaver.saveChunkBlockData(*savedChunks);
	savedChunks->otherData.dirtyBlockData = false;
}



void ServerChunkStorer::saveAllChunks(WorldSaver &worldSaver)
{
	for (auto &c : savedChunks)
	{
		if (c.second->otherData.dirty)
		{
			//std::cout << "Saved chunk\n";
			saveChunk(worldSaver, c.second);
		}
		else
		{
			//std::cout << "Saved entities\n";
			worldSaver.saveEntitiesForChunk(*c.second);
			c.second->otherData.dirtyEntity = false;
		}
	}
}

int ServerChunkStorer::unloadChunksThatNeedUnloading(WorldSaver &worldSaver, int count)
{
	//todo optimize, this seems to take an unnecessarily long time
	int unloaded = 0;
	for (auto it = savedChunks.begin(); it != savedChunks.end(); )
	{
		auto &c = *it;
		if (c.second->otherData.shouldUnload)
		{

			removeEntityChunkPositionsForChunk(*c.second);

			if (c.second->otherData.dirty)
			{
				saveChunk(worldSaver, c.second);
			}

			delete c.second;
			it = savedChunks.erase(it); // Erase the element

			unloaded++;
			if (unloaded >= count) break;
		}
		else
		{
			it++;
		}
	}

	return unloaded;
}

bool ServerChunkStorer::entityAlreadyExists(std::uint64_t eid)
{
	//todo
	//implement!

	return false;
}




template<class T>
std::uint64_t genericCheckEntitiesForCollisionWithBlock(T &container, glm::ivec3 position)
{
	if constexpr (hasCollidesWithPlacedBlocks<decltype(container[0].entity)>)
	{

		for (auto &e : container)
		{
			glm::dvec3 position = e.second.getPosition();

			if constexpr (hasGetColliderOffset<decltype(e.second.entity)>) { position += e.second.entityBuffered.getColliderOffset(); }

			auto rez = boxColideBlock(position, e.second.entity.getColliderSize(), position);
			
			if (rez)
			{
				return rez;
			}
		}
	}

	return 0;
};

template <int... Is>
std::uint64_t callGenericCheckEntitiesForCollisionWithBlock(std::integer_sequence<int, Is...>,
	EntityData &entityData, glm::ivec3 position)
{

	if constexpr (hasCollidesWithPlacedBlocks<decltype(entityData.players[0]->entity)>)
	{
		for (auto &e : entityData.players)
		{
			if (e.second)
			{

				glm::dvec3 position = e.second->getPosition();
				if constexpr (hasGetColliderOffset<decltype(e.second->entity)>) { position += e.second->entityBuffered.getColliderOffset(); }


				auto rez = boxColideBlock(position, e.second->entity.getColliderSize(), position);

				if (rez)
				{
					return rez;
				}
			}
		}
	}

	std::uint64_t result = 0;

	// Lambda function to evaluate and capture the result
	auto evaluate = [&](auto &entity) -> bool
	{
		result = genericCheckEntitiesForCollisionWithBlock(entity, position);
		return result != 0;
	};

	// Fold expression with short-circuiting
	(evaluate(*entityData.template entityGetter<Is + 1>()) || ...);
	return result;

	//return (genericCheckEntitiesForCollisionWithBlock(*entityData.template entityGetter<Is + 1>(),
	//	position) || ...);

}


std::uint64_t ServerChunkStorer::anyEntityIntersectsWithBlock(glm::ivec3 position)
{
	
	SavedChunk *chunksToCheck[9] = {};

	glm::ivec2 chunkPos{divideChunk(position.x), divideChunk(position.z)};
	

	chunksToCheck[0] = getChunkOrGetNull(chunkPos.x, chunkPos.y);

	//todo
	chunksToCheck[1] = getChunkOrGetNull(chunkPos.x+1, chunkPos.y);
	chunksToCheck[2] = getChunkOrGetNull(chunkPos.x-1, chunkPos.y);
	chunksToCheck[3] = getChunkOrGetNull(chunkPos.x, chunkPos.y+1);
	chunksToCheck[4] = getChunkOrGetNull(chunkPos.x, chunkPos.y-1);
	chunksToCheck[5] = getChunkOrGetNull(chunkPos.x+1, chunkPos.y+1);
	chunksToCheck[6] = getChunkOrGetNull(chunkPos.x-1, chunkPos.y-1);
	chunksToCheck[7] = getChunkOrGetNull(chunkPos.x+1, chunkPos.y-1);
	chunksToCheck[8] = getChunkOrGetNull(chunkPos.x-1, chunkPos.y+1);


	for (int i = 0; i < 9; i++)
	{

		if (chunksToCheck[i])
		{
			auto rez = callGenericCheckEntitiesForCollisionWithBlock
			(std::make_integer_sequence<int, EntitiesTypesCount - 1>(), chunksToCheck[i]->entityData,
				position);

			if (rez) { return rez; }
		}
	}

	return std::uint64_t(0);
}


template<class T>
bool genericRemoveEntity(T &container, std::uint64_t eid, std::unordered_map<std::uint64_t, glm::ivec2> &entityChunkPositions)
{
	auto found = container.find(eid);

	if (found != container.end())
	{
		container.erase(found);


		auto found2 = entityChunkPositions.find(eid);

		permaAssertComment(found2 != entityChunkPositions.end(), "Error, desync in entityChunkPositions");

		if (found2 != entityChunkPositions.end())
		{
			entityChunkPositions.erase(found2);
		}

		return 1;
	}

	return 0;
};

#define CASE_REMOVE(x) case x: { return genericRemoveEntity(*entityData.template entityGetter<x>(), eid, entityChunkPositions); } break;
bool callGenericRemoveEntity(EntityData &entityData, std::uint64_t eid, 
	std::unordered_map<std::uint64_t, glm::ivec2> &entityChunkPositions)
{
	auto entityType = getEntityTypeFromEID(eid);

	switch (entityType)
	{
		//REPEAT(CASE_REMOVE, 5);
		REPEAT_FOR_ALL_ENTITIES(CASE_REMOVE);

		//CASE_REMOVE(0);

		//case 0: { return genericRemoveEntity(*entityData.template entityGetter<0>(), eid); } break;
		//case 1: { return genericRemoveEntity(*entityData.template entityGetter<1>(), eid); } break;
		//case 2: { return genericRemoveEntity(*entityData.template entityGetter<2>(), eid); } break;
		//case 3: { return genericRemoveEntity(*entityData.template entityGetter<3>(), eid); } break;
		//case 4: { return genericRemoveEntity(*entityData.template entityGetter<4>(), eid); } break;
		//case 5: { static_assert(5 == EntitiesTypesCount); }

	default:;
	}

	return 0;
}
#undef CASE_REMOVE

//this function will just return 0 for players!
bool ServerChunkStorer::removeEntity(WorldSaver &worldSaver, std::uint64_t eid)
{
	auto entityType = getEntityTypeFromEID(eid);
	
	//this function will just return 0 for players!
	if (eid == EntityType::player) { return 0; }


	for (auto &c :savedChunks)
	{
		if (c.second)
		{

			bool rez = callGenericRemoveEntity(c.second->entityData, eid, entityChunkPositions);
			if (rez) { return 1; }
		}
	}

	return 0;
}


//this is where we compute all hitting things!
template<class T>
void doHittingThings(T &e, glm::vec3 dir, glm::dvec3 playerPosition, 
	Item &weapon, std::uint64_t &wasKilled, std::minstd_rand &rng, std::uint64_t currentId
	, float hitCorectness, float critChanceBonus)
{

	if constexpr (hasCanBeAttacked<decltype(e.entity)>
		&& hasLife<decltype(e.entity)>)
	{
		Life *life = 0;
		Armour armour = {};

	#pragma region get stuff
		//the players are stored differently
		if constexpr (std::is_same_v<T, PlayerServer>)
		{
			life = &e.newLife;
			armour = e.getArmour();
		}
		else
		{
			life = &e.entity.life;
			armour = e.entity.getArmour();
		}
	#pragma endregion

		WeaponStats stats = weapon.getWeaponStats();

		int damage = calculateDamage(armour, stats, rng,
			hitCorectness, critChanceBonus);

		//std::cout << "Damage: " << damage << "\n";

		if (damage >= life->life)
		{
			wasKilled = currentId;
			life->life = 0;
		}
		else
		{
			life->life -= damage;
		}

		glm::vec3 hitDir = dir;
		hitDir += glm::vec3(0, 0.22, 0);
		{
			float l = glm::length(hitDir);
			if (l == 0) { hitDir = {0,-1,0}; } else { hitDir /= l; }
		}
		
		float knockBack = stats.knockBack;

		//todo add knock back resistance
		//std::cout << "Attacked!\n";
		//std::cout << life->life << "\n";
		//std::cout << &life->life << "\n";

		knockBack = std::max(knockBack, 0.f);
		e.applyHitForce(hitDir * knockBack);

	}
	else
	{
		return;
	}

}

template<class T>
bool genericHitEntityByPlayer(T &container, std::uint64_t eid, glm::vec3 dir,
	glm::dvec3 playerPosition, Item &weapon, std::uint64_t &wasKilled, std::minstd_rand &rng
	, float hitCorectness, float critChanceBonus)
{

	auto found = container.find(eid);

	if (found != container.end())
	{

		if constexpr (std::is_pointer_v<decltype(found->second)>)
		{
			doHittingThings(*found->second, dir, playerPosition, weapon, wasKilled, rng, 
				eid, hitCorectness, critChanceBonus);
		}
		else
		{
			doHittingThings(found->second, dir, playerPosition, weapon, wasKilled, rng,
				eid, hitCorectness, critChanceBonus);
		}

		return 1;
	}

	return 0;
};


#define CASE_HIT(x) case x: \
{ return genericHitEntityByPlayer(*entityData.template entityGetter<x>(), \
eid, dir, playerPosition, weapon, wasKilled, rng, hitCorectness, critChanceBonus); } break;


bool callGenericHitEntityByPlayer(EntityData &entityData, std::uint64_t eid, glm::vec3 dir, 
	glm::dvec3 playerPosition, Item &weapon, std::uint64_t &wasKilled, std::minstd_rand &rng
	, float hitCorectness, float critChanceBonus)
{
	auto entityType = getEntityTypeFromEID(eid);

	switch (entityType)
	{

	REPEAT_FOR_ALL_ENTITIES(CASE_HIT);

	default:;
	}

	return 0;
}
#undef CASE_HIT

//todo the hit packet should contain a timer and if the timer is in the future
//or the player spamned hits in a single tick ignore it
bool ServerChunkStorer::hitEntityByPlayer(std::uint64_t eid,
	glm::dvec3 playerPosition, Item &weapon, std::uint64_t &wasKilled,
	glm::vec3 dir, std::minstd_rand &rng, float hitCorectness, float critChanceBonus)
{

	wasKilled = 0;

	auto entityType = getEntityTypeFromEID(eid);
	//todo if entity type doesn't have can be hit, return


	float length = glm::length(dir);
	if (length)
	{
		dir /= length;
	}
	else
	{
		dir = {0,0,1};
	}

	SavedChunk *chunksToCheck[9] = {};

	glm::ivec2 chunkPos{divideChunk(playerPosition.x), divideChunk(playerPosition.z)};

	chunksToCheck[0] = getChunkOrGetNull(chunkPos.x, chunkPos.y);
	chunksToCheck[1] = getChunkOrGetNull(chunkPos.x + 1, chunkPos.y);
	chunksToCheck[2] = getChunkOrGetNull(chunkPos.x - 1, chunkPos.y);
	chunksToCheck[3] = getChunkOrGetNull(chunkPos.x, chunkPos.y + 1);
	chunksToCheck[4] = getChunkOrGetNull(chunkPos.x, chunkPos.y - 1);
	chunksToCheck[5] = getChunkOrGetNull(chunkPos.x + 1, chunkPos.y + 1);
	chunksToCheck[6] = getChunkOrGetNull(chunkPos.x - 1, chunkPos.y - 1);
	chunksToCheck[7] = getChunkOrGetNull(chunkPos.x + 1, chunkPos.y - 1);
	chunksToCheck[8] = getChunkOrGetNull(chunkPos.x - 1, chunkPos.y + 1);

	bool rez = 0;

	for (int i = 0; i < 9; i++)
	{
		if (chunksToCheck[i])
		{
			rez = callGenericHitEntityByPlayer(chunksToCheck[i]->entityData, eid, dir,
				playerPosition, weapon, wasKilled, rng, hitCorectness, critChanceBonus);
			if (rez) 
			{
				return true; 
			}
		}
	}

	return 0;
}

#define CASE_SEARCH_POSITION(X) case X: return genericSearch(*entityData.entityGetter<X>());
std::optional<glm::dvec3> ServerChunkStorer::getEntityPosition(std::uint64_t entity)
{

	auto found = entityChunkPositions.find(entity);

	if (found != entityChunkPositions.end())
	{

		auto foundChunk = savedChunks.find(found->second);

		if (foundChunk != savedChunks.end() && foundChunk->second != nullptr)
		{

			auto type = getEntityTypeFromEID(entity);
			auto &entityData = foundChunk->second->entityData;
			
			if (type == 0)
			{
				auto foundEntity = entityData.players.find(entity);
				if (foundEntity != entityData.players.end()) { return foundEntity->second->getPosition(); }
			}
			else
			{

				auto genericSearch = [&](auto &container) ->std::optional<glm::dvec3>
				{
					auto foundEntity = container.find(entity);
					if (foundEntity != container.end()) { return foundEntity->second.getPosition(); }
					return std::nullopt;
				};
				
				switch (type)
				{

					REPEAT_FOR_ALL_ENTITIES_NO_PLAYERS(CASE_SEARCH_POSITION)

				default:
				break;
				}
					

			}
		

		}


	}


	return std::nullopt;
}

void SavedChunk::removeBlockWithData(glm::ivec3 pos,
	std::uint16_t blockType)
{

	if (blockType == BlockTypes::structureBase)
	{
		blockData.baseBlocks.erase(fromBlockPosInChunkToHashValue(pos.x, 
			pos.y, pos.z));
	}



}

bool SavedChunk::normalize()
{
	bool rez = 0;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				auto &b = chunk.blocks[x][z][y];

				if (b.normalize()) { rez = 1; }
			}

	return rez;

}
