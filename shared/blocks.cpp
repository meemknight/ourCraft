#include "blocks.h"
#include <algorithm>
#include <iostream>
#include <platformTools.h>
#include <string>


int blockReorder[] = {
	// Basic Blocks
	air, grassBlock, dirt, stone, stone_stairts, stone_slabs, stone_wall, cobblestone, cobbleStone_stairts, cobbleStone_slabs, cobbleStone_wall, gravel, water,

	// Ores & Resources
	coal_ore, iron_ore, gold_ore, diamond_ore, gold_block,

	// Natural Blocks
	sand, sand_stone, sandStone_stairts, sandStone_slabs, sandStone_wall, hardSandStone, hardSandStone_stairs, hardSandStone_slabs, hardSandStone_wall,
	clay, clayBricks, redClay, redClayBricks, mud,
	snow_dirt, snow_block, ice, glowstone, pathBlock, coarseDirt,
	limeStone, smoothLimeStone, smoothLimeStone_stairs, smoothLimeStone_slabs, smoothLimeStone_wall,

	// Wood & Planks
	woodLog, jungle_log, birch_log, palm_log, spruce_log, strippedOakLog, strippedBirchLog, strippedSpruceLog,
	wooden_plank, wooden_stairs, wooden_slab, wooden_wall, jungle_planks,
	birchPlanks, birchPlanks_stairs, birchPlanks_slabs, birchPlanks_wall,
	sprucePlank, sprucePlank_stairs, sprucePlank_slabs, sprucePlank_wall,

	// Leaves & Plants
	leaves, jungle_leaves, birch_leaves, palm_leaves, spruce_leaves, spruce_leaves_red,
	grass, rose, dead_bush, cactus_bud, vines, yellowGrass, hayBalde,

	// Decorative & Special Blocks
	bricks, bricks_stairs, bricks_slabs, bricks_wall, blueBricks, blueBricks_stairs, blueBricks_slabs, blueBricks_wall,
	mossyCobblestone,mossyCobblestone_stairs,mossyCobblestone_slab,mossyCobblestone_wall,
	stoneBrick, stoneBricks_stairts, stoneBricks_slabs, stoneBricks_wall,
	tiledStoneBricks, tiledStoneBricks_stairs, tiledStoneBricks_slab, tiledStoneBricks_wall,
	clothBlock, cloth_stairs, cloth_slabs, cloth_wall,
	terracotta, terracotta_stairs, terracotta_slabs, terracotta_wall,
	volcanicHotRock, volcanicRock, volcanicRock_stairts, volcanicRock_slabs, volcanicRock_wall,
	smoothStone, smoothStone_stairts, smoothStone_slabs, smoothStone_wall,
	marbleBlock, marbleBlock_stairs, marbleBlock_slabs, marbleBlock_wall, smoothMarbleBlock,
	marbleBricks, marbleBricks_stairs, marbleBricks_slabs, marbleBricks_wall, marblePillar,

	// Walls
	plankedWallBlock, plankedWallBlock_stairs, plankedWallBlock_wall,

	// Glass Variants
	glass, glassNotClear, glassNotClear2, glass2,
	vitral1, vitral2,
	 magenta_stained_glass, pink_stained_glass,

	// Structures & Functional Blocks
	craftingTable, workBench, cookingPot, ladder, trainingDummy, target,

	// Furniture
	bookShelf,
	oakChair, oakBigChair, oakLogChair, oakLogBigChair, oakTable, oakLogTable,
	woddenChest, goblinChest, copperChest, ironChest, silverChest, goldChest,
	crate, smallCrate,

	// Decorations
	torch, torchWood, skull, skullTorch, book, candleHolder, pot, jar, globe, keg, wineBottle, goblet, mug,
	lamp,

	// Miscellaneous
	smallRock, oakLogSlab, logWall, craftingItems, chickenCaracas, chickenWingsPlate, fishPlate,
	cobweb,

	// Dungeon Stuff
	dungeonBricks, dungeonBricks_stairts, dungeonBricks_slabs, dungeonBricks_wall,
	dungeonStone, dungeonStone_stairs, dungeonStone_slabs, dungeonStone_wall,
	dungeonCobblestone, dungeonCobblestone_stairs, dungeonCobblestone_slabs, dungeonCobblestone_wall,
	dungeonSmoothStone, dungeonSmoothStone_stairs, dungeonSmoothStone_slabs, dungeonSmoothStone_wall,
	dungeonPillar, dungeonSkullBlock, chiseledDungeonBrick, dungeonGlass,

	// Creative
	testBlock, control1, control2, control3, control4, structureBase,
};



