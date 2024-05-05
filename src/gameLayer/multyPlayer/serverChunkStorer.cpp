#include "multyPlayer/serverChunkStorer.h"
#include <iostream>
#include <chunkSystem.h>

#include <platformTools.h>


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

SavedChunk *ServerChunkStorer::getOrCreateChunk(int posX, int posZ,
	WorldGenerator &wg,
	StructuresManager &structureManager,
	BiomesManager &biomesManager, 
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	bool generateGhostAndStructures,
	std::vector<StructureToGenerate> *newStructuresToAdd, 
	WorldSaver &worldSaver, bool *wasGenerated)
{
	if (wasGenerated) { *wasGenerated = false; }

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

		if (!worldSaver.loadChunk(rez->chunk))
		{
			//create new chunk!
			if (wasGenerated) { *wasGenerated = true; }

			generateChunk(rez->chunk, wg, structureManager, biomesManager, newStructures);
			rez->otherData.dirty = true;
		}
		else
		{
			//std::cout << "Loaded!\n";
		}

		worldSaver.loadEntityData(rez->entityData, {posX, posZ});

		savedChunks[pos] = rez;


		//todo this part should be moved in world generator if possible...

		//generate big structures
		std::vector<glm::ivec2> newCreatedChunks;
		newCreatedChunks.push_back({posX, posZ});

		//generateGhostAndStructures = false;

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

						if (b.type == BlockTypes::grassBlock)
						{
							y++;
							break;
						}
						else if (b.type == BlockTypes::air || b.type == BlockTypes::grass)
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
						s.randomNumber1 = randValues[randomIndex++]; //todo
						s.randomNumber2 = randValues[randomIndex++]; //todo
						s.randomNumber3 = randValues[randomIndex++]; //todo
						s.randomNumber4 = randValues[randomIndex++]; //todo
						s.replaceBlocks = true;
						s.type = Structure_TreeHouse;


						if (generateStructure(s,
							structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, controllBlocks))
						{
							std::cout << "Generated a jungle tree! : " << s.pos.x << " " << s.pos.z << "\n";
							return true;
						}

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

						if (isControlBlock(b->type))
						{
							b->type = BlockTypes::jungle_leaves;

							auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, 0));

							bUp->type = BlockTypes::jungle_leaves;

							auto b2 = tryGetBlockIfChunkExistsNoChecks({pos.x,pos.y,pos.z - 1});
							auto b3 = tryGetBlockIfChunkExistsNoChecks({pos.x,pos.y,pos.z + 1});
							auto b4 = tryGetBlockIfChunkExistsNoChecks({pos.x - 1,pos.y,pos.z});
							auto b5 = tryGetBlockIfChunkExistsNoChecks({pos.x + 1,pos.y,pos.z});

							if (b2 && isControlBlock(b2->type))
							{
								b2->type = BlockTypes::jungle_leaves;
								auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, -1));
								bUp->type = BlockTypes::jungle_leaves;

							}
							else
								if (b3 && isControlBlock(b3->type))
								{
									b3->type = BlockTypes::jungle_leaves;
									auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(0, 1, +1));
									bUp->type = BlockTypes::jungle_leaves;
								}
								else
									if (b4 && isControlBlock(b4->type))
									{
										b4->type = BlockTypes::jungle_leaves;
										auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(-1, 1, 0));
										bUp->type = BlockTypes::jungle_leaves;
									}
									else
										if (b5 && isControlBlock(b5->type))
										{
											b5->type = BlockTypes::jungle_leaves;
											auto bUp = tryGetBlockIfChunkExistsNoChecks(pos + glm::ivec3(+1, 1, 0));
											bUp->type = BlockTypes::jungle_leaves;
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
								b->type = BlockTypes::jungle_planks;
							}
							pos += delta;
						}

						auto b = tryGetBlockIfChunkExistsNoChecks(from);
						if (b)
						{
							b->type = BlockTypes::jungle_planks;
						}
					};

					auto drawDoubleBridge = [&](glm::ivec3 to, glm::ivec3 from)
					{

						glm::ivec3 neighbour1 = to;
						glm::ivec3 neighbour2 = from;

						auto tryNeighbour = [this](glm::ivec3 pos)
						{
							auto b = tryGetBlockIfChunkExistsNoChecks(pos);
							if (b && isControlBlock(b->type))
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

							if (b.type == BlockTypes::sand || b.type == BlockTypes::sand_stone)
							{
								y -= 6 * randValues[randomIndex++];
								break;
							}
							else if (b.type == BlockTypes::air || b.type == BlockTypes::grass)
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

							if (b.type == BlockTypes::snow_block)
							{
								break;
							}
							else if (b.type == BlockTypes::air || b.type == BlockTypes::grass)
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
					c->otherData.dirty = true;
					placeGhostBlocksForChunk(d.x, d.z, d);
					newCreatedChunksSet.insert({d.x, d.z});
				}

			}

			//trees
			for (auto &s : newStructures)
			{
				generateStructure(s, structureManager, newCreatedChunksSet, sendNewBlocksToPlayers, 0);
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

bool ServerChunkStorer::generateStructure(StructureToGenerate s,
	StructureData *structure, int rotation, 
	std::unordered_set<glm::ivec2, Ivec2Hash> &newCreatedChunks, 
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers, 
	std::vector<glm::ivec3> *controlBlocks, bool replace,
	BlockType from, BlockType to)
{
	auto size = structure->size;

	auto replaceB = [&](BlockType &b)
	{
		if (replace)
		{
			if (b == from)
			{
				b = to;
			}
		}
	};

	if (s.pos.y + size.y <= CHUNK_HEIGHT)
	{



		glm::ivec3 startPos = s.pos;
		startPos.x -= size.x / 2;
		startPos.z -= size.z / 2;
		glm::ivec3 endPos = startPos + size;

		for (int x = startPos.x; x < endPos.x; x++)
			for (int z = startPos.z; z < endPos.z; z++)
			{

				int chunkX = divideChunk(x);
				int chunkZ = divideChunk(z);

				//todo this will be replaced with also a check if the
				//chunk was created in the past
				auto c = getChunkOrGetNull(chunkX, chunkZ);

				int inChunkX = modBlockToChunk(x);
				int inChunkZ = modBlockToChunk(z);

				constexpr bool replaceAnything = 0;

				bool sendDataToPlayers = 0;
				//auto it = newCreatedChunks.find({chunkX, chunkZ});
				//if (it != newCreatedChunks.end())
				//{
				//	//todo server should know what chunks can the player see to corectly send him the data
				//	//even if the chunk is not loaded in the server side
				//	//std::cout << "Updating chunk info to player\n";
				//	sendDataToPlayers = true;
				//
				//}
				sendDataToPlayers = true;


				if (c)
				{
					c->otherData.dirty = true;
					for (int y = startPos.y; y < endPos.y; y++)
					{

						auto &b = c->chunk.unsafeGet(inChunkX, y, inChunkZ);

						if (b.type == BlockTypes::air || replaceAnything)
						{
							auto newB = structure->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z,
								rotation);

							replaceB(newB);

							if (newB != BlockTypes::air)
							{
								b.type = newB;

								if (sendDataToPlayers)
								{
									SendBlocksBack sendB;
									sendB.pos = {x,y,z};
									sendB.block = newB;
									sendNewBlocksToPlayers.push_back(sendB);
								}

								if (controlBlocks)
								{
									if (isControlBlock(newB))
									{
										controlBlocks->push_back({x,y,z});
									}
								}
							}

						}
					}
				}
				else
				{

					auto it = ghostBlocks.find({chunkX, chunkZ});


					if (it == ghostBlocks.end())
					{

						std::unordered_map<BlockInChunkPos, GhostBlock, BlockInChunkHash> rez;

						for (int y = startPos.y; y < endPos.y; y++)
						{
							auto b = structure->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z,
								rotation);

							replaceB(b);

							GhostBlock ghostBlock;
							ghostBlock.type = b;
							ghostBlock.replaceAnything = replaceAnything;

							rez[{inChunkX, y, inChunkZ}] = ghostBlock; //todo either ghost either send

							if (sendDataToPlayers)
							{
								SendBlocksBack sendB;
								sendB.pos = {x,y,z};
								sendB.block = b;
								sendNewBlocksToPlayers.push_back(sendB);
							}

							if (controlBlocks)
							{
								if (isControlBlock(b))
								{
									controlBlocks->push_back({x,y,z});
								}
							}
						}

						ghostBlocks[{chunkX, chunkZ}] = rez;

					}
					else
					{
						for (int y = startPos.y; y < endPos.y; y++)
						{
							auto b = structure->unsafeGetRotated(x - startPos.x, y - startPos.y, z - startPos.z,
								rotation);

							replaceB(b);

							GhostBlock ghostBlock;
							ghostBlock.type = b;
							ghostBlock.replaceAnything = replaceAnything;

							auto blockIt = it->second.find({inChunkX, y, inChunkZ});

							if (blockIt == it->second.end())
							{
								it->second[{inChunkX, y, inChunkZ}] = ghostBlock;

								if (sendDataToPlayers)
								{
									SendBlocksBack sendB;
									sendB.pos = {x,y,z};
									sendB.block = b;
									sendNewBlocksToPlayers.push_back(sendB);
								}

								if (controlBlocks)
								{
									if (isControlBlock(b))
									{
										controlBlocks->push_back({x,y,z});
									}
								}
							}
							else
							{
								if (replaceAnything)
								{
									blockIt->second = ghostBlock;

									if (sendDataToPlayers)
									{
										SendBlocksBack sendB;
										sendB.pos = {x,y,z};
										sendB.block = b;
										sendNewBlocksToPlayers.push_back(sendB);
									}

									if (controlBlocks)
									{
										if (isControlBlock(b))
										{
											controlBlocks->push_back({x,y,z});
										}
									}
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
	StructuresManager& structureManager,
	std::unordered_set<glm::ivec2, Ivec2Hash>& newCreatedChunks,
	std::vector<SendBlocksBack>& sendNewBlocksToPlayers,
	std::vector<glm::ivec3>* controlBlocks)
{

	auto chooseRandomElement = [](float randVal, int elementCount)
		{
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<int> dist(0, elementCount - 1);
			return dist(gen);
		};

	// Map structure types to indices in the allStructures vector
	const std::unordered_map<int, size_t> typeToIndex = {
		{Structure_Tree, 0},
		{Structure_JungleTree, 1},
		{Structure_PalmTree, 2},
		{Structure_TreeHouse, 3},
		{Structure_Pyramid, 4},
		{Structure_BirchTree, 5},
		{Structure_Igloo, 6},
		{Structure_Spruce, 7}
	};

	auto it = typeToIndex.find(s.type);
	if (it != typeToIndex.end()) {
		size_t index = it->second;
		const auto& structures = structureManager.allStructures[index];
		if (!structures.empty()) {
			auto tree = structures[chooseRandomElement(s.randomNumber1, structures.size())];
			if (s.type == Structure_Spruce && s.randomNumber3 > 0.5) {
				return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
					newCreatedChunks, sendNewBlocksToPlayers, controlBlocks, true,
					BlockTypes::spruce_leaves, BlockTypes::spruce_leaves_red);
			}
			else {
				return generateStructure(s, tree, chooseRandomElement(s.randomNumber2, 4),
					newCreatedChunks, sendNewBlocksToPlayers, controlBlocks);
			}
		}
	}

	return false;
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


Block *ServerChunkStorer::tryGetBlockIfChunkExistsNoChecks(glm::ivec3 pos)
{
	auto c = getChunkOrGetNull(divideChunk(pos.x), divideChunk(pos.z));

	if (c)
	{
		return &c->chunk.unsafeGet(modBlockToChunk(pos.x), pos.y, modBlockToChunk(pos.z));
	}

	return nullptr;
}

void ServerChunkStorer::placeGhostBlocksForChunk(int posX, int posZ, ChunkData &c)
{
	auto iter = ghostBlocks.find({posX, posZ});

	if (iter != ghostBlocks.end())
	{

		for (auto &b : iter->second)
		{
			//the pos is in chunk space
			auto pos = b.first;

			auto &block = c.unsafeGet(pos.x, pos.y, pos.z);

			if (b.second.replaceAnything || block.type == BlockTypes::air)
			{
				block.type = b.second.type;
			}
		}

		ghostBlocks.erase(iter);
	}
}



bool operator==(const BlockInChunkPos &a, const BlockInChunkPos &b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
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
	bool succeeded = 0;
	for (auto &c : savedChunks)
	{
		if (c.second->otherData.dirty && count > 0)
		{

			saveChunk(worldSaver, c.second);
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
		else if(count <= 0 && entitySaver <= 0) 
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



void ServerChunkStorer::saveAllChunks(WorldSaver &worldSaver)
{
	for (auto &c : savedChunks)
	{
		if (c.second->otherData.dirty)
		{
			saveChunk(worldSaver, c.second);
		}
		else
		{
			worldSaver.saveEntitiesForChunk(*c.second);
			c.second->otherData.dirtyEntity = false;
		}
	}
}

int ServerChunkStorer::unloadChunksThatNeedUnloading(WorldSaver &worldSaver, int count)
{
	int unloaded = 0;
	for (auto it = savedChunks.begin(); it != savedChunks.end(); )
	{
		auto &c = *it;
		if (c.second->otherData.shouldUnload)
		{
			count++;

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


