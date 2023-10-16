#pragma once
#include <memory.h>
#include <vector>
#include <stdint.h>
#include <glad/glad.h>
#include <glm/vec3.hpp>

enum BlockTypes
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
	coar_ore,
	stoneBrick,
	iron_ore,
	diamond_ore,
	bookShelf,
	birch_wood,
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

struct Block
{
	BlockType type;
	unsigned char lightLevel; //first 4 bytes represent the sun level and bottom 4 bytes the other lights level
	unsigned char notUsed;

	bool air() { return type == 0; }
	bool isOpaque()
	{
		return ::isOpaque(type);
	}

	//rename is animated leaves
	bool isAnimatedBlock()
	{
		return
			type == BlockTypes::leaves || type == BlockTypes::jungle_leaves 
			|| type == BlockTypes::palm_leaves || type == BlockTypes::birch_leaves
		|| type == BlockTypes::spruce_leaves || type == BlockTypes::spruce_leaves_red;
	}

	bool isTransparentGeometry()
	{
		return ::isTransparentGeometry(type);
	}

	bool isGrassMesh()
	{
		return ::isGrassMesh(type);
	}

	bool isLightEmitor()
	{
		return ::isLightEmitor(type);
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
		return ::isBlockMesh(type);
	}

	bool isCrossMesh()
	{
		return ::isCrossMesh(type);
	}

};

