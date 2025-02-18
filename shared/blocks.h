#pragma once
#include <memory.h>
#include <vector>
#include <stdint.h>
#include <glad/glad.h>
#include <glm/vec3.hpp>

enum BlockTypes : unsigned short
{
	air = 0,
	grassBlock,
	dirt,
	stone,
	ice,
	woodLog,
	wooden_plank,
	cobblestone,
	gold_block,
	bricks,
	sand,
	sand_stone,
	snow_dirt,
	leaves,
	gold_ore,
	coal_ore,
	stoneBrick,
	iron_ore,
	diamond_ore,
	bookShelf,
	birch_log,
	gravel,
	grass,
	rose,
	water,
	jungle_log,
	jungle_leaves,
	palm_log,
	palm_leaves,
	cactus_bud,
	dead_bush,
	jungle_planks,
	clay,
	clayBricks,
	mud,
	redClay,
	redClayBricks,
	control1,
	control2,
	control3,
	control4,
	snow_block,
	birch_leaves,
	spruce_log,
	spruce_leaves,
	spruce_leaves_red,
	glowstone,
	glass,
	testBlock,
	torch,
	craftingTable,
	coarseDirt,
	birchPlanks,

	pathBlock,
	plankedWallBlock,
	plankedWallBlock_stairs,
	plankedWallBlock_wall,
	terracotta,
	terracotta_stairs,
	terracotta_slabs,
	terracotta_wall,
	glassNotClear,
	vitral1,
	vitral2,
	glassNotClear2,
	glass2,
	purple_stained_glass,
	magenta_stained_glass,
	pink_stained_glass,
	clothBlock,
	wooden_stairs,
	wooden_slab,
	wooden_wall,
	tiledStoneBricks,

	stone_stairts,
	stone_slabs,
	stone_wall,
	cobbleStone_stairts,
	cobbleStone_slabs,
	cobbleStone_wall,
	tiledStoneBricks_stairs,
	tiledStoneBricks_slab,
	tiledStoneBricks_wall,
	bricks_stairs,
	bricks_slabs,
	bricks_wall,

	stoneBricks_stairts,
	stoneBricks_slabs,
	stoneBricks_wall,

	structureBase,

	hardSandStone,
	hardSandStone_stairs,
	hardSandStone_slabs,
	hardSandStone_wall,

	sandStone_stairts,
	sandStone_slabs,
	sandStone_wall,

	dungeonBricks,
	dungeonBricks_stairts,
	dungeonBricks_slabs,
	dungeonBricks_wall,

	volcanicHotRock,
	volcanicRock,
	volcanicRock_stairts,
	volcanicRock_slabs,
	volcanicRock_wall,

	smoothStone,
	smoothStone_stairts,
	smoothStone_slabs,
	smoothStone_wall,

	limeStone,
	smoothLimeStone,
	smoothLimeStone_stairs,
	smoothLimeStone_slabs,
	smoothLimeStone_wall,

	marbleBlock,
	marbleBlock_stairs,
	marbleBlock_slabs,
	marbleBlock_wall,
	smoothMarbleBlock,
	marbleBricks,
	marbleBricks_stairs,
	marbleBricks_slabs,
	marbleBricks_wall,
	marblePillar,

	logWall,

	yellowGrass,

	blueBricks,
	blueBricks_stairs,
	blueBricks_slabs,
	blueBricks_wall,

	oakChair,
	oakLogChair,
	mug,
	goblet,
	wineBottle,

	skull,
	skullTorch,
	book,
	candleHolder,
	pot,
	jar,
	globe,
	keg,
	workBench,
	oakTable,
	oakLogTable,
	craftingItems,
	oakBigChair,
	oakLogBigChair,
	cookingPot,
	chickenCaracas,
	chickenWingsPlate,
	fishPlate,
	ladder,
	vines,
	cloth_stairs,
	cloth_slabs,
	cloth_wall,

	birchPlanks_stairs,
	birchPlanks_slabs,
	birchPlanks_wall,
	oakLogSlab,
	smallRock,

	strippedOakLog,
	strippedBirchLog,
	strippedSpruceLog,

	sprucePlank,
	sprucePlank_stairs,
	sprucePlank_slabs,
	sprucePlank_wall,

	dungeonStone,
	dungeonStone_stairs,
	dungeonStone_slabs,
	dungeonStone_wall,
	
	dungeonCobblestone,
	dungeonCobblestone_stairs,
	dungeonCobblestone_slabs,
	dungeonCobblestone_wall,

	dungeonSmoothStone,
	dungeonSmoothStone_stairs,
	dungeonSmoothStone_slabs,
	dungeonSmoothStone_wall,

	dungeonPillar,
	dungeonSkullBlock,
	chiseledDungeonBrick,
	dungeonGlass,
		
	woddenChest,
	goblinChest,
	copperChest,
	ironChest,
	silverChest,
	goldChest,

	crate,
	smallCrate,

	BlocksCount
};

using BlockType = uint16_t;

//todo look into this
bool isBlockMesh(BlockType type);