int getBlockReorder(int index)
{
	static_assert(sizeof(blockReorder) / sizeof(blockReorder[0]) == BlocksCount);

	return blockReorder[index];
}

bool isBlockMesh(BlockType type)
{
	return !isCrossMesh(type)
		&& !(type == torch)
		&& !(type == torchWood)
		&& !(type == lamp)
		&& !isStairsMesh(type)
		&& !isWallMesh(type)
		&& !isSlabMesh(type)
		&& !isDecorativeFurniture(type)
		&& !isWallMountedBlock(type)
		
		;
}

bool canWallMountedBlocksBePlacedOn(BlockType type)
{
	return isBlockMesh(type) ||
		isStairsMesh(type) ||
		isWallMesh(type);
}

bool isStairsMesh(BlockType type)
{
	return 
		type == wooden_stairs ||
		type == bricks_stairs ||
		type == stone_stairts ||
		type == birchPlanks_stairs ||
		type == cobbleStone_stairts ||
		type == stoneBricks_stairts ||
		type == hardSandStone_stairs ||
		type == sandStone_stairts ||
		type == dungeonBricks_stairts ||
		type == dungeonStone_stairs ||
		type == dungeonCobblestone_stairs ||
		type == dungeonSmoothStone_stairs ||
		type == volcanicRock_stairts ||
		type == smoothStone_stairts ||
		type == smoothLimeStone_stairs ||
		type == sprucePlank_stairs ||
		type == marbleBlock_stairs ||
		type == marbleBricks_stairs ||
		type == mossyCobblestone_stairs ||
		type == cloth_stairs ||
		type == terracotta_stairs ||
		type == plankedWallBlock_stairs ||
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
		type == birchPlanks_slabs ||
		type == sandStone_slabs ||
		type == terracotta_slabs ||
		type == dungeonBricks_slabs ||
		type == dungeonStone_slabs ||
		type == dungeonCobblestone_slabs ||
		type == dungeonSmoothStone_slabs ||
		type == volcanicRock_slabs ||
		type == smoothStone_slabs ||
		type == smoothLimeStone_slabs ||
		type == cloth_slabs ||
		type == marbleBlock_slabs ||
		type == sprucePlank_slabs ||
		type == marbleBricks_slabs ||
		type == blueBricks_slabs ||
		type == oakLogSlab ||
		type == mossyCobblestone_slab ||
		type == tiledStoneBricks_slab;
}

bool isWallMesh(BlockType type)
{
	return type == wooden_wall ||
		type == bricks_wall ||
		type == stone_wall ||
		type == cobbleStone_wall ||
		type == birchPlanks_wall ||
		type == stoneBricks_wall ||
		type == terracotta_wall ||
		type == hardSandStone_wall ||
		type == sandStone_wall ||
		type == dungeonBricks_wall ||
		type == dungeonStone_wall ||
		type == dungeonCobblestone_wall ||
		type == dungeonSmoothStone_wall ||
		type == volcanicRock_wall ||
		type == smoothStone_wall ||
		type == sprucePlank_wall ||
		type == plankedWallBlock_wall ||
		type == smoothLimeStone_wall ||
		type == marbleBlock_wall ||
		type == cloth_wall ||
		type == marbleBricks_wall ||
		type == logWall ||
		type == blueBricks_wall ||
		type == mossyCobblestone_wall ||
		type == tiledStoneBricks_wall;

}

bool isCrossMesh(BlockType type)
{
	return isGrassMesh(type) || type == cobweb;
}

bool isControlBlock(BlockType type)
{
	return 
		type == BlockTypes::control1 || 
		type == BlockTypes::control2 || 
		type == BlockTypes::control3 || 
		type == BlockTypes::control4;
}

bool isWallMountedBlock(BlockType type)
{
	return type == BlockTypes::ladder || type == BlockTypes::vines;
}

bool isWallMountedOrStangingBlock(BlockType type)
{
	return type == BlockTypes::torch
		|| type == BlockTypes::torchWood
		|| type == BlockTypes::lamp
		;

}

