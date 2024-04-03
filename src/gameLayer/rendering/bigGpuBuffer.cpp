#include <rendering/bigGpuBuffer.h>
#include <platformTools.h>
#include <iostream>

void setupVertexAttributes()
{
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_SHORT, 4 * sizeof(int), 0);
	glVertexAttribDivisor(0, 1);

	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 1, GL_SHORT, 4 * sizeof(int), (void *)(1 * sizeof(short)));
	glVertexAttribDivisor(1, 1);

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 3, GL_INT, 4 * sizeof(int), (void *)(1 * sizeof(int)));
	glVertexAttribDivisor(2, 1);

	//glEnableVertexAttribArray(3);
	//glVertexAttribIPointer(3, 1, GL_INT, 4 * sizeof(int), (void *)(4 * sizeof(int)));
	//glVertexAttribDivisor(3, 1);
	//
	//glEnableVertexAttribArray(4);
	//glVertexAttribIPointer(4, 1, GL_INT, 4 * sizeof(int), (void *)(5 * sizeof(int)));
	//glVertexAttribDivisor(4, 1);
}

void BigGpuBuffer::create(size_t chunks)
{
	unsigned char winding[4] = {0,1,2,4};

	arenaSize = chunks * 10'000 * 6 * sizeof(int);

	//add an empty size stub
	entriesList.push_front({});

	glGenBuffers(1, &opaqueGeometryBuffer);
	glGenBuffers(1, &opaqueGeometryIndex);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, opaqueGeometryBuffer);
	glBufferData(GL_ARRAY_BUFFER, arenaSize, 0, GL_DYNAMIC_DRAW); //todo look into a modern function here
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opaqueGeometryIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(winding), winding, GL_STATIC_DRAW);
	setupVertexAttributes();

	

	glBindVertexArray(0);




}

void BigGpuBuffer::cleanup()
{
	glDeleteBuffers(1, &opaqueGeometryBuffer);
	glDeleteBuffers(1, &opaqueGeometryIndex);
	glDeleteVertexArrays(1, &vao);
	*this = {};
}

void BigGpuBuffer::addChunk(glm::ivec2 chunkPos, std::vector<int> &data)
{
	{
		auto it = entriesMap.find(chunkPos);
	
		//chunk already exists, we delete the old one.
		if (it != entriesMap.end())
		{
			entriesList.erase(it->second);
			entriesMap.erase(it);
		}
	}
	
	if (data.empty()) { return; } //special case empty chunk, we just delete
	
	bool found = 0;
	for (auto i = entriesList.begin(); i != entriesList.end(); ++i)
	{
		auto second = i; ++second;
	
		size_t blockSize = 0;
		size_t blockStartPos = i->beg + i->size;
	
		size_t blockEnd = 0;
	
		if (second != entriesList.end())
		{
			blockEnd = second->beg;
		}
		else
		{
			blockEnd = arenaSize;
		}
	
		permaAssertComment(blockEnd >= blockStartPos, "desync in allocator");
	
		blockSize = blockEnd - blockStartPos;
	
		if (blockSize >= data.size() * sizeof(data[0]))
		{
			//we found a candidate;
			found = true;
	
			GpuEntry entry;
			entry.beg = blockStartPos;
			entry.size = data.size() * sizeof(data[0]);
	
			entriesMap[chunkPos] = entriesList.insert(second, entry);
	
			writeData(data, blockStartPos);
	
			break;
		}
	
	}
	
	if (!found)
	{
		//resize arena
		//todo
		std::cout << "Full arena\n";
	
		{
			auto it = entriesMap.find(chunkPos);
	
			if (it != entriesMap.end())
			{
				entriesList.erase(it->second);
				entriesMap.erase(it);
			}
		}
	}


}

void BigGpuBuffer::removeChunk(glm::ivec2 chunkPos)
{
	auto it = entriesMap.find(chunkPos);

	if (it != entriesMap.end())
	{
		entriesList.erase(it->second);
		entriesMap.erase(it);
	}
}

void BigGpuBuffer::writeData(std::vector<int> &data, size_t pos)
{
	//todo modern function
	glBindBuffer(GL_ARRAY_BUFFER, opaqueGeometryBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, pos, 
		data.size() * sizeof(data[0]),
		data.data()); //todo look into a modern function here

}

BigGpuBuffer::GpuEntry BigGpuBuffer::getEntry(glm::ivec2 chunkPos)
{
	auto it = entriesMap.find(chunkPos);

	if (it != entriesMap.end())
	{
		return *it->second;
	}
	else
	{
		return {};
	}
}
