#include "multyPlayer/chunkSaver.h"
#include <glm/vec2.hpp>
#include <filesystem>
#include <iostream>
#include <multyPlayer/serverChunkStorer.h>
#include <cmath>
#include <safeSave.h>

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



glm::ivec2 determineFilePos(glm::ivec2 chunkPos)
{
	glm::vec2 floatPos = chunkPos;
	floatPos /= CHUNK_PACK;
	const glm::ivec2 filePos = {floorf(floatPos.x), floorf(floatPos.y)};
	return filePos;
}


void saveEntityIntoOppenedFile(std::ofstream &f)
{

}


template<class T>
void saveOneEntityTypeIntoOpenFile(std::ofstream &f, T &entityContainer)
{
	for (auto &e : entityContainer)
	{
		e.second.appendDataToDisk(f, e.first);
	}
}

void saveAllEntitiesIntoOpenFile(std::ofstream &f, EntityData &entityData)
{
	//todo generalize
	saveOneEntityTypeIntoOpenFile(f, entityData.droppedItems);
	saveOneEntityTypeIntoOpenFile(f, entityData.zombies);
	saveOneEntityTypeIntoOpenFile(f, entityData.pigs);
	saveOneEntityTypeIntoOpenFile(f, entityData.cats);


}

bool WorldSaver::loadChunk(ChunkData &c)
{

	glm::ivec2 filePos = determineFilePos({c.x, c.z});

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
		if (positions[i] == glm::ivec2{c.x, c.z})
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
	const glm::ivec2 filePos = determineFilePos(pos);

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

void WorldSaver::saveChunkBlockData(SavedChunk &c)
{
	const glm::ivec2 pos = {c.chunk.x, c.chunk.z};
	const glm::ivec2 filePos = (pos);

	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/c";
	fileName += std::to_string(filePos.x);
	fileName += '_';
	fileName += std::to_string(filePos.y);
	fileName += ".block";

	std::vector<unsigned char> data;
	c.blockData.formatBlockData(data, pos.x, pos.y);
	
	if (data.empty())
	{
		std::filesystem::remove(fileName);
	}
	else
	{
		std::ofstream f;
		f.open(fileName, std::ios::binary | std::ios::trunc);

		if (f.is_open())
		{

			f.write((char *)data.data(), data.size());

			f.close();
		}
	}
	

}

bool fileIsEmpty(std::ifstream &f)
{
	return f.peek() == std::ifstream::traits_type::eof();
}

void WorldSaver::loadEntityData(EntityData &entityData,
	glm::ivec2 chunkPosition)
{
	const glm::ivec2 pos = {chunkPosition};
	const glm::ivec2 filePos = (pos);

	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/c";
	fileName += std::to_string(filePos.x);
	fileName += '_';
	fileName += std::to_string(filePos.y);
	fileName += ".entity";

	std::ifstream f(fileName, std::ios::binary);

	if (f.is_open())
	{

		if(!fileIsEmpty(f))
		while (!f.eof())
		{
			Marker m = 0;
			bool success = 0;
			if (readMarker(f, m))
			{
				if (m != 0)
				{
					std::uint64_t eid = 0;
					if (success = (readEntityId(f, eid) && eid != 0))
					{

						switch (m)
						{

						case Markers::droppedItem:
						{
							DroppedItemServer item;
							if (success = item.loadFromDisk(f))
							{
								entityData.droppedItems.insert({eid,item});
							}
						}
						break;


						default:
						success = false;
						};


					}
				}else
				{
					success = false;
				}

			}
			else
			{
				break;
			}

			if (!success)
			{
				std::cout << "file corupted!\n";
				break;
			}

		}
			
		f.close();

	}
	


}

void WorldSaver::loadBlockData(SavedChunk &c)
{
	const glm::ivec2 pos = {c.chunk.x, c.chunk.z};
	const glm::ivec2 filePos = (pos);

	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/c";
	fileName += std::to_string(filePos.x);
	fileName += '_';
	fileName += std::to_string(filePos.y);
	fileName += ".block";

	std::vector<unsigned char> data;
	sfs::readEntireFile(data, fileName.c_str());
	
	c.blockData.loadBlockData(data, pos.x, pos.y);

}

void WorldSaver::saveEntitiesForChunk(SavedChunk &c)
{
	const glm::ivec2 pos = {c.chunk.x, c.chunk.z};
	const glm::ivec2 filePos = (pos);

	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/c";
	fileName += std::to_string(filePos.x);
	fileName += '_';
	fileName += std::to_string(filePos.y);
	fileName += ".entity";

	std::ofstream f;
	f.open(fileName, std::ios::binary | std::ios::trunc);

	if (f.is_open())
	{
		saveAllEntitiesIntoOpenFile(f, c.entityData);
		f.close();
	}


}

//todo
void WorldSaver::appendEntitiesForChunk(glm::ivec2 chunkPos)
{

	const glm::ivec2 filePos = (chunkPos);

	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/c";
	fileName += std::to_string(filePos.x);
	fileName += '_';
	fileName += std::to_string(filePos.y);
	fileName += ".entity";

	std::ofstream f;
	f.open(fileName, std::ios::binary | std::ios::app);

	if (f.is_open())
	{


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

void WorldSaver::saveEntityId(std::uint64_t eid)
{
	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/eid.bin";
	std::ofstream f;
	f.open(fileName, std::ios::binary | std::ios::trunc);

	appendData(f, &eid, sizeof(eid));

	f.close();
}

//todo try to recover if fails
bool WorldSaver::loadEntityId(std::uint64_t &eid)
{

	std::string fileName;
	fileName.reserve(256);
	fileName = savePath;
	fileName += "/eid.bin";
	std::ifstream f;
	f.open(fileName, std::ios::binary);
	bool success = 0;

	if (f.is_open())
	{
		success = readData(f, &eid, sizeof(eid));
		f.close();
	}

	return success;
}