bool isOpaque(BlockType type)
{
	return
		type != BlockTypes::air
		&& type != BlockTypes::torch
		&& type != BlockTypes::torchWood
		&& type != BlockTypes::lamp
		&& type != BlockTypes::cobweb
		&& !isWallMountedBlock(type)
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
		type == keg ||
		type == cookingPot ||
		type == chickenCaracas ||
		type == chickenWingsPlate ||
		type == fishPlate ||
		type == workBench ||
		type == oakTable ||
		type == craftingItems ||
		type == oakLogTable ||
		type == oakBigChair ||
		type == oakLogBigChair ||
		type == smallRock ||
		type == woddenChest ||
		type == goblinChest ||
		type == copperChest ||
		type == ironChest ||
		type == silverChest ||
		type == goldChest ||
		type == smallCrate ||
		type == lamp ||
		type == torch ||
		type == torchWood ||
		type == trainingDummy ||
		type == target ||
		type == globe
		
		;
}

bool isLightEmitor(BlockType type)
{
	return type == BlockTypes::glowstone
		|| type == BlockTypes::torch
		|| type == BlockTypes::torchWood
		|| type == BlockTypes::lamp
		|| type == BlockTypes::candleHolder
		|| type == BlockTypes::skullTorch;
}

bool isTransparentGeometry(BlockType type)
{
	return type == BlockTypes::ice || type == BlockTypes::water ||
		::isAnyGlass(type);
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
		type != BlockTypes::lamp &&
		type != BlockTypes::torchWood &&
		type != BlockTypes::water &&
		type != BlockTypes::jar &&
		type != BlockTypes::globe &&
		type != BlockTypes::skull &&
		type != BlockTypes::skullTorch &&
		type != BlockTypes::book &&
		type != BlockTypes::candleHolder &&
		type != BlockTypes::goblet &&
		type != BlockTypes::wineBottle &&
		type != BlockTypes::chickenWingsPlate &&
		type != BlockTypes::chickenCaracas &&
		type != BlockTypes::fishPlate &&
		type != BlockTypes::ladder &&
		type != BlockTypes::vines &&
		type != BlockTypes::smallRock &&
		type != BlockTypes::cobweb &&
		type != BlockTypes::trainingDummy &&
		type != BlockTypes::craftingItems &&
		type != BlockTypes::target &&
		type != BlockTypes::mug;
}

bool isWoodPlank(BlockType type)
{
	return type == BlockTypes::wooden_plank ||
		type == BlockTypes::jungle_planks ||
		type == BlockTypes::sprucePlank ||
		type == BlockTypes::birchPlanks;
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
		type == sprucePlank_stairs ||
		type == sprucePlank_slabs ||
		type == sprucePlank_wall ||
		type == bookShelf ||
		type == birch_log ||
		type == jungle_log ||
		type == palm_log ||
		type == craftingTable ||
		type == wooden_stairs ||
		type == wooden_wall ||
		type == oakLogSlab ||
		type == ladder ||
		type == oakTable ||
		type == oakLogTable ||
		type == wooden_slab ||
		type == birchPlanks ||
		type == birchPlanks_slabs ||
		type == birchPlanks_stairs ||
		type == birchPlanks_wall ||
		type == spruce_log ||
		type == oakChair ||
		type == crate || 
		type == oakLogChair ||
		type == strippedOakLog ||
		type == strippedBirchLog ||
		type == strippedSpruceLog ||
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
		type == strippedOakLog ||
		type == strippedBirchLog  ||
		type == strippedSpruceLog ||
		type == spruce_log;
}

bool isAnyWool(BlockType type)
{
	return type == clothBlock || type == cloth_stairs || type == cloth_slabs || type == cloth_wall;
}

bool isAnyDirtBlock(BlockType type)
{
	return
		type == grassBlock ||
		type == dirt ||
		type == pathBlock || 
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
		type == sandStone_stairts||
		type == terracotta ||
		type == terracotta_wall ||
		type == terracotta_slabs ||
		type == terracotta_stairs
		;


}

//used for breaking
bool isAnyStone(BlockType type)
{
	return type == stone ||
		type == cobblestone ||
		type == bricks ||
		type == blueBricks ||
		type == blueBricks_stairs ||
		type == plankedWallBlock_stairs ||
		type == plankedWallBlock_wall ||
		type == blueBricks_wall ||
		type == blueBricks_slabs ||
		type == stoneBrick ||
		type == coal_ore ||
		type == gold_ore ||
		type == diamond_ore ||
		type == iron_ore ||
		type == plankedWallBlock ||
		type == tiledStoneBricks ||
		type == gold_block ||
		type == limeStone ||
		type == smoothLimeStone ||
		type == smoothLimeStone_wall ||
		type == smoothLimeStone_stairs ||
		type == smoothLimeStone_slabs ||
		type == mossyCobblestone ||
		type == mossyCobblestone_stairs ||
		type == mossyCobblestone_slab ||
		type == mossyCobblestone_wall ||

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



		type == smoothStone ||
		type == smoothStone_stairts ||
		type == smoothStone_slabs ||
		type == smoothStone_wall ||

		type == stoneBricks_stairts ||
		type == stoneBricks_slabs ||
		type == stoneBricks_wall;
}

