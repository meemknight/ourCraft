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

bool isStairsMesh(BlockType type)
{
	return 
		type == wooden_stairs ||
		type == bricks_stairts ||
		type == stone_stairts ||
		type == cobbleStone_stairts ||
		type == stoneBricks_stairts ||
		type == hardSandStone_stairs ||
		type == sandStone_stairts ||
		type == dungeonBricks_stairts ||
		type == volcanicRock_stairts ||
		type == smoothStone_stairts ||
		type == smoothLimeStone_stairs ||
		type == marbleBlock_stairs ||
		type == marbleBricks_stairs ||
		type == tiledStoneBricks_stairs
		;
}

bool isSlabMesh(BlockType type)
{
	return type == wooden_slab ||
		type == bricks_slabs ||
		type == stone_slabs ||
		type == cobbleStone_slabs ||
		type == stoneBricks_slabs ||
		type == hardSandStone_slabs ||
		type == sandStone_slabs ||
		type == dungeonBricks_slabs ||
		type == volcanicRock_slabs ||
		type == smoothStone_slabs ||
		type == smoothLimeStone_slabs ||
		type == marbleBlock_slabs ||
		type == marbleBricks_slabs ||
		type == tiledStoneBricks_slab;
}

bool isWallMesh(BlockType type)
{
	return type == wooden_wall ||
		type == bricks_wall ||
		type == stone_wall ||
		type == cobbleStone_wall ||
		type == stoneBricks_wall ||
		type == hardSandStone_wall ||
		type == sandStone_wall ||
		type == dungeonBricks_wall ||
		type == volcanicRock_wall ||
		type == smoothStone_wall ||
		type == smoothLimeStone_wall ||
		type == marbleBlock_wall ||
		type == marbleBricks_wall ||
		type == tiledStoneBricks_wall;

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
		&& type != BlockTypes::torch
		&& !(isStairsMesh(type))
		&& !(isSlabMesh(type))
		&& !(isWallMesh(type))
		&& !(isAnyLeaves(type))
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
		type == wooden_stairs ||
		type == wooden_wall ||
		type == wooden_slab ||
		type == spruce_log;
}

bool isBricksSound(BlockType type)
{
	return type == bricks ||
		type == tiledStoneBricks ||
		type == bricks_stairts ||
		type == bricks_slabs ||
		type == marbleBricks ||
		type == marbleBricks_stairs ||
		type == marbleBricks_slabs ||
		type == marbleBricks_wall ||
		type == bricks_wall;
}

bool isVolcanicActiveSound(BlockType type)
{
	return type == volcanicHotRock;
}

bool isVolcanicInActiveSound(BlockType type)
{
	return type == volcanicRock ||
		type == volcanicRock_stairts ||
		type == volcanicRock_slabs ||
		type == volcanicRock_wall;
}



bool isAnyWoddenLOG(BlockType type)
{
	return
		type == woodLog ||
		type == birch_log ||
		type == jungle_log ||
		type == palm_log ||
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
		type == coarseDirt ||
		type == mud;
}

bool isAnyClayBlock(BlockType type)
{
	return
		type == clay ||
		type == redClay;
		
}

bool isAnySandyBlock(BlockType type)
{
	return
		type == sand ||
		type == gravel;
}

bool isAnySemiHardBlock(BlockType type)
{
	return type == clayBricks ||
		type == redClayBricks ||
		type == sand_stone ||
		type == sandStone_wall ||
		type == sandStone_slabs ||
		type == sandStone_stairts;


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
		type == tiledStoneBricks ||
		type == gold_block ||
		type == limeStone ||
		type == smoothLimeStone ||
		type == smoothLimeStone_wall ||
		type == smoothLimeStone_stairs ||
		type == smoothLimeStone_slabs ||

		type == marbleBlock ||
		type == marbleBlock_stairs ||
		type == marbleBlock_slabs ||
		type == marbleBlock_wall ||
		type == smoothMarbleBlock ||
		type == marbleBricks ||
		type == marbleBricks_stairs ||
		type == marbleBricks_slabs ||
		type == marbleBricks_wall ||
		type == marblePillar ||

		type == stone_stairts ||
		type == stone_slabs ||
		type == stone_wall ||
		type == cobbleStone_stairts ||
		type == cobbleStone_slabs ||
		type == cobbleStone_wall ||
		type == tiledStoneBricks_stairs ||
		type == tiledStoneBricks_slab ||
		type == tiledStoneBricks_wall ||
		type == bricks_stairts ||
		type == bricks_slabs ||
		type == bricks_wall ||

		type == volcanicHotRock ||
		type == volcanicRock ||
		type == volcanicRock_slabs ||
		type == volcanicRock_stairts ||
		type == volcanicRock_wall ||

		type == hardSandStone ||
		type == hardSandStone_slabs ||
		type == hardSandStone_stairs ||
		type == hardSandStone_wall ||

		type == dungeonBricks ||
		type == dungeonBricks_slabs ||
		type == dungeonBricks_stairts  ||
		type == dungeonBricks_wall ||

		type == smoothStone ||
		type == smoothStone_stairts ||
		type == smoothStone_slabs ||
		type == smoothStone_wall ||

		type == stoneBricks_stairts ||
		type == stoneBricks_slabs ||
		type == stoneBricks_wall;
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

bool isAnyHotSoundingBlock(BlockType type)
{
	return type == volcanicHotRock;
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
	}else if (type == BlockTypes::structureBase)
	{
		return InteractionTypes::structureBaseBlock;
	}

	return InteractionTypes::none;
}

bool isBlock(std::uint16_t type) //todo == 0 ???????????????????/
{
	return type > 0 && type < BlocksCount;
}

float Block::getFriction()
{
	if (getType() == BlockTypes::ice)
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