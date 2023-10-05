#include "blocks.h"
#include <algorithm>

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
	return
		type != BlockTypes::air
		&& type != BlockTypes::leaves
		&& type != BlockTypes::jungle_leaves
		&& type != BlockTypes::palm_leaves
		&& !(isTransparentGeometry(type))
		&& !(isGrassMesh(type));
}

bool isTransparentGeometry(BlockType type)
{
	return type == BlockTypes::ice || type == BlockTypes::water;
}

bool isGrassMesh(BlockType type)
{
	return type == BlockTypes::grass
		|| type == BlockTypes::rose
		|| type == BlockTypes::cactus_bud
		|| type == BlockTypes::dead_bush
		;
}
