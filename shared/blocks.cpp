#include "blocks.h"
#include <algorithm>

constexpr static float BLOCK_DEFAULT_FRICTION = 8.f;


bool isBlockMesh(BlockType type)
{
	return !isCrossMesh(type);
}

bool isCrossMesh(BlockType type)
{
	return isGrassMesh(type);
}

bool isControlBlock(BlockType type)
{
	return 
		type == BlockTypes::control1 || 
		type == BlockTypes::control2 || 
		type == BlockTypes::control3 || 
		type == BlockTypes::control4;
}

bool isOpaque(BlockType type)
{
	//todo all leaves ?

	return
		type != BlockTypes::air
		&& type != BlockTypes::leaves
		&& type != BlockTypes::jungle_leaves
		&& type != BlockTypes::palm_leaves
		&& type != BlockTypes::birch_leaves
		&& type != BlockTypes::spruce_leaves
		&& type != BlockTypes::spruce_leaves_red
		&& type != BlockTypes::glowstone
		&& type != BlockTypes::torch
		&& !(isTransparentGeometry(type))
		&& !(isGrassMesh(type));
}

bool isLightEmitor(BlockType type)
{
	return type == BlockTypes::glowstone 
		|| type == BlockTypes::torch;
}

bool isTransparentGeometry(BlockType type)
{
	return type == BlockTypes::ice || type == BlockTypes::water || type == BlockTypes::glass || 
		::isStainedGlass(type);
}

bool isGrassMesh(BlockType type)
{
	return type == BlockTypes::grass
		|| type == BlockTypes::rose
		|| type == BlockTypes::cactus_bud
		|| type == BlockTypes::dead_bush
		;
}

bool isColidable(BlockType type)
{
	return
		type != BlockTypes::air &&
		type != BlockTypes::grass &&
		type != BlockTypes::rose &&
		type != BlockTypes::cactus_bud &&
		type != BlockTypes::dead_bush &&
		type != BlockTypes::torch &&
		type != BlockTypes::water;
}

bool isWoodPlank(BlockType type)
{
	return type == BlockTypes::wooden_plank || 
		type == BlockTypes::jungle_planks;
}

bool isStainedGlass(BlockType type)
{
	return type >= black_stained_glass && type <= pink_stained_glass;
}

unsigned char isInteractable(BlockType type)
{
	if (type == BlockTypes::craftingTable)
	{
		return InteractionTypes::craftingTable;
	}

	return InteractionTypes::none;
}

float Block::getFriction()
{
	if (type == BlockTypes::ice)
	{
		return 1.f;
	}

	return BLOCK_DEFAULT_FRICTION;
}