bool canWallMountedBlocksBePlacedOn(BlockType type);

bool isStairsMesh(BlockType type);

bool isSlabMesh(BlockType type);

bool isWallMesh(BlockType type);

bool isCrossMesh(BlockType type);

bool isControlBlock(BlockType type);

bool isWallMountedBlock(BlockType type);

bool isWallMountedOrStangingBlock(BlockType type);

bool isOpaque(BlockType type);

bool isDecorativeFurniture(BlockType type);

bool isLightEmitor(BlockType type);

bool isTransparentGeometry(BlockType type);

bool isGrassMesh(BlockType type);

bool isColidable(BlockType type);

bool isWoodPlank(BlockType type);

bool isChairMesh(BlockType type);

bool isGobletMesh(BlockType type);

//used for breaking
bool isAnyWoddenBlock(BlockType type);

bool isBricksSound(BlockType type);

bool isVolcanicActiveSound(BlockType type);
bool isVolcanicInActiveSound(BlockType type);


bool isAnyWoddenLOG(BlockType type);

//used for breaking, cloth blocks
bool isAnyWool(BlockType type);

//used for breaking
bool isAnyDirtBlock(BlockType type);

//used for breaking
bool isAnyClayBlock(BlockType type);

//used for breaking
bool isAnySandyBlock(BlockType type);

//used for breaking
bool isAnySemiHardBlock(BlockType type);

//used for breaking
bool isAnyStone(BlockType type);

//used for breaking
bool isDungeonBrick(BlockType type);

//used for breaking
bool isAnyPlant(BlockType type);

//used for breaking
bool isAnyGlass(BlockType type);

//used for breaking
bool isTriviallyBreakable(BlockType type);

//used for breaking, bedrock type blocks, that can be breaked in creative!
bool isAnyUnbreakable(BlockType type);

//used for sound
bool isAnyHotSoundingBlock(BlockType type);

//used for breaking
bool isAnyLeaves(BlockType type);

bool isStainedGlass(BlockType type);

unsigned char isInteractable(BlockType type);

bool isBlock(std::uint16_t type);


bool canBeMinedByHand(std::uint16_t type);

bool canBeMinedByPickaxe(std::uint16_t type);

bool canBeMinedByShovel(std::uint16_t type);

bool canBeMinedByAxe(std::uint16_t type);


BlockType fromAnyShapeToNormalBlockType(BlockType b);

struct BlockCollider
{
	glm::vec3 size = {1,1,1};
	glm::vec3 offset = {};
};


struct Block
{
	BlockType typeAndFlags = 0;
	unsigned char lightLevel = 0; //first 4 bytes represent the sun level and bottom 4 bytes the other lights level
	unsigned char colorAndOtherFlags = 0;

	unsigned short getColor()
	{
		return colorAndOtherFlags & 0b0001'1111;
	}

	void setColor(unsigned short color)
	{
		if (color >= 32) { color = 31; }
		colorAndOtherFlags &= 0b1110'0000;
		colorAndOtherFlags |= color;
	}

	bool normalize();

	BlockType getType()
	{
		return typeAndFlags & 0b0111'1111'1111;
	}

	unsigned char getFlagsBytes()
	{
		return typeAndFlags >> 11;
	}

	//used for stairs, or furnace type blocks
	unsigned char getRotationFor365RotationTypeBlocks()
	{
		return (typeAndFlags >> 11) & 0b0001'1;
	}

	//true for standing on wall!
	unsigned char getRotatedOrStandingForWallOrStandingBlocks()
	{
		return (typeAndFlags >> 13) & 0b001;
	}

	void setRotatedOrStandingForWallOrStandingBlocks(bool isOnWall)
	{
		unsigned int is = isOnWall;
		is <<= 13;
		typeAndFlags &= 0b1101'1111'1111'1111;
		typeAndFlags |= is;
	}

	void setRotationFor365RotationTypeBlocks(int rotation)
	{
		rotation <<= 11;
		typeAndFlags &= 0b1110'0111'1111'1111;
		typeAndFlags |= rotation;
	}

	void setTopPartForSlabs(int topPart)
	{
		topPart = (bool)topPart;
		topPart <<= 11;
		typeAndFlags &= 0b1111'0111'1111'1111;
		typeAndFlags |= topPart;
	}

	bool getTopPartForSlabs()
	{
		return (typeAndFlags >> 11) & 0b0000'1;
	}

	bool isDecorativeFurniture()
	{
		auto type = getType();
		return ::isDecorativeFurniture(getType());
	}

	//used for stairs, or furnace type blocks
	bool hasRotationFor365RotationTypeBlocks()
	{
		return isStairsMesh() || isWallMesh() || isDecorativeFurniture() || isWallMountedBlock()
			|| isWallMountedOrStangingBlock();
	}

	void setType(BlockType t)
	{
		typeAndFlags = t & 0b0111'1111'1111;
	}

	bool air() { return getType() == 0; }

	bool canBePainted() { return getType() != 0; }

