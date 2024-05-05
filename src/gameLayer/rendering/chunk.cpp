#include <chunk.h>
#include <rendering/renderer.h>
#include <blocksLoader.h>
#include <lightSystem.h>
#include <iostream>
#include <rendering/bigGpuBuffer.h>
#include <platformTools.h>

#undef max
#undef min

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

void arangeData(std::vector<int> &currentVector)
{
	glm::ivec4 *geometryArray = reinterpret_cast<glm::ivec4 *>(opaqueGeometry.data());
	permaAssertComment(opaqueGeometry.size() % 4 == 0, "baking vector corrupted...");
	size_t numElements = opaqueGeometry.size() / 4;

	// Custom comparator function for sorting
	auto comparator = [](const glm::ivec4 &a, const glm::ivec4 &b)
	{
		int firstPart = ((short *)&a.x)[0];
		int secondPart = ((short *)&b.x)[0];

		//return firstPart < secondPart;

		if (firstPart != secondPart)
			return firstPart < secondPart;
		else
		{
			firstPart = ((short *)&a.x)[1];
			secondPart = ((short *)&b.x)[1];
			return firstPart < secondPart;
		};

	};

	// Sort the array of glm::ivec4
	std::sort(geometryArray, geometryArray + numElements, comparator);

}

//todo a counter to know if I have transparent geometry in this chunk
bool Chunk::bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back, 
	glm::ivec3 playerPosition, BigGpuBuffer &gpuBuffer)
{

	bool updateGeometry = 0;
	bool updateTransparency = isDirtyTransparency();

	if (
		isDirty()
		|| (!isNeighbourToLeft() && left != nullptr)
		|| (!isNeighbourToRight() && right != nullptr)
		|| (!isNeighbourToFront() && front != nullptr)
		|| (!isNeighbourToBack() && back != nullptr)
		)
	{
		updateGeometry = true;
		updateTransparency = true;
	}

#pragma region helpers

	auto pushFlagsLightAndPosition = 
		[](std::vector<int> &vect,
		glm::ivec3 position,
		bool isWater, bool isInWater,
		unsigned char sunLight, unsigned char torchLight)
	{

		//0x    FF      FF      FF    FF
		//   -flags----light----position--

		unsigned char light = merge4bits(sunLight, torchLight);

		unsigned char flags = 0;
		if (isWater)
		{
			flags |= 0b1;
		}

		if (isInWater)
		{
			flags |= 0b10;
		}

		unsigned short firstHalf = mergeChars(flags, light);
		//unsigned short firstHalf = mergeChars(flags, 0xFF);

		int positionY = mergeShorts((short)position.y, firstHalf);

		vect.push_back(position.x);
		vect.push_back(positionY);
		vect.push_back(position.z);
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

	auto blockBakeLogicForSolidBlocks = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b, bool isAnimated)
	{
		Block *sides[6] = {};
		getNeighboursLogic(x, y, z, sides);

		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y, z + this->data.z * CHUNK_SIZE};

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


				bool isInWater = (sides[i] != nullptr) && sides[i]->type == BlockTypes::water;

				if (dontUpdateLightSystem)
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater, 
						15, 15);

				}
				else if (isLightEmitter(b.type))
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						b.getSkyLight(), 15);
					//todo transparent block should emit internal light
				}else
				if (i == 2 && y == CHUNK_HEIGHT - 1)
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						15, b.getLight());
				}
				else if (y == 0 && i == 3)
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						15, b.getLight()); //bottom of the world
					
				}
				else if (sides[i] != nullptr)
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						sides[i]->getSkyLight(), sides[i]->getLight());
				}
				else
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						0,0);
				}
				
			}
		}
	};

	auto blockBakeLogicForTransparentBlocks = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b, bool isAnimated)
	{
		Block *sides[6] = {};
		getNeighboursLogic(x, y, z, sides);

		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y,
				z + this->data.z * CHUNK_SIZE};

		for (int i = 0; i < 6; i++)
		{

			bool isWater = b.type == BlockTypes::water;

			if ((sides[i] != nullptr
				&& (!(sides[i])->isOpaque() && sides[i]->type != b.type)
				)||
				(
					isWater && i == 2 //display water if there is a block on top
				)
				|| (
				//(i == 3 && y == 0) ||		//display the bottom face
				(i == 2 && y == CHUNK_HEIGHT - 1)
				)
				)
			{

				//no faces in between water
				if (isWater && sides[i] && sides[i]->type == BlockTypes::water) { continue; }

				if (isWater)
				{
					//if (!sides[2] || sides[2]->type != BlockTypes::water)
					//{
					//	currentVector->push_back(mergeShorts(i + 22, getGpuIdIndexForBlock(b.type, i)));
					//}
					currentVector->push_back(mergeShorts(i + 22, getGpuIdIndexForBlock(b.type, i)));

				}
				else
				{
					currentVector->push_back(mergeShorts(i, getGpuIdIndexForBlock(b.type, i)));
				}

			
				bool isInWater = (sides[i] != nullptr) && sides[i]->type == BlockTypes::water;

				if (dontUpdateLightSystem)
				{
					pushFlagsLightAndPosition(*currentVector, position, isWater, isInWater,
						15, 15);
				}
				else
					if (sides[i] == nullptr && i == 2)
					{
						pushFlagsLightAndPosition(*currentVector, position, isWater, isInWater,
							15, b.getLight());
					}
					else if (sides[i] == nullptr && i == 3)
					{
						pushFlagsLightAndPosition(*currentVector, position, isWater, isInWater,
							5, b.getLight());
						//bottom of the world
					}
					else if (sides[i] != nullptr)
					{
						int val = merge4bits(
							std::max(b.getSkyLight(),sides[i]->getSkyLight()), 
							std::max(b.getLight(),sides[i]->getLight())
						);

						pushFlagsLightAndPosition(*currentVector, position, isWater, isInWater,
							std::max(b.getSkyLight(), sides[i]->getSkyLight()),
							std::max(b.getLight(), sides[i]->getLight()));
					}
					else
					{
						pushFlagsLightAndPosition(*currentVector, position, 
							isWater, isInWater,
							0, 0);
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

		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y,
			z + this->data.z * CHUNK_SIZE};

		for (int i = 6; i <= 9; i++)
		{
			//opaqueGeometry.push_back(mergeShorts(i, b.type));
			currentVector->push_back(mergeShorts(i, getGpuIdIndexForBlock(b.type, 0)));

		

			if (dontUpdateLightSystem)
			{
				pushFlagsLightAndPosition(*currentVector, position, 0, 0, 15, 15);
			}
			else
			{
				pushFlagsLightAndPosition(*currentVector, position,
					0, 0, b.getSkyLight(), b.getLight());
			}

		}

	};

	auto blockBakeLogicForTorches = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b)
	{
		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y,
				z + this->data.z * CHUNK_SIZE};

		for (int i = 0; i < 6; i++)
		{
		
			if (i == 3)
			{
				auto bbottom = safeGet(x, y - 1, z);
				if (bbottom && bbottom->isOpaque()) { continue; }
			}

			currentVector->push_back(mergeShorts(i + 16, getGpuIdIndexForBlock(b.type, i)));

			if (dontUpdateLightSystem)
			{
				pushFlagsLightAndPosition(*currentVector, position, 0, 0, 15, 15);
			}
			else
			{
				pushFlagsLightAndPosition(*currentVector, position,
					0, 0, b.getSkyLight(), b.getLight());
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
		setDirty(0);
		setNeighbourToLeft(left != nullptr);
		setNeighbourToRight(right != nullptr);
		setNeighbourToFront(front != nullptr);
		setNeighbourToBack(back != nullptr);

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
						else if (b.type == BlockTypes::torch)
						{
							blockBakeLogicForTorches(x, y, z, &opaqueGeometry, b);
						}else
						{
							if (!b.isTransparentGeometry())
							{
								blockBakeLogicForSolidBlocks(x, y, z, &opaqueGeometry, b, b.isAnimatedBlock());
							}
						}

						if (b.isLightEmitter())
						{
							lights.push_back({x + data.x * CHUNK_SIZE, y, z + data.z * CHUNK_SIZE,0});
						}
					}
				}

		//trying to place the data in a better way for the gpu
		arangeData(opaqueGeometry);
	}

	if (updateTransparency)
	{
		setDirtyTransparency(0);

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
			opaqueGeometry.data(), GL_STATIC_DRAW);
		elementCountSize = opaqueGeometry.size() / 4; //todo magic number

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(lights[0]),
			lights.data(), GL_STREAM_READ);
		lightsElementCountSize = lights.size();

		gpuBuffer.addChunk({data.x, data.z}, opaqueGeometry);
	}

	if (updateTransparency)
	{
		glBindBuffer(GL_ARRAY_BUFFER, transparentGeometryBuffer);
		glBufferData(GL_ARRAY_BUFFER, transparentGeometry.size() * sizeof(transparentGeometry[0]),
			transparentGeometry.data(), GL_STATIC_DRAW);
		transparentElementCountSize = transparentGeometry.size() / 4; //todo magic number
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
		isDirty()
		|| (!isNeighbourToLeft() && left != nullptr)
		|| (!isNeighbourToRight() && right != nullptr)
		|| (!isNeighbourToFront() && front != nullptr)
		|| (!isNeighbourToBack() && back != nullptr)
		)
	{
		return false;
	}

	return isDirtyTransparency();
}



