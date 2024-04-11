#pragma once
#include <chunk.h>
#include <fstream>
#include <string>

struct WorldSaver
{
	std::string savePath = "";

	//returns false if chunk hasn't been saved
	bool loadChunk(ChunkData &c);
	void saveChunk(ChunkData &c);

	void saveChunkDataInFile(std::fstream &f, ChunkData &c, int index);
	void appendChunkDataInFile(std::fstream &f, ChunkData &c);
	void loadChunkAtIndex(std::fstream &f, ChunkData &c, int index);


};