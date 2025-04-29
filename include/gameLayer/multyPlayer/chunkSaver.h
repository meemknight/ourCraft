#pragma once
#include <chunk.h>
#include <fstream>
#include <string>

struct SavedChunk;
struct EntityData;

struct WorldSaver
{
	std::string savePath = "";

	//returns false if chunk hasn't been saved
	bool loadChunk(ChunkData &c);
	void saveChunk(ChunkData &c);
	void saveChunkBlockData(SavedChunk &c);

	void loadEntityData(EntityData &entityData, glm::ivec2 chunkPosition);

	void loadBlockData(SavedChunk &c);


	void saveEntitiesForChunk(SavedChunk &c);
	void appendEntitiesForChunk(glm::ivec2 chunkPos);


	void saveChunkDataInFile(std::fstream &f, ChunkData &c, int index);
	void appendChunkDataInFile(std::fstream &f, ChunkData &c);
	void loadChunkAtIndex(std::fstream &f, ChunkData &c, int index);
	void loadChunkAtIndex(std::istream &f, ChunkData &c, int index);

	void saveEntityId(std::uint64_t eid);

	bool loadEntityId(std::uint64_t &eid);

	glm::ivec3 spawnPosition = glm::ivec3(0, 107, 0); //todo probably move from here

};