bool isDungeonBrick(BlockType type)
{
	
	return
		type == dungeonBricks ||
		type == dungeonBricks_slabs ||
		type == dungeonBricks_stairts ||
		type == dungeonBricks_wall ||
		type == dungeonStone ||
		type == dungeonStone_stairs ||
		type == dungeonStone_slabs ||
		type == dungeonStone_wall ||
		type == dungeonCobblestone ||
		type == dungeonCobblestone_stairs ||
		type == dungeonCobblestone_slabs ||
		type == dungeonCobblestone_wall ||
		type == dungeonSmoothStone ||
		type == dungeonSmoothStone_stairs ||
		type == dungeonSmoothStone_slabs ||
		type == dungeonSmoothStone_wall ||
		type == dungeonPillar ||
		type == chiseledDungeonBrick ||
		type == dungeonGlass ||
		type == dungeonSkullBlock;
}

bool isAnyPlant(BlockType type)
{
	return type == grass ||
		type == rose ||
		type == dead_bush ||
		type == vines ||
		type == cactus_bud;
}

bool isAnyGlass(BlockType type)
{
	return isStainedGlass(type) || type == glass || type == glass2 ||
		type == glassNotClear || type == vitral1 || type == vitral2 || type == glassNotClear2 
		 || type == dungeonGlass;
}

bool isTriviallyBreakable(BlockType type)
{
	return type == torch || type == torchWood || type == lamp;
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
	return type >= magenta_stained_glass && type <= pink_stained_glass;
}

unsigned char isInteractable(BlockType type)
{
	if (type == BlockTypes::craftingTable || type == BlockTypes::workBench || type == BlockTypes::craftingItems)
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

bool Block::normalize()
{

	auto type = getType();

	//remove wierd blocks!
	if (type >= BlockTypes::BlocksCount)
	{
		typeAndFlags = {};
		return true;
	}

	//we have flags...
	if (getFlagsBytes())
	{
		//todo
	}


	return false;
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
		return 2.5;
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
		return 2.5;
	}

	if (isDungeonBrick(type))
	{
		return 9999999.0;
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
		return 9999999.0f;
	}

	if (isTriviallyBreakable(type))
	{
		return 0.2;
	}

	if (type == cobweb)
	{
		return 1;
	}

	if (type == structureBase)
	{
		return 1;
	}

	if (isDecorativeFurniture(type))
	{
		return 0.2;
	}

	if (type == hayBalde)
	{
		return 0.3;
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
	case oakLogSlab:
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

	case cloth_slabs:
	case cloth_wall:
	case cloth_stairs: { return clothBlock; };

	case plankedWallBlock_wall:
	case plankedWallBlock_stairs: { return plankedWallBlock; };

	case terracotta_slabs:
	case terracotta_wall:
	case terracotta_stairs: { return terracotta; };

	case birchPlanks_slabs:
	case birchPlanks_wall:
	case birchPlanks_stairs: { return birchPlanks; };

	case sprucePlank_stairs:
	case sprucePlank_slabs:
	case sprucePlank_wall: { return sprucePlank; };

	case dungeonStone_stairs : 
	case dungeonStone_slabs : 
	case dungeonStone_wall: { return dungeonStone; }

	case dungeonCobblestone_stairs:
	case dungeonCobblestone_slabs:
	case dungeonCobblestone_wall: { return dungeonCobblestone; }

	case dungeonSmoothStone_stairs:
	case dungeonSmoothStone_slabs:
	case dungeonSmoothStone_wall: { return dungeonSmoothStone; }

	case mossyCobblestone_stairs:
	case mossyCobblestone_slab:
	case mossyCobblestone_wall: { return mossyCobblestone; }

	};

	if (INTERNAL_BUILD == 1)
	{
		permaAssertComment(!isStairsMesh(b), "error you forgot to add a stair here!");
		permaAssertComment(!isWallMesh(b), "error you forgot to add a wall here!");
		permaAssertComment(!isSlabMesh(b), "error you forgot to add a slab here!");
	}

	return b;
}