void Chunk::createGpuData()
{
	//unsigned char winding[4] = {0,1,2,4};
	unsigned char winding[4] = {0,1,2,3};

	glGenBuffers(1, &opaqueGeometryBuffer);
	glGenBuffers(1, &opaqueGeometryIndex);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, opaqueGeometryBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opaqueGeometryIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(winding), winding, GL_STATIC_DRAW);
	setupVertexAttributes();

	glGenBuffers(1, &transparentGeometryBuffer);
	glGenBuffers(1, &transparentGeometryIndex);
	glGenVertexArrays(1, &transparentVao);
	glBindVertexArray(transparentVao);
	glBindBuffer(GL_ARRAY_BUFFER, transparentGeometryBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, transparentGeometryIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(winding), winding, GL_STATIC_DRAW);
	setupVertexAttributes();

	glBindVertexArray(0);

	glGenBuffers(1, &lightsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


}

void Chunk::clearGpuData(BigGpuBuffer *gpuBuffer)
{
	glDeleteBuffers(1, &opaqueGeometryBuffer);
	glDeleteBuffers(1, &opaqueGeometryIndex);
	glDeleteBuffers(1, &transparentGeometryBuffer);
	glDeleteBuffers(1, &transparentGeometryIndex);
	glDeleteBuffers(1, &lightsBuffer);
	glDeleteVertexArrays(1, &vao);
	glDeleteVertexArrays(1, &transparentVao);

	if (gpuBuffer)
	{
		gpuBuffer->removeChunk({data.x, data.z});
	}
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



int modBlockToChunk(int x)
{
	if (x < 0)
	{
		x = -x;
		x--;
		return CHUNK_SIZE - (x % CHUNK_SIZE) - 1;
	}
	else
	{
		return x % CHUNK_SIZE;
	}
}
