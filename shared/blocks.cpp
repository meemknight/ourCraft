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
		type == bricks_stairs ||
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
		type == blueBricks_stairs ||
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
		type == blueBricks_slabs ||
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
		type == logWall ||
		type == blueBricks_wall ||
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
		&& !isDecorativeFurniture(type)
		//&& type != BlockTypes::glowstone
		&& !(isStairsMesh(type))
		&& !(isSlabMesh(type))
		&& !(isWallMesh(type))
		&& !(isAnyLeaves(type))
		&& !(isTransparentGeometry(type))
		&& !(isGrassMesh(type));
}

bool isDecorativeFurniture(BlockType type)
{
	return type == mug || isChairMesh(type) || isGobletMesh(type) || type == wineBottle ||
		type == skull ||
		type == skullTorch ||
		type == book ||
		type == candleHolder ||
		type == pot ||
		type == jar ||
		type == globe
		
		;
}

bool isLightEmitor(BlockType type)
{
	return type == BlockTypes::glowstone
		|| type == BlockTypes::torch
		|| type == BlockTypes::candleHolder
		|| type == BlockTypes::skullTorch;
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
		type != BlockTypes::mug &&
		type != BlockTypes::jar &&
		type != BlockTypes::globe &&
		type != BlockTypes::skull &&
		type != BlockTypes::skullTorch &&
		type != BlockTypes::book &&
		type != BlockTypes::candleHolder &&
		type != BlockTypes::goblet &&
		type != BlockTypes::wineBottle &&
		type != BlockTypes::water;
}

bool isWoodPlank(BlockType type)
{
	return type == BlockTypes::wooden_plank ||
		type == BlockTypes::jungle_planks ||
		type == BlockTypes::birch_planks;
}

bool isChairMesh(BlockType type)
{
	return type == oakChair || type == oakLogChair;
}

bool isGobletMesh(BlockType type)
{
	return type == BlockTypes::goblet;
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
		type == spruce_log ||
		type == oakChair ||
		type == oakLogChair ||
		type == logWall;
		
}

bool isBricksSound(BlockType type)
{
	return type == bricks ||
		type == tiledStoneBricks ||
		type == bricks_stairs ||
		type == bricks_slabs ||
		type == marbleBricks ||
		type == marbleBricks_stairs ||
		type == marbleBricks_slabs ||
		type == marbleBricks_wall ||
		type == blueBricks ||
		type == blueBricks_stairs ||
		type == blueBricks_wall ||
		type == blueBricks_slabs ||
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
		type == yellowGrass ||
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

//used for breaking
bool isAnyStone(BlockType type)
{
	return type == stone ||
		type == cobblestone ||
		type == bricks ||
		type == blueBricks ||
		type == blueBricks_stairs ||
		type == blueBricks_wall ||
		type == blueBricks_slabs ||
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
		type == bricks_stairs ||
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
	//todo make sure this can't get to the client or just make them breakable
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

	if (isTriviallyBreakable(type))
	{
		return 0.2;
	}

	if (type == structureBase)
	{
		return 1;
	}

	if (isDecorativeFurniture(type))
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

BlockType fromAnyShapeToNormalBlockType(BlockType b)
{

	switch (b)
	{

	case oakLogChair:
	case logWall: {return woodLog; }

	case oakChair:
	case wooden_slab:
	case wooden_wall:
	case wooden_stairs: { return wooden_plank; };

	case bricks_slabs:
	case bricks_wall:
	case bricks_stairs: { return bricks; };

	case stone_slabs:
	case stone_wall:
	case stone_stairts: { return stone; };

	case cobbleStone_slabs:
	case cobbleStone_wall:
	case cobbleStone_stairts: { return cobblestone; };

	case stoneBricks_slabs:
	case stoneBricks_wall:
	case stoneBricks_stairts: { return stoneBrick; };

	case hardSandStone_slabs:
	case hardSandStone_wall:
	case hardSandStone_stairs: { return hardSandStone; };

	case sandStone_slabs:
	case sandStone_wall:
	case sandStone_stairts: { return sand_stone; };

	case dungeonBricks_slabs:
	case dungeonBricks_wall:
	case dungeonBricks_stairts: { return dungeonBricks; };

	case volcanicRock_slabs:
	case volcanicRock_wall:
	case volcanicRock_stairts: { return volcanicRock_stairts; };

	case smoothStone_slabs:
	case smoothStone_wall:
	case smoothStone_stairts: { return smoothStone; };

	case smoothLimeStone_slabs:
	case smoothLimeStone_wall:
	case smoothLimeStone_stairs: { return limeStone; };

	case marbleBlock_slabs:
	case marbleBlock_wall:
	case marbleBlock_stairs: { return marbleBlock; };

	case marbleBricks_slabs:
	case marbleBricks_wall:
	case marbleBricks_stairs: { return marbleBlock_stairs; };

	case blueBricks_slabs:
	case blueBricks_wall:
	case blueBricks_stairs: { return blueBricks; };

	case tiledStoneBricks_slab:
	case tiledStoneBricks_wall:
	case tiledStoneBricks_stairs: { return tiledStoneBricks; };

	};

	if (INTERNAL_BUILD == 1)
	{
		permaAssertComment(!isStairsMesh(b), "error you forgot to add a stair here!");
		permaAssertComment(!isWallMesh(b), "error you forgot to add a wall here!");
		permaAssertComment(!isSlabMesh(b), "error you forgot to add a slab here!");
	}

	return b;
}
