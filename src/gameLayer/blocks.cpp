#include "blocks.h"
#include "rendering/renderer.h"
#include "worldGenerator.h"
#include "blocksLoader.h"
#include "lightSystem.h"
#include <algorithm>

Block* Chunk::safeGet(int x, int y, int z)
{
	if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || z >= CHUNK_SIZE || y >= CHUNK_HEIGHT)
	{
		return nullptr;
	}
	else
	{
		return &unsafeGet(x, y, z);
	}
}


struct TransparentCandidate
{
	glm::ivec3 position;
	float distance;
};

static std::vector<TransparentCandidate> transparentCandidates;
static std::vector<int> opaqueGeometry;
static std::vector<int> transparentGeometry;

//todo a flag to know if I have transparent geometry in this chunk
bool Chunk::bake(Chunk* left, Chunk* right, Chunk* front, Chunk* back, glm::ivec3 playerPosition)
{

	bool updateGeometry = 0;
	bool updateTransparency = dirtyTransparency;

	if (
		dirty
		|| (!neighbourToLeft && left != nullptr)
		|| (!neighbourToRight && right != nullptr)
		|| (!neighbourToFront && front != nullptr)
		|| (!neighbourToBack && back != nullptr)
		)
	{
		updateGeometry = true;
		updateTransparency = true;
	}

#pragma region helpers

	auto blockBackeLogic = [&](int x, int y, int z, std::vector<int> *currentVector, Block *sides[6], Block &b)
	{
		for (int i = 0; i < 6; i++)
		{

			if ((sides[i] != nullptr && !(sides[i])->isOpaque())
				|| (
				//(i == 3 && y == 0) ||		//display the bottom face
				(i == 2 && y == CHUNK_HEIGHT - 1)
				)
				)
			{
				//currentVector->push_back(mergeShorts(i + (int)b.isAnimated() * 10, b.type));

				if (b.isAnimated())
				{
					currentVector->push_back(mergeShorts(i + 10,
						getGpuIdIndexForBlock(b.type, i)));
				}
				else
				{
					currentVector->push_back(mergeShorts(i,
						getGpuIdIndexForBlock(b.type, i)));
				}


				currentVector->push_back(x + this->data.x * CHUNK_SIZE);
				currentVector->push_back(y);
				currentVector->push_back(z + this->data.z * CHUNK_SIZE);

				if (dontUpdateLightSystem)
				{
					currentVector->push_back(15);
				}
				else
					if (sides[i] == nullptr && i == 2)
					{
						currentVector->push_back(15);
					}
					else if (sides[i] == nullptr && i == 3)
					{
						currentVector->push_back(5); //bottom of the world
					}
					else if (sides[i] != nullptr)
					{
						int val = sides[i]->getSkyLight();

						if (val > 0)
						{
							int a = 0;
						}

						currentVector->push_back(val);
					}
					else
					{
						currentVector->push_back(0);
					}

			}
		}
	};

	auto getNeighboursLogic = [&](int x, int y, int z, Block *sides[6])
	{
		auto bfront = safeGet(x, y, z + 1);
		auto bback = safeGet(x, y, z - 1);
		auto btop = safeGet(x, y + 1, z);
		auto bbottom = safeGet(x, y - 1, z);
		auto bleft = safeGet(x - 1, y, z);
		auto bright = safeGet(x + 1, y, z);

		if (bfront == nullptr && front != nullptr)
		{
			bfront = front->safeGet(x, y, 0);
		}

		if (bback == nullptr && back != nullptr)
		{
			bback = back->safeGet(x, y, CHUNK_SIZE - 1);
		}

		if (bleft == nullptr && left != nullptr)
		{
			bleft = left->safeGet(CHUNK_SIZE - 1, y, z);
		}

		if (bright == nullptr && right != nullptr)
		{
			bright = right->safeGet(0, y, z);
		}

		sides[0] = bfront;
		sides[1] = bback;
		sides[2] = btop;
		sides[3] = bbottom;
		sides[4] = bleft;
		sides[5] = bright;
	};

	opaqueGeometry.clear();
	transparentGeometry.clear();
	transparentCandidates.clear();

#pragma endregion

	if (updateGeometry)
	{
		dirty = 0;
		neighbourToLeft = (left != nullptr);
		neighbourToRight = (right != nullptr);
		neighbourToFront = (front != nullptr);
		neighbourToBack = (back != nullptr);


		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				for (int y = 0; y < CHUNK_HEIGHT; y++)
				{
					auto &b = unsafeGet(x, y, z);
					if (!b.air())
					{
						//todo occlude if all of the blocks
						if (b.isGrassMesh())
						{
							Block *sides[6] = {};
							getNeighboursLogic(x, y, z, sides);

							for (int i = 6; i <= 9; i++)
							{
								//opaqueGeometry.push_back(mergeShorts(i, b.type));
								opaqueGeometry.push_back(mergeShorts(i, getGpuIdIndexForBlock(b.type, 0)));

								opaqueGeometry.push_back(x + this->data.x * CHUNK_SIZE);
								opaqueGeometry.push_back(y);
								opaqueGeometry.push_back(z + this->data.z * CHUNK_SIZE);
								//opaqueGeometry.push_back(15);

								if (dontUpdateLightSystem)
								{
									opaqueGeometry.push_back(15);
								}
								else
								{
									opaqueGeometry.push_back(b.getSkyLight());
								}

							}

						}
						else
						{
							Block *sides[6] = {};
							getNeighboursLogic(x, y, z, sides);

							if (!b.isTransparentGeometry())
							{
								blockBackeLogic(x, y, z, &opaqueGeometry, sides, b);
							}
						}
					}
				}

		}

	if (updateTransparency)
	{
		dirtyTransparency = 0;

		int chunkPosX = data.x * CHUNK_SIZE;
		int chunkPosZ = data.z * CHUNK_SIZE;

		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				for (int y = 0; y < CHUNK_HEIGHT; y++)
				{
					auto &b = unsafeGet(x, y, z);

					//transparent geometry doesn't include air
					if (b.isTransparentGeometry())
					{
						glm::vec3 difference = playerPosition - glm::ivec3{x, y, z} - glm::ivec3{chunkPosX, 0, chunkPosZ};
						float distance = glm::dot(difference, difference);
						transparentCandidates.push_back({{x,y,z}, distance});
					}

				}

		std::sort(transparentCandidates.begin(), transparentCandidates.end(), [](TransparentCandidate &a,
			TransparentCandidate &b)
		{
			return a.distance > b.distance;
		});

		for (auto &c : transparentCandidates)
		{
			auto &b = unsafeGet(c.position.x, c.position.y, c.position.z);
			Block *sides[6] = {};
			getNeighboursLogic(c.position.x, c.position.y, c.position.z, sides);
			blockBackeLogic(c.position.x, c.position.y, c.position.z, &transparentGeometry, sides, b);
		}

	};

	if (updateGeometry)
	{
		glBindBuffer(GL_ARRAY_BUFFER, opaqueGeometryBuffer);
		glBufferData(GL_ARRAY_BUFFER, opaqueGeometry.size() * sizeof(opaqueGeometry[0]),
			opaqueGeometry.data(), GL_STREAM_DRAW);
		elementCountSize = opaqueGeometry.size() / 5;
	}
		
	if (updateTransparency)
	{
		glBindBuffer(GL_ARRAY_BUFFER, transparentGeometryBuffer);
		glBufferData(GL_ARRAY_BUFFER, transparentGeometry.size() * sizeof(transparentGeometry[0]),
			transparentGeometry.data(), GL_STREAM_DRAW);
		transparentElementCountSize = transparentGeometry.size() / 5;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if ((updateTransparency && transparentElementCountSize>0)|| updateGeometry)
	{
		return true;
	}
	else
	{
		return false;
	}
	
}

void Chunk::create(int x, int z)
{

	this->data.x = x;
	this->data.z = z;

	data.clearLightLevels();

	generateChunk(1234, *this);

}

void setupVertexAttributes()
{
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_SHORT, 5 * sizeof(int), 0);
	glVertexAttribDivisor(0, 1);

	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 1, GL_SHORT, 5 * sizeof(int), (void *)(1 * sizeof(short)));
	glVertexAttribDivisor(1, 1);

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 3, GL_INT, 5 * sizeof(int), (void *)(1 * sizeof(int)));
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, 5 * sizeof(int), (void *)(4 * sizeof(int)));
	glVertexAttribDivisor(3, 1);
}

