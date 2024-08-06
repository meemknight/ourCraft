#include <chunk.h>
#include <rendering/renderer.h>
#include <blocksLoader.h>
#include <lightSystem.h>
#include <iostream>
#include <rendering/bigGpuBuffer.h>
#include <platformTools.h>
#include <algorithm>

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
	Chunk *frontLeft, Chunk *frontRight, Chunk *backLeft, Chunk *backRight,
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
		unsigned char sunLight, unsigned char torchLight, unsigned char aoShape)
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

		//aoShape &= 0x0F;
		aoShape <<= 4;
		flags |= aoShape;

		//shadow flag stuff.


		unsigned short firstHalf = mergeChars(flags, light);
		//unsigned short firstHalf = mergeChars(flags, 0xFF);

		int positionY = mergeShorts((short)position.y, firstHalf);

		vect.push_back(position.x);
		vect.push_back(positionY);
		vect.push_back(position.z);
	};

	const int FRONT = 0;
	const int BACK = 1;
	const int TOP = 2;
	const int BOTTOM = 3;
	const int LEFT = 4;
	const int RIGHT = 5;

	const int DOWN_FRONT = 6;
	const int DOWN_BACK = 7;
	const int DOWN_LEFT = 8;
	const int DOWN_RIGHT = 9;

	const int UP_FRONT = 10;
	const int UP_BACK = 11;
	const int UP_LEFT = 12;
	const int UP_RIGHT = 13;

	const int UP_FRONTLEFT = 14;
	const int UP_FRONTRIGHT = 15;
	const int UP_BACKLEFT = 16;
	const int UP_BACKRIGHT = 17;

	auto getNeighboursLogic = [&](int x, int y, int z, Block *sides[18])
	{
		auto justGetBlock = [&](int x, int y, int z) -> Block *
		{
			if (y >= CHUNK_HEIGHT || y < 0) { return nullptr; }

			if (x >= 0 && x < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE)
			{
				return &unsafeGet(x, y, z);
			}

			if (x >= 0 && x < CHUNK_SIZE)
			{
				//z is the problem
				if (z < 0)
				{
					if (back)
					{
						return &back->unsafeGet(x, y, CHUNK_SIZE - 1);
					}
				}
				else
				{
					if (front)
					{
						return &front->unsafeGet(x, y, 0);
					}
				}
			}
			else if (z >= 0 && z < CHUNK_SIZE)
			{
				//x is the problem
				if (x < 0)
				{
					if (left)
					{
						return &left->unsafeGet(CHUNK_SIZE - 1, y, z);
					}
				}
				else
				{
					if (right)
					{
						return &right->unsafeGet(0, y, z);
					}
				}
			}
			else
			{
				//both are the problem
				if (x < 0 && z < 0)
				{
					if (backLeft)
					{
						return &backLeft->unsafeGet(CHUNK_SIZE - 1, y, CHUNK_SIZE - 1);
					}
				}else
				if (x >= CHUNK_SIZE && z < 0)
				{
					if (backRight)
					{
						return &backRight->unsafeGet(0, y, CHUNK_SIZE - 1);
					}
				}else if (x < 0 && z >= CHUNK_SIZE)
				{
					if (frontLeft)
					{
						return &frontLeft->unsafeGet(CHUNK_SIZE - 1, y, 0);
					}
				}else
				if (x >= CHUNK_SIZE && z >= CHUNK_SIZE)
				{
					if (frontRight)
					{
						return &frontRight->unsafeGet(0, y, 0);
					}
				}
				else
				{
					permaAssertComment(0, "error in chunk get neighbour logic!");
				}

				return nullptr;
			}
		};

		auto bfront = justGetBlock(x, y, z + 1);
		auto bback = justGetBlock(x, y, z - 1);
		auto btop = justGetBlock(x, y + 1, z);
		auto bbottom = justGetBlock(x, y - 1, z);
		auto bleft = justGetBlock(x - 1, y, z);
		auto bright = justGetBlock(x + 1, y, z);

		auto bdownfront = justGetBlock(x, y-1, z + 1);
		auto bdownback = justGetBlock(x, y-1, z - 1);
		auto bdownleft = justGetBlock(x - 1, y-1, z);
		auto bdownright = justGetBlock(x + 1, y-1, z);

		auto bupfront = justGetBlock(x, y + 1, z + 1);
		auto bupback = justGetBlock(x, y + 1, z - 1);
		auto bupleft = justGetBlock(x - 1, y + 1, z);
		auto bupright = justGetBlock(x + 1, y + 1, z);

		auto bupfrontLeft = justGetBlock(x - 1, y + 1, z + 1);
		auto bupfrontright = justGetBlock(x + 1, y + 1, z + 1);
		auto bupbackleft = justGetBlock(x - 1, y + 1, z - 1);
		auto bupbackright = justGetBlock(x + 1, y + 1, z - 1);

		//if (bfront == nullptr && front != nullptr)
		//{
		//	bfront = front->safeGet(x, y, 0);
		//}
		//
		//if (bdownfront == nullptr && front != nullptr)
		//{
		//	bdownfront = front->safeGet(x, y-1, 0);
		//}
		//
		//if (bback == nullptr && back != nullptr)
		//{
		//	bback = back->safeGet(x, y, CHUNK_SIZE - 1);
		//}
		//
		//if (bdownback == nullptr && back != nullptr)
		//{
		//	bdownback = back->safeGet(x, y-1, CHUNK_SIZE - 1);
		//}
		//
		//
		//if (bleft == nullptr && left != nullptr)
		//{
		//	bleft = left->safeGet(CHUNK_SIZE - 1, y, z);
		//}
		//
		//if (bdownleft == nullptr && left != nullptr)
		//{
		//	bdownleft = left->safeGet(CHUNK_SIZE - 1, y-1, z);
		//}
		//
		//if (bright == nullptr && right != nullptr)
		//{
		//	bright = right->safeGet(0, y, z);
		//}
		//
		//if (bdownright == nullptr && right != nullptr)
		//{
		//	bdownright = right->safeGet(0, y-1, z);
		//}
		//
		////
		//if (bupfront == nullptr && front != nullptr)
		//{
		//	bupfront = front->safeGet(x, y + 1, 0);
		//}
		//
		//if (bupback == nullptr && back != nullptr)
		//{
		//	bupback = back->safeGet(x, y + 1, CHUNK_SIZE - 1);
		//}
		//
		//if (bupright == nullptr && right != nullptr)
		//{
		//	bupright = right->safeGet(0, y + 1, z);
		//}
		//
		//if (bupleft == nullptr && left != nullptr)
		//{
		//	bupleft = left->safeGet(CHUNK_SIZE - 1, y + 1, z);
		//}
		//
		//// corners
		//if (bupfrontLeft == nullptr && frontLeft != nullptr)
		//{
		//	bupfrontLeft = frontLeft->safeGet(CHUNK_SIZE - 1, y + 1, 0);
		//}
		//
		//if (bupfrontright == nullptr && frontRight != nullptr)
		//{
		//	bupfrontright = frontRight->safeGet(0, y + 1, 0);
		//}
		//
		//if (bupbackleft == nullptr && backLeft != nullptr)
		//{
		//	bupbackleft = backLeft->safeGet(CHUNK_SIZE - 1, y + 1, CHUNK_SIZE - 1);
		//}
		//
		//if (bupbackright == nullptr && backRight != nullptr)
		//{
		//	bupbackright = backRight->safeGet(0, y + 1, CHUNK_SIZE - 1);
		//}

		sides[0] = bfront;
		sides[1] = bback;
		sides[2] = btop;
		sides[3] = bbottom;
		sides[4] = bleft;
		sides[5] = bright;

		sides[6] = bdownfront;
		sides[7] = bdownback;
		sides[8] = bdownleft;
		sides[9] = bdownright;

		sides[10] = bupfront;
		sides[11] = bupback;
		sides[12] = bupleft;
		sides[13] = bupright;

		sides[14] = bupfrontLeft;
		sides[15] = bupfrontright;
		sides[16] = bupbackleft;
		sides[17] = bupbackright;

	};


	auto blockBakeLogicForSolidBlocks = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b, bool isAnimated)
	{
		Block *sides[18] = {};
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

				int aoShape = 0;

				if (i == 2) //top
				{
					bool upFront = (sides[UP_FRONT] && sides[UP_FRONT]->isOpaque());
					bool upBack = (sides[UP_BACK] && sides[UP_BACK]->isOpaque());
					bool upLeft = (sides[UP_LEFT] && sides[UP_LEFT]->isOpaque());
					bool upRight = (sides[UP_RIGHT] && sides[UP_RIGHT]->isOpaque());

					bool upFrontLeft = (sides[UP_FRONTLEFT] && sides[UP_FRONTLEFT]->isOpaque());
					bool upFrontRight = (sides[UP_FRONTRIGHT] && sides[UP_FRONTRIGHT]->isOpaque());
					bool upBackLeft = (sides[UP_BACKLEFT] && sides[UP_BACKLEFT]->isOpaque());
					bool upBackRight = (sides[UP_BACKRIGHT] && sides[UP_BACKRIGHT]->isOpaque());

					aoShape = 0;


					if (upFrontLeft)
					{
						aoShape = 6;
					}
					else if (upFrontRight)
					{
						aoShape = 7;
					}
					else if (upBackLeft)
					{
						aoShape = 8;
					}
					else if (upBackRight)
					{
						aoShape = 5;
					}


					if (upFront || (upFrontRight && upFrontLeft))
					{
						aoShape = 1;
					}
					else if (upBack || (upBackRight && upBackLeft))
					{
						aoShape = 2;
					}
					else if (upLeft || (upBackLeft && upFrontLeft))
					{
						aoShape = 3;
					}
					else if (upRight || (upBackRight && upFrontRight))
					{
						aoShape = 4;
					}


					//opposite corners
					if ((upFrontLeft && upBackRight)
						|| (upFrontRight && upBackLeft))
					{
						aoShape = 14; 
					}


					//darker corners
					if ((upFront && (upLeft || upBackLeft)) || 
						(upBackLeft && upFrontLeft && upFrontRight) || (upLeft && upFrontRight) )
					{
						aoShape = 10;
					}
					else if (upFront && (upRight || upBackRight) ||
						(upFrontLeft && upFrontRight && upBackRight) || (upRight && upFrontLeft)
						)
					{
						aoShape = 11;
					}
					else if (upBack && (upLeft || upFrontLeft) ||
						(upFrontLeft && upBackLeft && upBackRight) || (upLeft && upBackRight)

						)
					{
						aoShape = 12;
					}
					else if (upBack && (upRight || upFrontRight) ||
						(upBackLeft && upBackRight && upFrontRight) || (upRight && upBackLeft)
						)
					{
						aoShape = 9;
					}

					bool backLeftCorner = upBack || upBackLeft || upLeft;
					bool backRightCorner = upBack || upBackRight || upRight;
					bool frontLeftCorner = upFront || upFrontLeft || upLeft;
					bool frontRightCorner = upFront || upFrontRight || upRight;
					
					if(backLeftCorner && backRightCorner && frontLeftCorner && frontRightCorner)
					{
						aoShape = 13; //full shaodw
					}

				}

				bool isInWater = (sides[i] != nullptr) && sides[i]->type == BlockTypes::water;

				if (dontUpdateLightSystem)
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater, 
						15, 15, aoShape);

				}
				else if (isLightEmitor(b.type))
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						b.getSkyLight(), 15, aoShape);
					//todo transparent block should emit internal light
				}else
				if (i == 2 && y == CHUNK_HEIGHT - 1)
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						15, b.getLight(), aoShape);
				}
				else if (y == 0 && i == 3)
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						15, b.getLight(), aoShape); //bottom of the world
					
				}
				else if (sides[i] != nullptr)
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						sides[i]->getSkyLight(), sides[i]->getLight(), aoShape);
				}
				else
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						0,0, aoShape);
				}
				
			}
		}
	};

	auto blockBakeLogicForTransparentBlocks = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b, bool isAnimated)
	{


		Block *sides[18] = {};
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
					//front back top bottom left right
					if (i == 0 || i == 1 || i == 4 || i == 5)
					{
						int currentIndex = i;

						//bootom variant
						bool bottomVariant = 0;

						auto doABottomCheck = [&](int checkDown)
						{
							if (sides[BOTTOM])
							{
								auto blockBottom = *sides[BOTTOM];
								if (blockBottom.type == BlockTypes::water)
								{
									if (sides[checkDown]
										&& (sides[checkDown]->type == BlockTypes::water)
										)
									{
										bottomVariant = true;
									}
								}
							}
						};

						if (i == FRONT)
						{
							doABottomCheck(DOWN_FRONT);
						}
						else if (i == BACK)
						{
							doABottomCheck(DOWN_BACK);
						}
						else if (i == LEFT)
						{
							doABottomCheck(DOWN_LEFT);
						}
						else if (i == RIGHT)
						{
							doABottomCheck(DOWN_RIGHT);
						}

						bool topVariant = 1;
						if (y < CHUNK_HEIGHT - 1)
						{
							auto blockTop = unsafeGet(x, y + 1, z);
							if (blockTop.type != BlockTypes::water)
							{
								currentIndex += 22;
								topVariant = false;
							}
						}

						if (topVariant && !bottomVariant)
						{
							currentIndex = i; //normal block
						}
						else if(topVariant && bottomVariant)
						{
							if (i == 0) { currentIndex = 32; } //front
							if (i == 1) { currentIndex = 33; } //back
							if (i == 4) { currentIndex = 34; } //front
							if (i == 5) { currentIndex = 35; } //front
						}
						else if (!topVariant && !bottomVariant)
						{
							//normal water
							currentIndex = i + 22;
						}
						else if (!topVariant && bottomVariant)
						{
							//bottom water;
							if (i == 0) { currentIndex = 28; } //front
							if (i == 1) { currentIndex = 29; } //back
							if (i == 4) { currentIndex = 30; } //front
							if (i == 5) { currentIndex = 31; } //front
						}

						currentVector->push_back(mergeShorts(currentIndex, getGpuIdIndexForBlock(b.type, i)));
					}
					else
					{
						currentVector->push_back(mergeShorts(i + 22, getGpuIdIndexForBlock(b.type, i)));
					}

					//if (!sides[2] || sides[2]->type != BlockTypes::water)
					//{
					//	currentVector->push_back(mergeShorts(i + 22, getGpuIdIndexForBlock(b.type, i)));
					//}

				}
				else
				{
					currentVector->push_back(mergeShorts(i, getGpuIdIndexForBlock(b.type, i)));
				}

				int aoShape = 0;
			
				bool isInWater = (sides[i] != nullptr) && sides[i]->type == BlockTypes::water;

				if (dontUpdateLightSystem)
				{
					pushFlagsLightAndPosition(*currentVector, position, isWater, isInWater,
						15, 15, aoShape);
				}
				else
					if (sides[i] == nullptr && i == 2)
					{
						pushFlagsLightAndPosition(*currentVector, position, isWater, isInWater,
							15, b.getLight(), aoShape);
					}
					else if (sides[i] == nullptr && i == 3)
					{
						pushFlagsLightAndPosition(*currentVector, position, isWater, isInWater,
							5, b.getLight(), aoShape);
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
							std::max(b.getLight(), sides[i]->getLight()), aoShape);
					}
					else
					{
						pushFlagsLightAndPosition(*currentVector, position, 
							isWater, isInWater,
							0, 0, aoShape);
					}

			}
		}
	};

	auto blockBakeLogicForGrassMesh = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b)
	{
		Block *sides[18] = {};
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
				pushFlagsLightAndPosition(*currentVector, position, 0, 0, 15, 15, 0);
			}
			else
			{
				pushFlagsLightAndPosition(*currentVector, position,
					0, 0, b.getSkyLight(), b.getLight(), 0);
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
				pushFlagsLightAndPosition(*currentVector, position, 0, 0, 15, 15, 0);
			}
			else
			{
				pushFlagsLightAndPosition(*currentVector, position,
					0, 0, b.getSkyLight(), b.getLight(), 0);
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

						if (b.isLightEmitor())
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

		//glBufferStorage(GL_ARRAY_BUFFER, opaqueGeometry.size() * sizeof(opaqueGeometry[0]),
		//	opaqueGeometry.data(), GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
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
		//glBufferStorage(GL_ARRAY_BUFFER, transparentGeometry.size() * sizeof(transparentGeometry[0]),
		//	transparentGeometry.data(), GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

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

glm::ivec2 modBlockToChunk(glm::ivec2 x)
{
	return glm::ivec2(modBlockToChunk(x.x), modBlockToChunk(x.y));
}
