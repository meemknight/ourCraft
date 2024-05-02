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

	void loadEntityData(EntityData &entityData, glm::ivec2 chunkPosition);

	void saveEntitiesForChunk(SavedChunk &c);
	void appendEntitiesForChunk(glm::ivec2 chunkPos);


	void saveChunkDataInFile(std::fstream &f, ChunkData &c, int index);
	void appendChunkDataInFile(std::fstream &f, ChunkData &c);
	void loadChunkAtIndex(std::fstream &f, ChunkData &c, int index);


};