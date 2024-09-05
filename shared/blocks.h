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
	hardened_clay,
	mud,
	packed_mud,
	mud_bricks,
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
	whiteWool,
	birch_planks,

	black_stained_glass,
	gray_stained_glass,
	light_gray_stained_glass,
	white_stained_glass,
	brown_stained_glass,
	red_stained_glass,
	orange_stained_glass,
	yellow_stained_glass,
	lime_stained_glass,
	green_stained_glass,
	cyan_stained_glass,
	light_blue_stained_glass,
	blue_stained_glass,
	purple_stained_glass,
	magenta_stained_glass,
	pink_stained_glass,


	BlocksCount
};

using BlockType = uint16_t;

bool isBlockMesh(BlockType type);

bool isCrossMesh(BlockType type);

bool isControlBlock(BlockType type);

bool isOpaque(BlockType type);

bool isLightEmitor(BlockType type);

bool isTransparentGeometry(BlockType type);

bool isGrassMesh(BlockType type);

bool isColidable(BlockType type);

bool isWoodPlank(BlockType type);

//used for breaking
bool isAnyWoddenBlock(BlockType type);

//used for breaking
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
bool isAnyPlant(BlockType type);

//used for breaking
bool isAnyGlass(BlockType type);

//used for breaking
bool isTriviallyBreakable(BlockType type);

//used for breaking, bedrock type blocks, that can be breaked in creative!
bool isAnyUnbreakable(BlockType type);

//used for breaking
bool isAnyLeaves(BlockType type);

bool isStainedGlass(BlockType type);

unsigned char isInteractable(BlockType type);

bool isBlock(std::uint16_t type);


bool canBeMinedByHand(std::uint16_t type);

bool canBeMinedByPickaxe(std::uint16_t type);

bool canBeMinedByShovel(std::uint16_t type);

bool canBeMinedByAxe(std::uint16_t type);


struct Block
{
	BlockType typeAndFlags;
	unsigned char lightLevel; //first 4 bytes represent the sun level and bottom 4 bytes the other lights level
	unsigned char notUsed;

	BlockType getType()
	{
		return typeAndFlags & 0b11'1111'1111;
	}

	void setType(BlockType t)
	{
		typeAndFlags = t & 0b11'1111'1111;
	}

	bool air() { return getType() == 0; }
	bool isOpaque()
	{
		return ::isOpaque(getType());
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
		return isColidable() && !isAnyLeaves(getType());
	}

	float getFriction();

};

namespace InteractionTypes
{

	enum 
	{
		none = 0,
		craftingTable = 1,
	};

};

float getBlockBaseMineDuration(BlockType type);

