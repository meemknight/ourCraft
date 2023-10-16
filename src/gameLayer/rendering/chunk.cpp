#include <chunk.h>
#include <rendering/renderer.h>
#include <blocksLoader.h>
#include <lightSystem.h>

Block *Chunk::safeGet(int x, int y, int z)
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
static std::vector<glm::ivec4> lights;

//todo a counter to know if I have transparent geometry in this chunk
bool Chunk::bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back, glm::ivec3 playerPosition)
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

	auto blockBakeLogicForSolidBlocks = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b, bool isAnimated)
	{
		Block *sides[6] = {};
		getNeighboursLogic(x, y, z, sides);

		for (int i = 0; i < 6; i++)
		{

			if ((sides[i] != nullptr && !(sides[i])->isOpaque())
				|| (
				//(i == 3 && y == 0) ||		//display the bottom face
				(i == 2 && y == CHUNK_HEIGHT - 1)
				)
				)
			{
				currentVector->push_back(mergeShorts(i + isAnimated * 10, getGpuIdIndexForBlock(b.type, i)));
				currentVector->push_back(x + this->data.x * CHUNK_SIZE);
				currentVector->push_back(y);
				currentVector->push_back(z + this->data.z * CHUNK_SIZE);

				if (dontUpdateLightSystem)
				{
					currentVector->push_back(15);
				}
				else if (isLightEmitor(b.type))
				{
					currentVector->push_back(15);
				}else
				if (i == 2 && y == CHUNK_HEIGHT - 1)
				{
					currentVector->push_back(15);
				}
				else if (y == 0 && i == 3)
				{
					currentVector->push_back(5); //bottom of the world
				}
				else if (sides[i] != nullptr)
				{
					int val = std::max(sides[i]->getSkyLight(), sides[i]->getLight());
					currentVector->push_back(val);
				}
				else
				{
					currentVector->push_back(0);
				}

			}
		}
	};

	auto blockBakeLogicForTransparentBlocks = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b, bool isAnimated)
	{
		Block *sides[6] = {};
		getNeighboursLogic(x, y, z, sides);

		for (int i = 0; i < 6; i++)
		{

			if ((sides[i] != nullptr
				&& (!(sides[i])->isOpaque() && sides[i]->type != b.type)
				)
				|| (
				//(i == 3 && y == 0) ||		//display the bottom face
				(i == 2 && y == CHUNK_HEIGHT - 1)
				)
				)
			{
				currentVector->push_back(mergeShorts(i + isAnimated * 10, getGpuIdIndexForBlock(b.type, i)));
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
						int val = std::max(sides[i]->getSkyLight(), sides[i]->getLight());
						currentVector->push_back(val);
					}
					else
					{
						currentVector->push_back(0);
					}

			}
		}
	};

	auto blockBakeLogicForGrassMesh = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b)
	{
		Block *sides[6] = {};
		getNeighboursLogic(x, y, z, sides);

		bool ocluded = 1;
		for (int i = 0; i < 6; i++)
		{
			if (sides[i] != nullptr && !sides[i]->isOpaque())
			{
				ocluded = 0;
				break;
			}
		}

		if (ocluded)return;


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
				opaqueGeometry.push_back( std::max(b.getSkyLight(), b.getLight()));
			}

		}

	};

	opaqueGeometry.clear();
	transparentGeometry.clear();
	transparentCandidates.clear();
	lights.clear();

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
						if (b.isGrassMesh())
						{
							blockBakeLogicForGrassMesh(x, y, z, &opaqueGeometry, b);
						}
						else
						{
							if (!b.isTransparentGeometry())
							{
								blockBakeLogicForSolidBlocks(x, y, z, &opaqueGeometry, b, b.isAnimatedBlock());
							}
						}

						if (b.isLightEmitor())
						{
							lights.push_back({x + data.x * CHUNK_SIZE, y, z + data.z * CHUNK_SIZE,0});
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
			blockBakeLogicForTransparentBlocks(c.position.x, c.position.y, c.position.z, &transparentGeometry, b,
				b.isAnimatedBlock());
		}

	};

	if (updateGeometry)
	{
		glBindBuffer(GL_ARRAY_BUFFER, opaqueGeometryBuffer);
		glBufferData(GL_ARRAY_BUFFER, opaqueGeometry.size() * sizeof(opaqueGeometry[0]),
			opaqueGeometry.data(), GL_STREAM_DRAW);
		elementCountSize = opaqueGeometry.size() / 5;

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(lights[0]),
			lights.data(), GL_STREAM_DRAW);
		lightsElementCountSize = lights.size();
	}

	if (updateTransparency)
	{
		glBindBuffer(GL_ARRAY_BUFFER, transparentGeometryBuffer);
		glBufferData(GL_ARRAY_BUFFER, transparentGeometry.size() * sizeof(transparentGeometry[0]),
			transparentGeometry.data(), GL_STREAM_DRAW);
		transparentElementCountSize = transparentGeometry.size() / 5;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if ((updateTransparency && transparentElementCountSize > 0) || updateGeometry)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool Chunk::shouldBakeOnlyBecauseOfTransparency(Chunk *left, Chunk *right, Chunk *front, Chunk *back)
{
	if (
		dirty
		|| (!neighbourToLeft && left != nullptr)
		|| (!neighbourToRight && right != nullptr)
		|| (!neighbourToFront && front != nullptr)
		|| (!neighbourToBack && back != nullptr)
		)
	{
		return false;
	}

	return dirtyTransparency;
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

	glGenBuffers(1, &lightsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsBuffer);


}

void Chunk::clearGpuData()
{
	glDeleteBuffers(1, &opaqueGeometryBuffer);
	glDeleteBuffers(1, &transparentGeometryBuffer);
	glDeleteBuffers(1, &lightsBuffer);
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
