#include "blocks.h"
#include <algorithm>
#include <iostream>
#include <platformTools.h>
#include <string>

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
		type == BlockTypes::jungle_planks ||
		type == BlockTypes::birch_planks;
}

//used for breaking related things
bool isAnyWoddenBlock(BlockType type)
{
	return isWoodPlank(type) ||
		type == woodLog ||
		type == bookShelf ||
		type == birch_log ||
		type == jungle_log ||
		type == palm_log ||
		type == craftingTable ||
		type == spruce_log;
}

bool isAnyWool(BlockType type)
{
	return type == whiteWool;
}

bool isAnyDirtBlock(BlockType type)
{
	return
		type == grassBlock ||
		type == dirt ||
		type == snow_dirt ||
		type == mud;
}

bool isAnyClayBlock(BlockType type)
{
	return
		type == clay;
		
}

bool isAnySandyBlock(BlockType type)
{
	return
		type == sand ||
		type == gravel;
}

bool isAnySemiHardBlock(BlockType type)
{
	return type == hardened_clay ||
		type == mud_bricks ||
		type == packed_mud ||
		type == sand_stone;
}

bool isAnyStone(BlockType type)
{
	return type == stone ||
		type == cobblestone ||
		type == bricks ||
		type == stoneBrick ||
		type == coal_ore ||
		type == gold_ore ||
		type == diamond_ore ||
		type == iron_ore ||
		type == gold_block;

}

bool isAnyPlant(BlockType type)
{
	return type == grass ||
		type == rose ||
		type == dead_bush ||
		type == cactus_bud;
}

bool isAnyGlass(BlockType type)
{
	return isStainedGlass(type) || type == glass;
}

bool isTriviallyBreakable(BlockType type)
{
	return type == torch;
}

bool isAnyUnbreakable(BlockType type)
{
	return isControlBlock(type);
}

bool isAnyLeaves(BlockType type)
{
	return type == leaves ||
		type == palm_leaves ||
		type == spruce_leaves ||
		type == spruce_leaves_red ||
		type == jungle_leaves ||
		type == birch_leaves;

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

bool isBlock(std::uint16_t type)
{
	return type > 0 && type < BlocksCount;
}

float Block::getFriction()
{
	if (type == BlockTypes::ice)
	{
		return 1.f;
	}

	return BLOCK_DEFAULT_FRICTION;
}


float getBlockBaseMineDuration(BlockType type)
{

	if (!isBlock(type)) { return 0; }
	if (type == water) { return 0; }

	if (isAnyWoddenBlock(type))
	{
		return 3.0;
	}

	if (isAnyDirtBlock(type))
	{
		return 0.75;
	}

	if (isAnyClayBlock(type))
	{
		return 0.75;
	}

	if (isAnySandyBlock(type))
	{
		return 0.75;
	}

	if (type == snow_block)
		{ return 0.75; }

	if (isAnySemiHardBlock(type) || type == testBlock)
	{
		return 3.0;
	}

	if (isAnyStone(type))
	{
		return 3.5;
	}

	if (isAnyPlant(type))
	{
		return 0.2;
	}

	if (isAnyGlass(type) || type == ice || type == glowstone)
	{
		return 0.75;
	}

	if (isAnyLeaves(type))
	{
		return 0.75 / 3.f;
	}

	if (isAnyWool(type))
	{
		return 0.75;
	}

	if (isAnyUnbreakable(type))
	{
		return 99999999999.0f;
	}

	if (isTriviallyBreakable(torch))
	{
		return 0.2;
	}

	std::cout << "Block without base mine duration assigned!: " << type << "\n";
	permaAssertComment(0, ("Block without base mine duration assigned!: " + std::to_string(type)).c_str());

	return 0.0f;
}

bool canBeMinedByHand(std::uint16_t type)
{
	if (!isBlock(type)) { return 0; }

	if (
		isAnyWoddenBlock(type) ||
		isAnyDirtBlock(type) ||
		isAnyClayBlock(type) ||
		isAnySandyBlock(type) ||
		type == snow_block || 
		isAnySemiHardBlock(type) || type == testBlock ||
		isAnyPlant(type) || isAnyGlass(type) || type == ice || type == glowstone ||
		isAnyWool(type) ||
		isTriviallyBreakable(type)
		)
	{
		return true;
	}

	return false;
}

bool canBeMinedByPickaxe(std::uint16_t type)
{
	if (!isBlock(type)) { return 0; }

	if (
		isAnySemiHardBlock(type) || type == testBlock ||
		isAnyPlant(type) || isAnyGlass(type) || type == ice || type == glowstone ||
		isTriviallyBreakable(type) || 
		isAnyStone(type)
		)
	{
		return true;
	}

	return false;
}

bool canBeMinedByShovel(std::uint16_t type)
{
	if (!isBlock(type)) { return 0; }

	if (
		isAnyDirtBlock(type) ||
		isAnyClayBlock(type) ||
		type == snow_block ||
		isAnyPlant(type) ||
		isTriviallyBreakable(type)
		)
	{
		return true;
	}

	return false;
}

bool canBeMinedByAxe(std::uint16_t type)
{
	if (!isBlock(type)) { return 0; }

	if (!isBlock(type)) { return 0; }

	if (
		isAnyWoddenBlock(type) ||
		isAnyPlant(type) ||
		isTriviallyBreakable(type)
		)
	{
		return true;
	}

	return false;
}