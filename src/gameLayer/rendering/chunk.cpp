#include <chunk.h>
#include <rendering/renderer.h>
#include <blocksLoader.h>
#include <lightSystem.h>
#include <iostream>
#include <rendering/bigGpuBuffer.h>
#include <platformTools.h>
#include <algorithm>
#include <array>
#include <gameplay/entity.h>

#undef max
#undef min

constexpr int halfBottomStartGeometry = 36;
constexpr int cornerUpStartGeometry = 40;
constexpr int topHalfStartGeometry = 48;
constexpr int topHalfBottomPartStartGeometry = 52;
constexpr int frontalMiddleTopPieceStartGeometry = 56;
constexpr int slabTopFace = 60; //in the middle
constexpr int slabTopSides = 61;
constexpr int slabFaceUnderside = 65; //in the middle
constexpr int wallsInnerFace = 66;
constexpr int wallsBottomPart = 70;
constexpr int wallsSideParts = 74;
constexpr int lod1Parts = 82;


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



void arangeData(std::vector<int> &currentVector)
{
	glm::ivec4 *geometryArray = reinterpret_cast<glm::ivec4 *>(currentVector.data());
	permaAssertComment(currentVector.size() % 4 == 0, "baking vector corrupted...");
	size_t numElements = currentVector.size() / 4;

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

void pushFaceShapeTextureAndColor(std::vector<int> &vect, short shape, short texture, unsigned short color)
{

	// texture and color format
	// obxxxxx000'00000000
	//  color    texture

	//permaAssertComment(texture < 1000, "Too many textures");
	if (texture > 1000) { texture = 0; }

	unsigned short textureAndColor = texture;
	color <<= 11;

	textureAndColor |= color;

	vect.push_back(mergeShortsUnsigned(shape, textureAndColor));


}


void pushFlagsLightAndPosition(std::vector<int> &vect,
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


inline uint32_t hash(int x, int y, int z)
{
	uint32_t h = static_cast<uint32_t>(x) * 374761393 +
		static_cast<uint32_t>(y) * 668265263 +
		static_cast<uint32_t>(z) * 2147483647;
	h ^= (h >> 13);
	h *= 1274126177;
	h ^= (h >> 16);
	return h;
}

bool getRandomChance(int x, int y, int z, float chance)
{
	std::minstd_rand rng;
	rng.seed(hash(x,y,z));

	return getRandomChance(rng, chance);
}


float getRandomNumberFloat(int x, int y, int z, float a, float b)
{
	std::minstd_rand rng;
	rng.seed(hash(x, y, z));

	return getRandomNumberFloat(rng, a, b);
}

bool Chunk::bake(Chunk *left, Chunk *right, Chunk *front, Chunk *back, 
	Chunk *frontLeft, Chunk *frontRight, Chunk *backLeft, Chunk *backRight,
	glm::ivec3 playerPosition,
	std::vector<TransparentCandidate> &transparentCandidates,
	std::vector<int> &opaqueGeometry,
	std::vector<int> &transparentGeometry,
	std::vector<glm::ivec4> &lights, int lod, Renderer &renderer
	)
{

	bool updateGeometry = 0;
	bool updateTransparency = 0;

	bakeAndDontSendDataToOpenGl(left, right, front, back, frontLeft, frontRight, backLeft,
		backRight, playerPosition, transparentCandidates, opaqueGeometry,
		transparentGeometry, lights, updateGeometry, updateTransparency, lod, renderer);

	//send data to GPU
	sendDataToOpenGL(updateGeometry, updateTransparency, transparentCandidates,
		opaqueGeometry, transparentGeometry, lights);

	if ((updateTransparency && transparentElementCountSize > 0) || updateGeometry)
	{
		return true;
	}
	else
	{
		return false;
	}

}



bool Chunk::bakeAndDontSendDataToOpenGl(Chunk *left,
	Chunk *right, Chunk *front,
	Chunk *back, Chunk *frontLeft, 
	Chunk *frontRight, Chunk *backLeft, Chunk *backRight, 
	glm::ivec3 playerPosition, std::vector<TransparentCandidate> &transparentCandidates, 
	std::vector<int> &opaqueGeometry, std::vector<int> &transparentGeometry, 
	std::vector<glm::ivec4> &lights,
	bool &updateGeometry,
	bool &updateTransparency, int lod, Renderer &renderer)
{

	updateGeometry = 0;
	updateTransparency = isDirtyTransparency();

	if (
		isDirty()
		|| (!isNeighbourToLeft() && left != nullptr)
		|| (!isNeighbourToRight() && right != nullptr)
		|| (!isNeighbourToFront() && front != nullptr)
		|| (!isNeighbourToBack() && back != nullptr)

		|| (!isNeighbourToFrontLeft() && frontLeft != nullptr)
		|| (!isNeighbourToFrontRight() && frontRight != nullptr)
		|| (!isNeighbourToBackLeft() && backLeft != nullptr)
		|| (!isNeighbourToBackRight() && backRight != nullptr)
		|| currentLod != lod
		)
	{
		updateGeometry = true;
		updateTransparency = true;
	}

	opaqueGeometry.clear();
	transparentGeometry.clear();
	transparentCandidates.clear();
	lights.clear();

	currentLod = lod;

#pragma region helpers

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

	const int DOWN_FRONTLEFT = 18;
	const int DOWN_FRONTRIGHT = 19;
	const int DOWN_BACKLEFT = 20;
	const int DOWN_BACKRIGHT = 21;

	const int FRONTLEFT = 22;
	const int FRONTRIGHT = 23;
	const int BACKLEFT = 24;
	const int BACKRIGHT = 25;

	auto getGpuIdIndexForBlockWithVariation = [&](BlockType type, int i,
		int x, int y, int z) -> std::uint16_t
	{

		if (type == bricks || type == bricks_wall || type == bricks_slabs || type == bricks_stairs)
		{
			if (getRandomChance(x, y, z, 0.2))
			{
				return BRICKS_VARIATION_TEXTURE_INDEX * 4;
			}
		}
		else if (type == blueBricks ||
			type == blueBricks_wall || type == blueBricks_slabs || type == blueBricks_stairs
			)
		{
			if (getRandomChance(x, y, z, 0.2))
			{
				return BLUE_BRICKS_VARIATION_TEXTURE_INDEX * 4;
			}
		}

		return getGpuIdIndexForBlock(type, i);
	};

	auto getNeighboursLogic = [&](int x, int y, int z, Block *sides[26], int lod)
	{
		auto justGetBlock = [&](int x, int y, int z) -> Block *
		{
			int HEIGHT = CHUNK_HEIGHT;
			int SIZE = CHUNK_SIZE;
			if (lod == 1) { HEIGHT = CHUNK_HEIGHT / 2; }
			if (lod == 1) { SIZE = CHUNK_SIZE / 2; }

			if (y >= HEIGHT || y < 0) { return nullptr; }

			if (x >= 0 && x < SIZE && z >= 0 && z < SIZE)
			{
				return &unsafeGet(x, y, z, lod);
			}

			if (x >= 0 && x < SIZE)
			{
				//z is the problem
				if (z < 0)
				{
					if (back)
					{
						return &back->unsafeGet(x, y, SIZE - 1, lod);
					}
				}
				else
				{
					if (front)
					{
						return &front->unsafeGet(x, y, 0, lod);
					}
				}
			}
			else if (z >= 0 && z < SIZE)
			{
				//x is the problem
				if (x < 0)
				{
					if (left)
					{
						return &left->unsafeGet(SIZE - 1, y, z, lod);
					}
				}
				else
				{
					if (right)
					{
						return &right->unsafeGet(0, y, z, lod);
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
						return &backLeft->unsafeGet(SIZE - 1, y, SIZE - 1, lod);
					}
				}
				else
					if (x >= SIZE && z < 0)
					{
						if (backRight)
						{
							return &backRight->unsafeGet(0, y, SIZE - 1, lod);
						}
					}
					else if (x < 0 && z >= SIZE)
					{
						if (frontLeft)
						{
							return &frontLeft->unsafeGet(SIZE - 1, y, 0, lod);
						}
					}
					else
						if (x >= SIZE && z >= SIZE)
						{
							if (frontRight)
							{
								return &frontRight->unsafeGet(0, y, 0, lod);
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

		auto bdownfront = justGetBlock(x, y - 1, z + 1);
		auto bdownback = justGetBlock(x, y - 1, z - 1);
		auto bdownleft = justGetBlock(x - 1, y - 1, z);
		auto bdownright = justGetBlock(x + 1, y - 1, z);

		auto bupfront = justGetBlock(x, y + 1, z + 1);
		auto bupback = justGetBlock(x, y + 1, z - 1);
		auto bupleft = justGetBlock(x - 1, y + 1, z);
		auto bupright = justGetBlock(x + 1, y + 1, z);

		auto bupfrontLeft = justGetBlock(x - 1, y + 1, z + 1);
		auto bupfrontright = justGetBlock(x + 1, y + 1, z + 1);
		auto bupbackleft = justGetBlock(x - 1, y + 1, z - 1);
		auto bupbackright = justGetBlock(x + 1, y + 1, z - 1);

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

		sides[18] = justGetBlock(x - 1, y - 1, z + 1);
		sides[19] = justGetBlock(x + 1, y - 1, z + 1);
		sides[20] = justGetBlock(x - 1, y - 1, z - 1);
		sides[21] = justGetBlock(x + 1, y - 1, z - 1);

		sides[22] = justGetBlock(x - 1, y, z + 1);
		sides[23] = justGetBlock(x + 1, y, z + 1);
		sides[24] = justGetBlock(x - 1, y, z - 1);
		sides[25] = justGetBlock(x + 1, y, z - 1);

	};

	auto determineAOShape = [&](int i, Block *sides[26])
	{
		int aoShape = 0;

		auto calculateSide = [](Block *sides[26], bool front,
			bool back, bool left, bool right,
			bool frontLeft, bool frontRight, bool backLeft, bool backRight)
		{

			int aoShape = 0;

			if (frontLeft)
			{
				aoShape = 6;
			}
			else if (frontRight)
			{
				aoShape = 7;
			}
			else if (backLeft)
			{
				aoShape = 8;
			}
			else if (backRight)
			{
				aoShape = 5;
			}


			if (front || (frontRight && frontLeft))
			{
				aoShape = 1;
			}
			else if (back || (backRight && backLeft))
			{
				aoShape = 2;
			}
			else if (left || (backLeft && frontLeft))
			{
				aoShape = 3;
			}
			else if (right || (backRight && frontRight))
			{
				aoShape = 4;
			}


			//opposite corners
			if ((frontLeft && backRight)
				|| (frontRight && backLeft))
			{
				aoShape = 14;
			}


			//darker corners
			if ((front && (left || backLeft)) ||
				(backLeft && frontLeft && frontRight) || (left && frontRight))
			{
				aoShape = 10;
			}
			else if (front && (right || backRight) ||
				(frontLeft && frontRight && backRight) || (right && frontLeft)
				)
			{
				aoShape = 11;
			}
			else if (back && (left || frontLeft) ||
				(frontLeft && backLeft && backRight) || (left && backRight)

				)
			{
				aoShape = 12;
			}
			else if (back && (right || frontRight) ||
				(backLeft && backRight && frontRight) || (right && backLeft)
				)
			{
				aoShape = 9;
			}

			bool backLeftCorner = back || backLeft || left;
			bool backRightCorner = back || backRight || right;
			bool frontLeftCorner = front || frontLeft || left;
			bool frontRightCorner = front || frontRight || right;

			if (backLeftCorner && backRightCorner && frontLeftCorner && frontRightCorner)
			{
				aoShape = 13; //full shaodw
			}

			return aoShape;
		};

		if (i == 0) // front
		{
			bool upFront = (sides[UP_FRONT] && sides[UP_FRONT]->isOpaque());
			bool downFront = (sides[DOWN_FRONT] && sides[DOWN_FRONT]->isOpaque());
			bool leftFront = (sides[FRONTLEFT] && sides[FRONTLEFT]->isOpaque());
			bool rightFront = (sides[FRONTRIGHT] && sides[FRONTRIGHT]->isOpaque());

			bool upFrontLeft = (sides[UP_FRONTLEFT] && sides[UP_FRONTLEFT]->isOpaque());
			bool upFrontRight = (sides[UP_FRONTRIGHT] && sides[UP_FRONTRIGHT]->isOpaque());
			bool downFrontLeft = (sides[DOWN_FRONTLEFT] && sides[DOWN_FRONTLEFT]->isOpaque());
			bool downFrontRight = (sides[DOWN_FRONTRIGHT] && sides[DOWN_FRONTRIGHT]->isOpaque());

			aoShape = calculateSide(sides,
				leftFront, rightFront, upFront, downFront,
				upFrontLeft, downFrontLeft, upFrontRight,
				downFrontRight);
		}
		else if (i == 1) // back
		{
			bool upBack = (sides[UP_BACK] && sides[UP_BACK]->isOpaque());
			bool downBack = (sides[DOWN_BACK] && sides[DOWN_BACK]->isOpaque());
			bool leftBack = (sides[BACKLEFT] && sides[BACKLEFT]->isOpaque());
			bool rightBack = (sides[BACKRIGHT] && sides[BACKRIGHT]->isOpaque());

			bool upBackLeft = (sides[UP_BACKLEFT] && sides[UP_BACKLEFT]->isOpaque());
			bool upBackRight = (sides[UP_BACKRIGHT] && sides[UP_BACKRIGHT]->isOpaque());
			bool downBackLeft = (sides[DOWN_BACKLEFT] && sides[DOWN_BACKLEFT]->isOpaque());
			bool downBackRight = (sides[DOWN_BACKRIGHT] && sides[DOWN_BACKRIGHT]->isOpaque());

			aoShape = calculateSide(sides,
				leftBack, rightBack, upBack, downBack,
				upBackLeft, downBackLeft, upBackRight,
				downBackRight);
		}
		else
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

				aoShape = calculateSide(sides, upFront, upBack, upLeft, upRight, upFrontLeft, upFrontRight,
					upBackLeft, upBackRight);
			}
			else
				if (i == 3) //bottom
				{
					bool downFront = (sides[DOWN_FRONT] && sides[DOWN_FRONT]->isOpaque());
					bool downBack = (sides[DOWN_BACK] && sides[DOWN_BACK]->isOpaque());
					bool downLeft = (sides[DOWN_LEFT] && sides[DOWN_LEFT]->isOpaque());
					bool downRight = (sides[DOWN_RIGHT] && sides[DOWN_RIGHT]->isOpaque());

					bool downFrontLeft = (sides[DOWN_FRONTLEFT] && sides[DOWN_FRONTLEFT]->isOpaque());
					bool downFrontRight = (sides[DOWN_FRONTRIGHT] && sides[DOWN_FRONTRIGHT]->isOpaque());
					bool downBackLeft = (sides[DOWN_BACKLEFT] && sides[DOWN_BACKLEFT]->isOpaque());
					bool downBackRight = (sides[DOWN_BACKRIGHT] && sides[DOWN_BACKRIGHT]->isOpaque());

					aoShape = calculateSide(sides,
						downLeft, downRight, downFront, downBack,
						downFrontLeft, downBackLeft, downFrontRight, downBackRight
					);
				}
				else if (i == 4) //left
				{
					bool upLeft = (sides[UP_LEFT] && sides[UP_LEFT]->isOpaque());
					bool downLeft = (sides[DOWN_LEFT] && sides[DOWN_LEFT]->isOpaque());
					bool leftFront = (sides[FRONTLEFT] && sides[FRONTLEFT]->isOpaque());
					bool leftBack = (sides[BACKLEFT] && sides[BACKLEFT]->isOpaque());

					bool upFrontLeft = (sides[UP_FRONTLEFT] && sides[UP_FRONTLEFT]->isOpaque());
					bool upBackLeft = (sides[UP_BACKLEFT] && sides[UP_BACKLEFT]->isOpaque());
					bool downFrontLeft = (sides[DOWN_FRONTLEFT] && sides[DOWN_FRONTLEFT]->isOpaque());
					bool downBackLeft = (sides[DOWN_BACKLEFT] && sides[DOWN_BACKLEFT]->isOpaque());

					aoShape = calculateSide(sides,
						leftBack, leftFront, upLeft, downLeft,
						upBackLeft, downBackLeft, upFrontLeft, downFrontLeft
					);

				}
				else if (i == 5) //right
				{
					bool upRight = (sides[UP_RIGHT] && sides[UP_RIGHT]->isOpaque());
					bool downRight = (sides[DOWN_RIGHT] && sides[DOWN_RIGHT]->isOpaque());
					bool rightFront = (sides[FRONTRIGHT] && sides[FRONTRIGHT]->isOpaque());
					bool rightBack = (sides[BACKRIGHT] && sides[BACKRIGHT]->isOpaque());

					bool upFrontRight = (sides[UP_FRONTRIGHT] && sides[UP_FRONTRIGHT]->isOpaque());
					bool upBackRight = (sides[UP_BACKRIGHT] && sides[UP_BACKRIGHT]->isOpaque());
					bool downFrontRight = (sides[DOWN_FRONTRIGHT] && sides[DOWN_FRONTRIGHT]->isOpaque());
					bool downBackRight = (sides[DOWN_BACKRIGHT] && sides[DOWN_BACKRIGHT]->isOpaque());

					aoShape = calculateSide(sides,
						rightFront, rightBack, upRight, downRight,
						upFrontRight, downFrontRight, upBackRight, downBackRight
					);

				}

		return aoShape;
	};

	auto blockBakeLogicForSolidBlocks = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b, bool isAnimated)
	{
		Block *sides[26] = {};
		getNeighboursLogic(x, y, z, sides, 0);

		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y, z + this->data.z * CHUNK_SIZE};
		auto type = b.getType();
		auto color = b.getColor();

		for (int i = 0; i < 6; i++)
		{

			if (
				(!(isAnyLeaves(b.getType()) && sides[i] != nullptr && isAnyLeaves((sides[i])->getType()))
				&&
				(sides[i] != nullptr && !(sides[i])->isOpaque()))
				||
				(
				//(i == 3 && y == 0) ||		//display the bottom face
				(i == 2 && y == CHUNK_HEIGHT - 1) //display the top face
				)
				)
			{

				pushFaceShapeTextureAndColor(*currentVector, i + isAnimated * 10,
					getGpuIdIndexForBlockWithVariation(type, i, x, y, z), color);


				int aoShape = determineAOShape(i, sides);
				bool isInWater = (sides[i] != nullptr) && sides[i]->getType() == BlockTypes::water;

				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				if (dontUpdateLightSystem)
				{
					sunLight = 15;
					torchLight = 0;
				}
				else if (isLightEmitor(b.getType()))
				{
					if (sides[i]) { sunLight = sides[i]->getSkyLight(); }
					torchLight = 15;
				}
				else if (i == 2 && y == CHUNK_HEIGHT - 1)
				{
					sunLight = 15; //top face;

				}
				else if (y == 0 && i == 3)
				{
					sunLight = 10; //bottom face;

				}
				else  if (sides[i] != nullptr)
				{
					sunLight = sides[i]->getSkyLight();
					torchLight = sides[i]->getLight();
				}
				///
				if (isTransparentGeometry(b.getType()))
				{
					sunLight = std::max(sunLight, b.getSkyLight());
					torchLight = std::max(torchLight, b.getLight());
				}

				pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
					sunLight, torchLight, aoShape);

			}
		}
	};

	//todo reuse up
	auto calculateLightThings = [&](unsigned char &sunLight, unsigned char &torchLight,
		Block *side, Block &b, int i, int y)
	{
		sunLight = 0;
		torchLight = 0;
		if (dontUpdateLightSystem)
		{
			sunLight = 15;
			torchLight = 0;
		}
		else if (isLightEmitor(b.getType()))
		{
			if (side) { sunLight = side->getSkyLight(); }
			torchLight = 15;
		}
		else if (i == 2 && y == CHUNK_HEIGHT - 1)
		{
			sunLight = 15; //top face;

		}
		else if (y == 0 && i == 3)
		{
			sunLight = 10; //bottom face;

		}
		else  if (side != nullptr)
		{
			sunLight = side->getSkyLight();
			torchLight = side->getLight();
		}
		///
		if (isTransparentGeometry(b.getType()))
		{
			sunLight = std::max(sunLight, b.getSkyLight());
			torchLight = std::max(torchLight, b.getLight());
		}
	};

	auto blockBakeLogicForLods = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b)
	{
		Block *sides[26] = {};
		getNeighboursLogic(x, y, z, sides, 1);

		glm::ivec3 position = {(x * 2) + this->data.x * CHUNK_SIZE, y * 2, (z * 2) + this->data.z * CHUNK_SIZE};

		for (int i = 0; i < 6; i++)
		{

			if (
				(!(isAnyLeaves(b.getType()) && sides[i] != nullptr && isAnyLeaves((sides[i])->getType()))
				&&
				(sides[i] != nullptr && !(sides[i])->isOpaque()))
				||
				(
				//(i == 3 && y == 0) ||		//display the bottom face
				(i == 2 && y == CHUNK_HEIGHT/2 - 1) //display the top face
				)
				)
			{

				auto type = b.getType();
				currentVector->push_back(mergeShorts(i + lod1Parts,
					getGpuIdIndexForBlock(type, i)));

				int aoShape = determineAOShape(i, sides);
				bool isInWater = (sides[i] != nullptr) && sides[i]->getType() == BlockTypes::water;

				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				calculateLightThings(sunLight, torchLight, sides[i], b, i, y);

				//todo idea instead of is whater type flag why not just look for the texture id maybe?
				pushFlagsLightAndPosition(*currentVector, position, type == BlockTypes::water, isInWater,
					sunLight, torchLight, aoShape);

			}
		}
	};

	auto blockBakeLogicForStairs = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b)
	{
		Block *sides[26] = {};
		getNeighboursLogic(x, y, z, sides, 0);

		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y, z + this->data.z * CHUNK_SIZE};

		int rotation = b.getRotationFor365RotationTypeBlocks();

		for (int i = 0; i < 6; i++)
		{

			if (
				(sides[i] != nullptr && !(sides[i])->isOpaque())
				|| (
				//(i == 3 && y == 0) ||		//display the bottom face
				(i == 2 && y == CHUNK_HEIGHT - 1) //display the top face
				)
				)
			{

				int aoShape = determineAOShape(i, sides);
				bool isInWater = (sides[i] != nullptr) && sides[i]->getType() == BlockTypes::water;
				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				calculateLightThings(sunLight, torchLight, sides[i], b, i, y);


				auto placeFlagsLightsNormally = [&]()
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						sunLight, torchLight, aoShape);
				};

				auto placeNormally = [&]()
				{
					currentVector->push_back(mergeShorts(i,
						getGpuIdIndexForBlockWithVariation(b.getType(), i, x, y, z)));
					placeFlagsLightsNormally();
				};

				if (i == 0)
				{
					//front
					if (rotation == 2)
					{
						placeNormally();
					}
					else
					{
						//bottom rim
						currentVector->push_back(mergeShorts(halfBottomStartGeometry + 0
							, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z)

						));
						placeFlagsLightsNormally();

						if (rotation == 1)
						{
							currentVector->push_back(mergeShorts(cornerUpStartGeometry + 4
								, getGpuIdIndexForBlockWithVariation(b.getType(), 1, x, y, z))
							);
							placeFlagsLightsNormally();
						}
						else
							if (rotation == 3)
							{
								currentVector->push_back(mergeShorts(cornerUpStartGeometry + 6
									, getGpuIdIndexForBlockWithVariation(b.getType(), 1, x, y, z)
								));
								placeFlagsLightsNormally();
							}
					}

				}
				else if (i == 1)
				{
					//back
					if (rotation == 0)
					{
						placeNormally();
					}
					else
					{
						//bottom rim
						currentVector->push_back(mergeShorts(halfBottomStartGeometry + 1
							, getGpuIdIndexForBlockWithVariation(b.getType(), 1, x, y, z)
						));
						placeFlagsLightsNormally();

						if (rotation == 1)
						{
							currentVector->push_back(mergeShorts(cornerUpStartGeometry + 5
								, getGpuIdIndexForBlockWithVariation(b.getType(), 1, x, y, z)
							));
							placeFlagsLightsNormally();
						}
						else
							if (rotation == 3)
							{
								currentVector->push_back(mergeShorts(cornerUpStartGeometry + 7
									, getGpuIdIndexForBlockWithVariation(b.getType(), 1, x, y, z)
								));
								placeFlagsLightsNormally();
							}
					}

				}
				else if (i == 2)
				{
					//top

				}
				else if (i == 3)
				{
					//bottom
					placeNormally();
				}
				else if (i == 4)
				{
					//left
					if (rotation == 1)
					{
						placeNormally();
					}
					else
					{
						//bottom rim
						currentVector->push_back(mergeShorts(halfBottomStartGeometry + 2
							, getGpuIdIndexForBlockWithVariation(b.getType(), 2, x, y, z)
						));
						placeFlagsLightsNormally();

						if (rotation == 0)
						{
							currentVector->push_back(mergeShorts(cornerUpStartGeometry + 0
								, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z)
							));
							placeFlagsLightsNormally();
						}
						else if (rotation == 2)
						{
							currentVector->push_back(mergeShorts(cornerUpStartGeometry + 2
								, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z)
							));
							placeFlagsLightsNormally();
						}
					}


				}
				else if (i == 5)
				{
					//right
					if (rotation == 3)
					{
						placeNormally();
					}
					else
					{
						//bottom rim
						currentVector->push_back(mergeShorts(halfBottomStartGeometry + 3
							, getGpuIdIndexForBlockWithVariation(b.getType(), 3, x, y, z)
						));
						placeFlagsLightsNormally();

						if (rotation == 0)
						{
							currentVector->push_back(mergeShorts(cornerUpStartGeometry + 1
								, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z)
							));
							placeFlagsLightsNormally();
						}
						else
							if (rotation == 2)
							{
								currentVector->push_back(mergeShorts(cornerUpStartGeometry + 3
									, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z)
								));
								placeFlagsLightsNormally();
							}

					}

				}



			}
		}


		//top face
		if (sides[2] != nullptr && !(sides[2])->isOpaque()
			|| y == CHUNK_HEIGHT - 1
			)
		{

			int aoShape = determineAOShape(2, sides);
			bool isInWater = (sides[2] != nullptr) && sides[2]->getType() == BlockTypes::water;
			unsigned char sunLight = 0;
			unsigned char torchLight = 0;
			calculateLightThings(sunLight, torchLight, sides[2], b, 2, y);

			auto placeFlagsLightsNormally = [&]()
			{
				pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
					sunLight, torchLight, aoShape);
			};

			if (rotation == 0)
			{
				currentVector->push_back(mergeShorts(topHalfStartGeometry + 0
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();
			}
			else if (rotation == 2)
			{
				currentVector->push_back(mergeShorts(topHalfStartGeometry + 1
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();
			}
			else if (rotation == 1)
			{
				currentVector->push_back(mergeShorts(topHalfStartGeometry + 2
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();
			}
			else if (rotation == 3)
			{
				currentVector->push_back(mergeShorts(topHalfStartGeometry + 3
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();
			}
		}

		//middle faces
		if (
			(y == CHUNK_HEIGHT - 1) ||
			(sides[0] != nullptr && !(sides[0])->isOpaque()) ||
			(sides[1] != nullptr && !(sides[1])->isOpaque()) ||
			(sides[2] != nullptr && !(sides[2])->isOpaque()) ||
			(sides[4] != nullptr && !(sides[4])->isOpaque()) ||
			(sides[5] != nullptr && !(sides[5])->isOpaque())
			)
		{
			int aoShape = determineAOShape(2, sides);
			//bool isInWater = (sides[2] != nullptr) && sides[2]->getType() == BlockTypes::water;
			bool isInWater = 0;
			unsigned char sunLight = 0;
			unsigned char torchLight = 0;
			calculateLightThings(sunLight, torchLight, sides[2], b, 2, y);

			auto placeFlagsLightsNormally = [&]()
			{
				pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
					sunLight, torchLight, aoShape);
			};

			if (rotation == 0)
			{
				currentVector->push_back(mergeShorts(topHalfBottomPartStartGeometry + 1
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();

				currentVector->push_back(mergeShorts(frontalMiddleTopPieceStartGeometry + 0
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();
			}
			else if (rotation == 2)
			{
				currentVector->push_back(mergeShorts(topHalfBottomPartStartGeometry + 0
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();

				currentVector->push_back(mergeShorts(frontalMiddleTopPieceStartGeometry + 1
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();
			}
			else if (rotation == 1)
			{
				currentVector->push_back(mergeShorts(topHalfBottomPartStartGeometry + 3
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();

				currentVector->push_back(mergeShorts(frontalMiddleTopPieceStartGeometry + 2
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();
			}
			else if (rotation == 3)
			{
				currentVector->push_back(mergeShorts(topHalfBottomPartStartGeometry + 2
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();

				currentVector->push_back(mergeShorts(frontalMiddleTopPieceStartGeometry + 3
					, getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
				)));
				placeFlagsLightsNormally();
			}


		}


	};

	auto blockBakeLogicForWalls = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b)
	{
		int sideFace = 0;

		if (b.getType() == logWall)
		{
			sideFace = 2;
		}

		Block *sides[26] = {};
		getNeighboursLogic(x, y, z, sides, 0);

		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y, z + this->data.z * CHUNK_SIZE};

		int rotation = b.getRotationFor365RotationTypeBlocks();

		for (int i = 0; i < 6; i++)
		{

			if (
				(sides[i] != nullptr && !(sides[i])->isOpaque())
				|| (
				//(i == 3 && y == 0) ||		//display the bottom face
				(i == 2 && y == CHUNK_HEIGHT - 1) //display the top face
				)
				)
			{

				int aoShape = determineAOShape(i, sides);
				bool isInWater = (sides[i] != nullptr) && sides[i]->getType() == BlockTypes::water;
				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				calculateLightThings(sunLight, torchLight, sides[i], b, i, y);

				auto placeFlagsLightsNormally = [&]()
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						sunLight, torchLight, aoShape);
				};

				auto placeNormally = [&]()
				{
					currentVector->push_back(mergeShorts(i,
						getGpuIdIndexForBlockWithVariation(b.getType(), i, x, y, z)
					));
					placeFlagsLightsNormally();
				};

				if (i == 0)
				{
					//front
					if (rotation == 2)
					{
						placeNormally();
					}
					else
					{
						if (rotation == 1)
						{

							currentVector->push_back(mergeShorts(wallsSideParts + 0
								, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
							)));
							placeFlagsLightsNormally();
						}
						else if (rotation == 3)
						{
							currentVector->push_back(mergeShorts(wallsSideParts + 1
								, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
							)));
							placeFlagsLightsNormally();
						}
					}

				}
				else if (i == 1)
				{
					//back
					if (rotation == 0)
					{
						placeNormally();
					}
					else
					{
						if (rotation == 1)
						{
							currentVector->push_back(mergeShorts(wallsSideParts + 2
								, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
							)));
							placeFlagsLightsNormally();
						}
						else if (rotation == 3)
						{
							currentVector->push_back(mergeShorts(wallsSideParts + 3
								, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
							)));
							placeFlagsLightsNormally();
						}
					}

				}
				else if (i == 2)
				{
					if (rotation == 0)
					{
						currentVector->push_back(mergeShorts(topHalfStartGeometry + 0
							, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (rotation == 2)
					{
						currentVector->push_back(mergeShorts(topHalfStartGeometry + 1
							, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (rotation == 1)
					{
						currentVector->push_back(mergeShorts(topHalfStartGeometry + 2
							, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (rotation == 3)
					{
						currentVector->push_back(mergeShorts(topHalfStartGeometry + 3
							, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
						)));
						placeFlagsLightsNormally();
					}

				}
				else if (i == 3)
				{
					if (rotation == 0)
					{
						currentVector->push_back(mergeShorts(wallsBottomPart + 0
							, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (rotation == 2)
					{
						currentVector->push_back(mergeShorts(wallsBottomPart + 1
							, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (rotation == 1)
					{
						currentVector->push_back(mergeShorts(wallsBottomPart + 2
							, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (rotation == 3)
					{
						currentVector->push_back(mergeShorts(wallsBottomPart + 3
							, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
						)));
						placeFlagsLightsNormally();
					}
				}
				else if (i == 4)
				{
					//left
					if (rotation == 1)
					{
						placeNormally();
					}
					else
					{
						if (rotation == 0)
						{
							currentVector->push_back(mergeShorts(wallsSideParts + 4
								, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
							)));
							placeFlagsLightsNormally();
						}
						else if (rotation == 2)
						{
							currentVector->push_back(mergeShorts(wallsSideParts + 5
								, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
							)));
							placeFlagsLightsNormally();
						}
					}


				}
				else if (i == 5)
				{
					//right
					if (rotation == 3)
					{
						placeNormally();
					}
					else
					{
						if (rotation == 0)
						{
							currentVector->push_back(mergeShorts(wallsSideParts + 6
								, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
							)));
							placeFlagsLightsNormally();
						}
						else if (rotation == 2)
						{
							currentVector->push_back(mergeShorts(wallsSideParts + 7
								, getGpuIdIndexForBlockWithVariation(b.getType(), sideFace, x, y, z
							)));
							placeFlagsLightsNormally();
						}
					}

				}

			}

		}


		//inner face
		if (
			(sides[0] != nullptr && !sides[0]->isOpaque()) ||
			(sides[1] != nullptr && !sides[1]->isOpaque()) ||
			(sides[2] != nullptr && !sides[2]->isOpaque()) ||
			(sides[3] != nullptr && !sides[3]->isOpaque()) ||
			(sides[4] != nullptr && !sides[4]->isOpaque()) ||
			(sides[5] != nullptr && !sides[5]->isOpaque()) ||
			y == CHUNK_HEIGHT - 1 ||
			y == 0
			)
		{

			int aoShape = 0;
			bool isInWater = 0;
			unsigned char sunLight = b.getSkyLight();
			unsigned char torchLight = b.getLight();

			if (dontUpdateLightSystem)
			{
				sunLight = 15;
				torchLight = 0;
			}

			//calculateLightThings(sunLight, torchLight, sides[i], b, i, y);

			auto placeFlagsLightsNormally = [&]()
			{
				pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
					sunLight, torchLight, aoShape);
			};


			if (rotation == 0)
			{


				int i = 0; //front
				aoShape = determineAOShape(i, sides);
				currentVector->push_back(mergeShorts(wallsInnerFace,
					getGpuIdIndexForBlockWithVariation(b.getType(), i, x, y, z))
				);
				placeFlagsLightsNormally();
			}
			else if (rotation == 1)
			{
				int i = 5; //right
				aoShape = determineAOShape(i, sides);
				currentVector->push_back(mergeShorts(wallsInnerFace + 3,
					getGpuIdIndexForBlockWithVariation(b.getType(), i, x, y, z))
				);
				placeFlagsLightsNormally();
			}
			else if (rotation == 2)
			{
				int i = 1; //back
				aoShape = determineAOShape(i, sides);
				currentVector->push_back(mergeShorts(wallsInnerFace + 1,
					getGpuIdIndexForBlockWithVariation(b.getType(), i, x, y, z)));
				placeFlagsLightsNormally();
			}
			else if (rotation == 3)
			{
				int i = 4; //left
				aoShape = determineAOShape(i, sides);
				currentVector->push_back(mergeShorts(wallsInnerFace + 2,
					getGpuIdIndexForBlockWithVariation(b.getType(), i, x, y, z))
				);
				placeFlagsLightsNormally();
			}


		}

	};

	auto blockBakeLogicForSlabs = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b)
	{
		Block *sides[26] = {};
		getNeighboursLogic(x, y, z, sides, 0);


		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y, z + this->data.z * CHUNK_SIZE};

		if (b.getTopPartForSlabs())
		{
			for (int i = 0; i < 6; i++)
			{


				int aoShape = determineAOShape(i, sides);
				bool isInWater = (sides[i] != nullptr) && sides[i]->getType() == BlockTypes::water;
				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				calculateLightThings(sunLight, torchLight, sides[i], b, i, y);


				auto placeFlagsLightsNormally = [&]()
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						sunLight, torchLight, aoShape);
				};

				auto placeNormally = [&]()
				{
					currentVector->push_back(mergeShorts(i,
						getGpuIdIndexForBlockWithVariation(b.getType(), i, x, y, z)));
					placeFlagsLightsNormally();
				};

				if (
					(sides[i] != nullptr && !(sides[i])->isOpaque())
					|| (
					//(i == 3 && y == 0) ||		//display the bottom face
					(i == 2 && y == CHUNK_HEIGHT - 1) //display the top face
					)
					)
				{

					//bottom face
					if (i == 3)
					{
						//bottom face
					}
					else if (i == 2)
					{
						placeNormally();

					}
					else if (i == 0)
					{

						currentVector->push_back(mergeShorts(slabTopSides + 0,
							getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (i == 1)
					{
						currentVector->push_back(mergeShorts(slabTopSides + 1,
							getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (i == 4)
					{
						currentVector->push_back(mergeShorts(slabTopSides + 2,
							getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (i == 5)
					{
						currentVector->push_back(mergeShorts(slabTopSides + 3,
							getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
						)));
						placeFlagsLightsNormally();
					}


				}


			}

			//bottom face (in this case in the middle)
			if (
				(y == CHUNK_HEIGHT - 1) ||
				(sides[0] != nullptr && !(sides[0])->isOpaque()) ||
				(sides[1] != nullptr && !(sides[1])->isOpaque()) ||
				(sides[2] != nullptr && !(sides[2])->isOpaque()) ||
				(sides[4] != nullptr && !(sides[4])->isOpaque()) ||
				(sides[5] != nullptr && !(sides[5])->isOpaque())
				)
			{
				int aoShape = determineAOShape(2, sides);
				//bool isInWater = (sides[2] != nullptr) && sides[2]->getType() == BlockTypes::water;
				bool isInWater = 0;
				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				//calculateLightThings(sunLight, torchLight, sides[2], b, 2, y);

				if (dontUpdateLightSystem)
				{
					sunLight = 15;
					torchLight = 0;
				}
				else
				{
					sunLight = b.getSkyLight();
					torchLight = b.getLight();
				}

				currentVector->push_back(mergeShorts(slabFaceUnderside,
					getGpuIdIndexForBlockWithVariation(b.getType(), 2, x, y, z
				)));
				pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
					sunLight, torchLight, aoShape);
			}
		}
		else
		{
			for (int i = 0; i < 6; i++)
			{


				int aoShape = determineAOShape(i, sides);
				bool isInWater = (sides[i] != nullptr) && sides[i]->getType() == BlockTypes::water;
				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				calculateLightThings(sunLight, torchLight, sides[i], b, i, y);


				auto placeFlagsLightsNormally = [&]()
				{
					pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
						sunLight, torchLight, aoShape);
				};

				auto placeNormally = [&]()
				{
					currentVector->push_back(mergeShorts(i,
						getGpuIdIndexForBlockWithVariation(b.getType(), i, x, y, z
					)));
					placeFlagsLightsNormally();
				};

				if (
					(sides[i] != nullptr && !(sides[i])->isOpaque())
					|| (
					//(i == 3 && y == 0) ||		//display the bottom face
					(i == 2 && y == CHUNK_HEIGHT - 1) //display the top face
					)
					)
				{

					//bottom face
					if (i == 3)
					{
						placeNormally();
					}
					else if (i == 2)
					{
						//top face

					}
					else if (i == 0)
					{
						currentVector->push_back(mergeShorts(halfBottomStartGeometry + 0,
							getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (i == 1)
					{
						currentVector->push_back(mergeShorts(halfBottomStartGeometry + 1,
							getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (i == 4)
					{
						currentVector->push_back(mergeShorts(halfBottomStartGeometry + 2,
							getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
						)));
						placeFlagsLightsNormally();
					}
					else if (i == 5)
					{
						currentVector->push_back(mergeShorts(halfBottomStartGeometry + 3,
							getGpuIdIndexForBlockWithVariation(b.getType(), 0, x, y, z
						)));
						placeFlagsLightsNormally();
					}


				}


			}

			//top face (in this case in the middle)
			if (
				(y == CHUNK_HEIGHT - 1) ||
				(sides[0] != nullptr && !(sides[0])->isOpaque()) ||
				(sides[1] != nullptr && !(sides[1])->isOpaque()) ||
				(sides[2] != nullptr && !(sides[2])->isOpaque()) ||
				(sides[4] != nullptr && !(sides[4])->isOpaque()) ||
				(sides[5] != nullptr && !(sides[5])->isOpaque())
				)
			{
				int aoShape = determineAOShape(2, sides);

				if ((sides[2] != nullptr) && (sides[2]->isOpaque()))
				{
					aoShape == 13; //full shadow
				}

				//bool isInWater = (sides[2] != nullptr) && sides[2]->getType() == BlockTypes::water;
				bool isInWater = 0;
				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				//calculateLightThings(sunLight, torchLight, sides[2], b, 2, y);

				if (dontUpdateLightSystem)
				{
					sunLight = 15;
					torchLight = 0;
				}
				else
				{
					sunLight = b.getSkyLight();
					torchLight = b.getLight();
				}

				currentVector->push_back(mergeShorts(slabTopFace,
					getGpuIdIndexForBlockWithVariation(b.getType(), 2, x, y, z
				)));
				pushFlagsLightAndPosition(*currentVector, position, 0, isInWater,
					sunLight, torchLight, aoShape);
			}
		}



	};

	auto blockBakeLogicForTransparentBlocks = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b, int lod
		)
	{
		int chunkPosX = data.x * CHUNK_SIZE;
		int chunkPosZ = data.z * CHUNK_SIZE;

		glm::vec3 displace[6] = {{0,0,0.5f},{0,0,-0.5f},{0,0.5f,0},{0,-0.5f,0},{-0.5f,0,0},{0.5f,0,0},};

		std::array<glm::vec2, 6> distances = {};
		for (int i = 0; i < 6; i++)
		{
			glm::ivec3 blockPos = glm::ivec3{x, y, z};
			if (lod == 1) { blockPos *= 2; }

			auto diff = glm::vec3(playerPosition - blockPos - glm::ivec3{chunkPosX, 0, chunkPosZ}) - displace[i];
			distances[i].x = glm::dot(diff, diff);
			distances[i].y = i;
		}

		std::sort(distances.begin(), distances.end(), [](glm::vec2 &a,
			glm::vec2 &b)
		{
			return a.x > b.x;
		});

		Block *sides[26] = {};
		getNeighboursLogic(x, y, z, sides, lod);

		glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y,
				z + this->data.z * CHUNK_SIZE};

		if (lod == 1)
		{
			position = glm::vec3{x * 2 + this->data.x * CHUNK_SIZE, y * 2,
				z * 2 + this->data.z * CHUNK_SIZE};
		}

		for (int index = 0; index < 6; index++)
		{
			int i = distances[index].y;

			bool isWater = b.getType() == BlockTypes::water;

			if ((sides[i] != nullptr
				&& (!(sides[i])->isOpaque() && sides[i]->getType() != b.getType())
				) ||
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
				if (isWater && sides[i] && sides[i]->getType() == BlockTypes::water) { continue; }

				//no faces in between same types
				if (sides[i] && sides[i]->getType() == b.getType()) { continue; }


				unsigned char sunLight = 0;
				unsigned char torchLight = 0;
				calculateLightThings(sunLight, torchLight, sides[i], b, i, y);


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
								if (blockBottom.getType() == BlockTypes::water)
								{
									if (sides[checkDown]
										&& (sides[checkDown]->getType() == BlockTypes::water)
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
							if (blockTop.getType() != BlockTypes::water)
							{
								currentIndex += 22;
								topVariant = false;
							}
						}

						if (topVariant && !bottomVariant)
						{
							currentIndex = i; //normal block
						}
						else if (topVariant && bottomVariant)
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

						if (lod == 1) { currentIndex = i + lod1Parts; }

						currentVector->push_back(mergeShorts(currentIndex, getGpuIdIndexForBlock(b.getType(), i)));
					}
					else
					{
						int currentIndex = i + 22;

						if (lod == 1) { currentIndex = i + lod1Parts; }

						currentVector->push_back(mergeShorts(currentIndex, getGpuIdIndexForBlock(b.getType(), i)));
					}

					//if (!sides[2] || sides[2]->type != BlockTypes::water)
					//{
					//	currentVector->push_back(mergeShorts(i + 22, getGpuIdIndexForBlock(b.type, i)));
					//}

				}
				else
				{
					int currentIndex = i;
					if (lod == 1) { currentIndex = i + lod1Parts; }
					currentVector->push_back(mergeShorts(currentIndex, getGpuIdIndexForBlock(b.getType(), i)));
				}

				int aoShape = determineAOShape(i, sides);

				bool isInWater = (sides[i] != nullptr) && sides[i]->getType() == BlockTypes::water;

				pushFlagsLightAndPosition(*currentVector, position, isWater, isInWater,
					sunLight, torchLight, aoShape);

			}
		}
	};

	auto blockBakeLogicForGrassMesh = [&](int x, int y, int z,
		std::vector<int> *currentVector, Block &b)
	{
		Block *sides[26] = {};
		getNeighboursLogic(x, y, z, sides, 0);

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

		bool snowGrass = 0;
		bool yellowGrass = 0;
		if (b.getType() == grass)
		{
			if (sides[BOTTOM] && sides[BOTTOM]->getType() == BlockTypes::snow_dirt)
			{
				snowGrass = true;
			}

			if (sides[BOTTOM] && sides[BOTTOM]->getType() == BlockTypes::yellowGrass)
			{
				yellowGrass = true;
			}
		}

		for (int i = 6; i <= 9; i++)
		{

			if (snowGrass)
			{
				currentVector->push_back(mergeShorts(i, SNOW_GRASS_TEXTURE_INDEX * 4));
			}
			else if (yellowGrass)
			{
				currentVector->push_back(mergeShorts(i, YELLOW_GRASS_TEXTURE_INDEX * 4));
			}
			else
			{
				currentVector->push_back(mergeShorts(i, getGpuIdIndexForBlock(b.getType(), 0)));
			}

			if (dontUpdateLightSystem)
			{
				pushFlagsLightAndPosition(*currentVector, position, 0, 0, 15, 0, 0);
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

			currentVector->push_back(mergeShorts(i + 16, getGpuIdIndexForBlock(b.getType(), i)));

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

#pragma endregion

	if (updateGeometry)
	{
		setDirty(0);
		setNeighbourToLeft(left != nullptr);
		setNeighbourToRight(right != nullptr);
		setNeighbourToFront(front != nullptr);
		setNeighbourToBack(back != nullptr);

		setNeighbourToFrontLeft(frontLeft != nullptr);
		setNeighbourToFrontRight(frontRight != nullptr);
		setNeighbourToBackLeft(backLeft != nullptr);
		setNeighbourToBackRight(backRight != nullptr);

		//update lod
		{
			memset(chunkLod1, 0, sizeof(chunkLod1));

			auto check = [&](Block &b)
			{
				if (!b.air() && !b.isGrassMesh()
					&& b.getType() != BlockTypes::torch)
				{
					return true;
				}
				return false;
			};

			for (int x = 0; x < CHUNK_SIZE; x += 2)
				for (int z = 0; z < CHUNK_SIZE; z += 2)
					for (int y = 0; y < CHUNK_HEIGHT; y += 2)
					{

						//auto &b = unsafeGet(x, y, z);
						//auto &bFront = unsafeGet(x, y, z + 1);
						//auto &bRight = unsafeGet(x + 1, y, z);
						//auto &bFrontRight = unsafeGet(x + 1, y, z + 1);
						//auto &bTop = unsafeGet(x, y + 1, z);
						//auto &bTopFront = unsafeGet(x, y + 1, z + 1);
						//auto &bTopRight = unsafeGet(x + 1, y + 1, z);
						//auto &bTopFrontRight = unsafeGet(x + 1, y + 1, z + 1);

						Block b[8] = {};
						b[0] = unsafeGet(x, y + 1, z);
						b[1] = unsafeGet(x, y + 1, z + 1);
						b[2] = unsafeGet(x + 1, y + 1, z);
						b[3] = unsafeGet(x + 1, y + 1, z + 1);
						b[4] = unsafeGet(x, y, z);
						b[5] = unsafeGet(x, y, z + 1);
						b[6] = unsafeGet(x + 1, y, z);
						b[7] = unsafeGet(x + 1, y, z + 1);

						Block results[8] = {};
						int index = 0;

						for (int i = 0; i < 8; i++)
						{
							if (check(b[i]))
							{
								results[index].setType(fromAnyShapeToNormalBlockType(b[i].getType()));
								index++;
							}
						}

						Block finalResult = {};
						int counters[8] = {};

						for (int i = 0; i < index; i++)
						{
							if (results[i].getType() == 0) { break; }

							counters[i]++;

							for (int j = i + 1; j < index; j++)
							{
								if (results[j].getType() == 0) { break; }

								if (results[i].getType() == results[j].getType())
								{
									results[j].setType(0);
									counters[i]++;
								}
							}
						}

						int biggestIndex = 0;
						int biggestSize = 0;
						for (int i = 0; i < index; i++)
						{
							if (counters[i] == 0) { break; }

							if (counters[i] > biggestSize)
							{
								biggestSize = counters[i];
								biggestIndex = i;
							}
						}

						if (biggestSize)
						{
							finalResult.setType(results[biggestIndex].getType());
						}

						if (finalResult.getType())
						{
							for (int i = 0; i < index; i++)
							{
								if (results[i].getLight() > finalResult.getLight())
								{
									finalResult.setLightLevel(results[i].getLight());
								}

								if (results[i].getSkyLight() > finalResult.getSkyLight())
								{
									finalResult.setSkyLevel(results[i].getSkyLight());
								}
							}
						}

						chunkLod1[x / 2][z / 2][y / 2] = finalResult;
					}
		}

		auto bakeForBlockGeometry = [&](int x, int y, int z, Renderer::BlockGeometryIndex &geometry, Block &b)
		{

			//std::minstd_rand rng;
			//rng.seed(hash(x, y, z));
			//int rotation = getRandomNumber(rng, 0, 3);
			int rotation = b.getRotationFor365RotationTypeBlocks();


			auto type = b.getType();
			glm::ivec3 position = {x + this->data.x * CHUNK_SIZE, y, z + this->data.z * CHUNK_SIZE};
			for (int i = 0; i < geometry.componentCount; i++)
			{
				opaqueGeometry.push_back(mergeShorts(i + geometry.startIndex + geometry.componentCount * rotation,
					getGpuIdIndexForBlockWithVariation(type, 0, x, y, z)));

				if (dontUpdateLightSystem)
				{
					pushFlagsLightAndPosition(opaqueGeometry, position, 0, false,
						15, 0, 0);
				}
				else
				{
					pushFlagsLightAndPosition(opaqueGeometry, position, 0, false,
						b.getSkyLight(), b.getLight(), 0);
				}

			}

		};

		if (currentLod == 1)
		{


			for (int x = 0; x < CHUNK_SIZE/2; x ++)
				for (int z = 0; z < CHUNK_SIZE/2; z ++)
					for (int y = 0; y < CHUNK_HEIGHT / 2; y++)
					{
						auto &b = unsafeGet(x, y, z, 1);
						if (!b.air() && !b.isTransparentGeometry())
						{
							blockBakeLogicForLods(x, y, z, &opaqueGeometry, b);
						}
					}

		}
		else
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
				for (int z = 0; z < CHUNK_SIZE; z++)
					for (int y = 0; y < CHUNK_HEIGHT; y++)
					{
						auto &b = unsafeGet(x, y, z);
						if (!b.air())
						{
							auto type = b.getType();
							if (type == mug)
							{
								bakeForBlockGeometry(x, y, z, renderer.blockGeometry[ModelsManager::mugModel], b);
							}else if (b.isChairMesh())
							{
								bakeForBlockGeometry(x, y, z, renderer.blockGeometry[ModelsManager::chairModel], b);
							}else if (b.isGobletMesh())
							{
								bakeForBlockGeometry(x, y, z, renderer.blockGeometry[ModelsManager::gobletModel], b);
							}
							else if (b.getType() == wineBottle)
							{
								bakeForBlockGeometry(x, y, z, renderer.blockGeometry[ModelsManager::wineBottleModel], b);
							}
							else if (type >= skull && type <= globe)
							{
								bakeForBlockGeometry(x, y, z, renderer.blockGeometry[ModelsManager::skullModel +
									type - skull], b);
							}
							else if (b.isWallMesh())
							{
								blockBakeLogicForWalls(x, y, z, &opaqueGeometry, b);
							}
							else if (b.isStairsMesh())
							{
								blockBakeLogicForStairs(x, y, z, &opaqueGeometry, b);
							}
							else if (b.isSlabMesh())
							{
								blockBakeLogicForSlabs(x, y, z, &opaqueGeometry, b);
							}
							else
								if (b.isGrassMesh())
								{
									blockBakeLogicForGrassMesh(x, y, z, &opaqueGeometry, b);
								}
								else if (b.getType() == BlockTypes::torch)
								{
									blockBakeLogicForTorches(x, y, z, &opaqueGeometry, b);
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



		//todo see if removing this improves speed!!!!!1
		//trying to place the data in a better way for the gpu
		arangeData(opaqueGeometry);
	}

	if (updateTransparency)
	{
		setDirtyTransparency(0);

		int chunkPosX = data.x * CHUNK_SIZE;
		int chunkPosZ = data.z * CHUNK_SIZE;

		if (currentLod == 1)
		{
			for (int x = 0; x < CHUNK_SIZE/2; x++)
				for (int z = 0; z < CHUNK_SIZE/2; z++)
					for (int y = 0; y < CHUNK_HEIGHT/2; y++)
					{
						auto &b = unsafeGet(x, y, z, 1);

						//transparent geometry doesn't include air
						if (b.isTransparentGeometry())
						{
							glm::vec3 difference = playerPosition - glm::ivec3{x*2, y*2, z*2} - glm::ivec3{chunkPosX, 0, chunkPosZ};
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
				auto &b = unsafeGet(c.position.x, c.position.y, c.position.z, 1);
				blockBakeLogicForTransparentBlocks(c.position.x, c.position.y, c.position.z, &transparentGeometry,
					b, 1);
			}
		}
		else
		{
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
				blockBakeLogicForTransparentBlocks(c.position.x, c.position.y, c.position.z, &transparentGeometry, 
					b, 0);
			}
		}


	};


	return updateTransparency || updateGeometry;
}

void Chunk::sendDataToOpenGL(bool updateGeometry,
	bool updateTransparency, std::vector<TransparentCandidate> &transparentCandidates,
	std::vector<int> &opaqueGeometry, std::vector<int> &transparentGeometry,
	std::vector<glm::ivec4> &lights)
{

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

		//gpuBuffer->addChunk({data.x, data.z}, opaqueGeometry);
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

}

bool Chunk::shouldBake(Chunk *left,
	Chunk *right, Chunk *front, Chunk *back,
	Chunk *frontLeft, Chunk *frontRight, Chunk *backLeft, Chunk *backRight, int lod)
{
	if (
		isDirty()
		|| isDirtyTransparency()
		|| (!isNeighbourToLeft() && left != nullptr)
		|| (!isNeighbourToRight() && right != nullptr)
		|| (!isNeighbourToFront() && front != nullptr)
		|| (!isNeighbourToBack() && back != nullptr)

		|| (!isNeighbourToFrontLeft() && frontLeft != nullptr)
		|| (!isNeighbourToFrontRight() && frontRight != nullptr)
		|| (!isNeighbourToBackLeft() && backLeft != nullptr)
		|| (!isNeighbourToBackRight() && backRight != nullptr)
		|| currentLod != lod
		)
	{
		return true;
	}

	return false;
}

bool Chunk::forShureShouldntbake(int lod)
{
	if(isDirty() || isDirtyTransparency() ||
		!isNeighbourToLeft() ||
		!isNeighbourToRight() ||
		!isNeighbourToFront() ||
		!isNeighbourToBack() ||
		!isNeighbourToFrontLeft() ||
		!isNeighbourToFrontRight() ||
		!isNeighbourToBackLeft() ||
		!isNeighbourToBackRight()
		|| currentLod != lod
		)
		
	{
		return false;
	}

	return true;
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

void Chunk::removeBlockDataFromThisPos(Block lastBlock,
	std::uint8_t x, std::uint8_t y, std::uint8_t z)
{

	auto type = lastBlock.getType();
	if (type == BlockTypes::structureBase)
	{
		blockData.baseBlocks.erase(fromBlockPosInChunkToHashValue(x, y, z));
	}


}

std::vector<unsigned char> Chunk::getExtraDataForThisPosAndRemoveIt(Block lastBlock,
	std::uint8_t x, std::uint8_t y, std::uint8_t z)
{
	std::vector<unsigned char> rez;

	auto type = lastBlock.getType();
	if (type == BlockTypes::structureBase)
	{
		auto found = blockData.baseBlocks.find(fromBlockPosInChunkToHashValue(x, y, z));

		if (found != blockData.baseBlocks.end())
		{
			found->second.formatIntoData(rez);
			blockData.baseBlocks.erase(found);
		}

	}

	return rez;
}

void Chunk::addExtraDataToBlock(std::vector<unsigned char> &data, unsigned char x, unsigned char y, unsigned char z)
{

	//todo change later to unsafe get!
	auto b = this->safeGet(x, y, z);

	permaAssertComment(b, "error in addExtraDataToBlock");

	if (b)
	{
	
		auto type = b->getType();

		if(type == BlockTypes::structureBase)
		{
			BaseBlock baseBlock;
			size_t _ = 0;
			if(baseBlock.readFromBuffer(data.data(), data.size(), _))
			{
				auto writePlace = blockData.getOrCreateBaseBlock(x, y, z);
				*writePlace = baseBlock;
			}
		
		}


	}


}



void ChunkData::clearLightLevels()
{

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				unsafeGet(x, y, z).lightLevel = 0;
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