	bool isOpaque()
	{
		return ::isOpaque(getType());
	}

	bool isWallMountedBlock()
	{
		return ::isWallMountedBlock(getType());
	}

	bool isWallMountedOrStangingBlock()
	{
		return::isWallMountedOrStangingBlock(getType());
	}

	bool hasSecondCollider()
	{
		return isStairsMesh();
	}

	BlockCollider getCollider()
	{

		if (isChairMesh())
		{
			BlockCollider b{};
			b.size.y = 0.5;
			b.size.x = 0.8;
			b.size.z = 0.8;
			return b;
		}else
		if (isStairsMesh())
		{
			BlockCollider b{};
			b.size.y = 0.5;
			return b;
		}else
		if (isSlabMesh())
		{
			if (getTopPartForSlabs())
			{
				BlockCollider b{};
				b.size.y = 0.5;
				b.offset.y = 0.5;
				return b;
			}
			else
			{
				BlockCollider b{};
				b.size.y = 0.5;
				return b;
			}
		}else
		if (isWallMesh())
		{
			int rotation = getRotationFor365RotationTypeBlocks();

			if (rotation == 0)
			{
				BlockCollider b{};
				b.size.z = 0.5;
				b.offset.z = -0.25;
				return b;
			}
			else if (rotation == 2)
			{
				BlockCollider b{};
				b.size.z = 0.5;
				b.offset.z = 0.25;
				return b;
			}
			else if (rotation == 1)
			{
				BlockCollider b{};
				b.size.x = 0.5;
				b.offset.x = -0.25;
				return b;
			}
			else if (rotation == 3)
			{
				BlockCollider b{};
				b.size.x = 0.5;
				b.offset.x = 0.25;
				return b;
			}
		}

		return BlockCollider{};
	}

	BlockCollider getSecondCollider()
	{
		if (isStairsMesh())
		{
			int rotation = getRotationFor365RotationTypeBlocks();

			if (rotation == 0)
			{
				BlockCollider b{};
				b.size.z = 0.5;
				b.offset.z = -0.25;
				return b;
			}
			else if (rotation == 2)
			{
				BlockCollider b{};
				b.size.z = 0.5;
				b.offset.z = 0.25;
				return b;
			}
			else if (rotation == 1)
			{
				BlockCollider b{};
				b.size.x = 0.5;
				b.offset.x = -0.25;
				return b;
			}
			else if (rotation == 3)
			{
				BlockCollider b{};
				b.size.x = 0.5;
				b.offset.x = 0.25;
				return b;
			}

		}

		BlockCollider b;
		b.size = {};
		return b;
	}

	//rename is animated leaves
	bool isAnimatedBlock()
	{
		auto type = getType();
		return
			type == BlockTypes::leaves || type == BlockTypes::jungle_leaves 
			|| type == BlockTypes::palm_leaves || type == BlockTypes::birch_leaves
		|| type == BlockTypes::spruce_leaves || type == BlockTypes::spruce_leaves_red;
	}

	bool isTransparentGeometry()
	{
		return ::isTransparentGeometry(getType());
	}

	bool isStairsMesh()
	{
		return ::isStairsMesh(getType());
	}

	bool isChairMesh()
	{
		return ::isChairMesh(getType());
	}

	bool isGobletMesh()
	{
		return ::isGobletMesh(getType());
	}

	bool isWallMesh()
	{
		return ::isWallMesh(getType());
	}

	bool isSlabMesh()
	{
		return ::isSlabMesh(getType());
	}

	bool isGrassMesh()
	{
		return ::isGrassMesh(getType());
	}

	bool isLightEmitor()
	{
		return ::isLightEmitor(getType());
	}

	unsigned char getSkyLight()
	{
		return (lightLevel >> 4);
	}

	unsigned char getLight()
	{
		return lightLevel & 0b1111;
	}

	void setSkyLevel(unsigned char s)
	{
		s = s & 0b1111;
		s <<= 4;
		lightLevel &= 0b0000'1111;
		lightLevel |= s;
	}

	void setLightLevel(unsigned char l)
	{
		l = l & 0b1111;
		lightLevel &= 0b1111'0000;
		lightLevel |= l;
	}

	bool canWallMountedBlocksBePlacedOn()
	{
		return ::canWallMountedBlocksBePlacedOn(getType());
	}

	bool isBlockMesh()
	{
		return ::isBlockMesh(getType());
	}

	bool isCrossMesh()
	{
		return ::isCrossMesh(getType());
	}

	bool isColidable()
	{
		return ::isColidable(getType());
	}

	bool stopsGrassFromGrowingIfOnTop()
	{
		return isColidable() && !isAnyLeaves(getType())
			&& !isWallMesh() && !(isSlabMesh() && getTopPartForSlabs())
			&& !isDecorativeFurniture()
			;
	}

	float getFriction();

};

namespace InteractionTypes
{

	enum 
	{
		none = 0,
		craftingTable,
		structureBaseBlock,
	};

};

float getBlockBaseMineDuration(BlockType type);

