#include <metrics.h>



int divideChunk(int x)
{
	return (int)std::floorf((float)x / (float)CHUNK_SIZE);
}
glm::ivec2 fromBlockPosToChunkPos(glm::ivec3 blockPos)
{
	return glm::ivec2(divideChunk(blockPos.x), divideChunk(blockPos.z));
}
glm::ivec2 fromBlockPosToChunkPos(int x, int z)
{
	return glm::ivec2(divideChunk(x), divideChunk(z));
}
glm::ivec3 fromBlockPosToBlockPosInChunk(glm::ivec3 blockPos)
{
	int modX = modBlockToChunk(blockPos.x);
	int modZ = modBlockToChunk(blockPos.z);
	return {modX, blockPos.y, modZ};
}

int divideMetaChunk(int chunkPos)
{
	return (int)std::floor((float)chunkPos / (float)META_CHUNK_SIZE);
}


//todo move into a new header file
glm::ivec2 determineChunkThatIsEntityIn(glm::dvec3 position)
{
	return {divideChunk(position.x), divideChunk(position.z)};
}