void Chunk::createGpuData()
{
	glGenBuffers(1, &opaqueGeometryBuffer);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, opaqueGeometryBuffer);
	setupVertexAttributes();

	glGenBuffers(1, &transparentGeometryBuffer);
	glGenVertexArrays(1, &transparentVao);
	glBindVertexArray(transparentVao);
	glBindBuffer(GL_ARRAY_BUFFER, transparentGeometryBuffer);
	setupVertexAttributes();

	glBindVertexArray(0);
}

void Chunk::clearGpuData()
{
	glDeleteBuffers(1, &opaqueGeometryBuffer);
}

void ChunkData::clearLightLevels()
{

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				unsafeGet(x, y, z).lightLevel = 0;
				unsafeGet(x, y, z).notUsed = 0;
			}

}

bool isBlockMesh(uint16_t type)
{
	return !isBlockMesh(type);
}

bool isCrossMesh(uint16_t type)
{
	return type == grass || type == rose;
}

bool isOpaque(uint16_t type)
{
	return
		type != BlockTypes::air
		&& type != BlockTypes::leaves
		&& !(isTransparentGeometry(type))
		&& !(isGrassMesh(type));
}

bool isTransparentGeometry(uint16_t type)
{
	return type == BlockTypes::ice;
}

bool isGrassMesh(uint16_t type)
{
	return type == BlockTypes::grass
		|| type == BlockTypes::rose
		;
}
