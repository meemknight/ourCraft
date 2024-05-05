#pragma once
#include <memory.h>
#include <vector>
#include <stdint.h>
#include <glad/glad.h>
#include <glm/vec3.hpp>

const struct Mask {
	static constexpr uint32_t IS_OPAQUE_MASK = 1 << 0;
	static constexpr uint32_t IS_TRANSPARENT_MASK = 1 << 1;
	static constexpr uint32_t IS_LIGHT_EMITTER_MASK = 1 << 2;
	static constexpr uint32_t IS_ANIMATED_MASK = 1 << 3;
	static constexpr uint32_t IS_GRASS_MESH_MASK = 1 << 4;
	static constexpr uint32_t IS_CROSS_MESH_MASK = 1 << 5;
	static constexpr uint32_t IS_COLLIDABLE_MASK = 1 << 6;
	static constexpr uint32_t IS_CONTROL_BLOCK_MASK = 1 << 7;
};


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
	glass,
	testBlock,
	torch,
	BlocksCount
};

const uint32_t blockProperties[BlocksCount] = {
	NULL, // air
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,     // grassBlock
	Mask::IS_COLLIDABLE_MASK, // dirt
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,     // stone
	Mask::IS_TRANSPARENT_MASK | Mask::IS_COLLIDABLE_MASK,// ice
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // woodLog
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // wooden_plank
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // cobblestone
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // gold_block
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // bricks
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // sand
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // sand_stone
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // snow_dirt
	Mask::IS_ANIMATED_MASK | Mask::IS_COLLIDABLE_MASK,// leaves
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // gold_ore
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // coal_ore
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // stoneBrick
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // iron_ore
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // diamond_ore
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // bookShelf
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // birch_wood
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // gravel
	Mask::IS_GRASS_MESH_MASK, // grass
	Mask::IS_GRASS_MESH_MASK, // rose
	Mask::IS_TRANSPARENT_MASK, // water
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // jungle_log
	Mask::IS_ANIMATED_MASK | Mask::IS_COLLIDABLE_MASK,// jungle_leaves
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK,  // palm_log
	Mask::IS_ANIMATED_MASK | Mask::IS_COLLIDABLE_MASK,// palm_leaves
	Mask::IS_GRASS_MESH_MASK, // cactus_bud
	Mask::IS_GRASS_MESH_MASK, // dead_bush
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK, // jungle_planks
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK, // clay
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK, // hardened_clay
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK, // mud
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK, // packed_mud
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK, // mud_bricks
	Mask::IS_CONTROL_BLOCK_MASK, // control1
	Mask::IS_CONTROL_BLOCK_MASK, // control2
	Mask::IS_CONTROL_BLOCK_MASK, // control3
	Mask::IS_CONTROL_BLOCK_MASK, // control4
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK, // snow_block
	Mask::IS_ANIMATED_MASK | Mask::IS_COLLIDABLE_MASK, // birch_leaves
	Mask::IS_OPAQUE_MASK | Mask::IS_COLLIDABLE_MASK, // spruce_log
	Mask::IS_ANIMATED_MASK | Mask::IS_COLLIDABLE_MASK, // spruce_leaves
	Mask::IS_ANIMATED_MASK | Mask::IS_COLLIDABLE_MASK, // spruce_leaves_red
	Mask::IS_LIGHT_EMITTER_MASK | Mask::IS_COLLIDABLE_MASK, // glowstone
	Mask::IS_TRANSPARENT_MASK | Mask::IS_COLLIDABLE_MASK, // glass
	NULL, // testBlock (this is not used anywhere?)
	Mask::IS_LIGHT_EMITTER_MASK // torch
};

using BlockType = uint16_t;

bool isBlockMesh(BlockType type);

bool isCrossMesh(BlockType type);

bool isControlBlock(BlockType type);

bool isOpaque(BlockType type);

bool isLightEmitter(BlockType type);

bool isTransparentGeometry(BlockType type);

bool isGrassMesh(BlockType type);

bool isCollidable(BlockType type);

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

	//rename is animated leaves (why?)
	bool isAnimatedBlock()
	{
		return (blockProperties[type] & Mask::IS_ANIMATED_MASK) != 0;
	}

	bool isTransparentGeometry()
	{
		return ::isTransparentGeometry(type);
	}

	bool isGrassMesh()
	{
		return ::isGrassMesh(type);
	}

	bool isLightEmitter()
	{
		return ::isLightEmitter(type);
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

	bool isCollidable()
	{
		return ::isCollidable(type);
	}

};

