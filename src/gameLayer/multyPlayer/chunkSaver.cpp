#include "multyPlayer/chunkSaver.h"
#include <glm/vec2.hpp>
#include <filesystem>
#include <iostream>

constexpr unsigned int CHUNK_PACK = 4;

constexpr int chunkDataSize = CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE * sizeof(Block);
constexpr int headDist = 1 + (CHUNK_PACK * CHUNK_PACK * sizeof(glm::ivec2));

//////////////////////Chunks//format/////////////////////////////////////////
//
//	(     char     ) (			glm::ivec2 8bytes		 ) ( unsigned char ) ( unsigned char )
//	/* 1 element  */ /* CHUNK_PACK*CHUNK_PACK elements	*/ /*  1 element  */ /*chunkDataSize*/
//	/*chunks count*/ /*			Chunk positions			*/ /* fully loaded*/ /* chunk data  */
//
/////////////////////////////////////////////////////////////////////////////
bool WorldSaver::loadChunk(ChunkData &c)
{
	//todo move into a function
	const glm::ivec2 pos = {c.x, c.z};

	glm::vec2 floatPos = pos;
	floatPos /= CHUNK_PACK;

	const glm::ivec2 filePos = {floorf(floatPos.x), floorf(floatPos.y)};

	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/c";
	fileName += std::to_string(filePos.x);
	fileName += '_';
	fileName += std::to_string(filePos.y);
	fileName += ".chunks";

	if (!std::filesystem::exists(fileName))
	{
		return 0;
	}

	//if it does exist, we first read it's content
	std::fstream f(fileName, std::ios::in | std::ios::out | std::ios::binary);

	f.seekg(0);

	char count = 0;
	f.read(&count, 1);

	if (count > CHUNK_PACK * CHUNK_PACK)
	{
		//todo error report.
		std::cout << "Corrupted file size bigger";
		return 0;
	}

	glm::ivec2 positions[CHUNK_PACK * CHUNK_PACK];

	f.read((char *)positions, count * sizeof(glm::ivec2));

	//then we see if that chunk was already loaded
	int loadIndex = -1;
	for (int i = 0; i < count; i++)
	{
		if (positions[i] == pos)
		{
			loadIndex = i;
			break;
		}
	}

	if (loadIndex == -1)
	{
		//chunk file exists but there is no such chunk generate new chunk...
		f.close();
		return 0;
	}

	loadChunkAtIndex(f, c, loadIndex);
	c.clearLightLevels();


	f.close();
	return 1;
}

void WorldSaver::saveChunk(ChunkData &c)
{

	const glm::ivec2 pos = {c.x, c.z};

	glm::vec2 floatPos = pos;
	floatPos /= CHUNK_PACK;

	const glm::ivec2 filePos = {floorf(floatPos.x), floorf(floatPos.y)};

	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/c";
	fileName += std::to_string(filePos.x);
	fileName += '_';
	fileName += std::to_string(filePos.y);
	fileName += ".chunks";

	if (!std::filesystem::exists(fileName))
	{
		//if the chunk doesn't exist, we create it
		std::fstream f;
		f.open(fileName, std::ios::out | std::ios::binary);

		unsigned char fillData = 0xFF;

		char count = 1;

		f.write(&count, 1);

		for (int i = 0; i < CHUNK_PACK * CHUNK_PACK * sizeof(glm::ivec2); i++)
		{
			f.write((char *)&fillData, sizeof(fillData));
		}

		f.seekp(1);
		//f.seekg(1);

		f.write((char *)&pos, sizeof(pos));

		appendChunkDataInFile(f, c);

		f.close();
	}
	else
	{
		//if it does exist, we first read it's content
		std::fstream f(fileName, std::ios::in | std::ios::out | std::ios::binary);

		f.seekg(0);

		char count = 0;
		f.read(&count, 1);

		if (count > CHUNK_PACK * CHUNK_PACK)
		{
			//todo error report.
			//todo probably try to recreate this chunk?
			std::cout << "corrupted file bigger while saving\n";
		}

		glm::ivec2 positions[CHUNK_PACK * CHUNK_PACK];

		f.read((char *)positions, count * sizeof(glm::ivec2));

		//then we see if that chunk was already loaded
		int loadIndex = -1;
		for (int i = 0; i < count; i++)
		{
			if (positions[i] == pos)
			{
				loadIndex = i;
				break;
			}
		}

		if (loadIndex != -1)
		{
			saveChunkDataInFile(f, c, loadIndex);
		}
		else
		{
			//we add the new chunk
			f.seekp(0, std::ios_base::beg);
			count++;
			f.write(&count, 1);

			f.seekp(1 + (count - 1) * sizeof(glm::ivec2), std::ios_base::beg);
			f.write((char *)&pos, sizeof(glm::ivec2));

			appendChunkDataInFile(f, c);
		}

		//const char* test = "12345678";
		//f.write(test, 8);
		//f.write((char*)&pos.x, sizeof(pos.x));
		f.close();
	}

}

void WorldSaver::saveChunkDataInFile(std::fstream &f, ChunkData &c, int index)
{
	f.seekp(headDist + (chunkDataSize * index), std::ios_base::beg);
	f.write((char *)c.blocks, chunkDataSize);
}

void WorldSaver::appendChunkDataInFile(std::fstream &f, ChunkData &c)
{
	f.seekp(0, std::ios_base::end);
	f.write((char *)c.blocks, chunkDataSize);
}

void WorldSaver::loadChunkAtIndex(std::fstream &f, ChunkData &c, int index)
{
	f.seekg(headDist + (chunkDataSize * index), std::ios_base::beg);
	f.read((char *)c.blocks, chunkDataSize);
}
