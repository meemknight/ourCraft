#include <gameplay/items.h>
#include <serializing.h>
#include <platformTools.h>
#include <iostream>
#include <climits>
#include <magic_enum.hpp>

//todo can be placed

bool Item::isBlock()
{
	return ::isBlock(type);
}

bool Item::isItemThatCanBeUsed()
{
	if (type == pigSpawnEgg || type == zombieSpawnEgg 
		|| type == catSpawnEgg || type == goblinSpawnEgg
		|| isEatable() || isPaint()
		)
	{
		return true;
	}

	return false;
}

bool Item::isConsumedAfterUse()
{
	if (type == pigSpawnEgg || type == zombieSpawnEgg || type == catSpawnEgg
		|| type == goblinSpawnEgg
		|| isEatable()
		)
	{
		return true;
	}

	return false;
}

bool Item::isEatable()
{
	return type == apple;
}


void Item::formatIntoData(std::vector<unsigned char> &data)
{
	if (type == 0 || counter == 0)
	{
		writeData(data, (unsigned short)(0));
	}
	else
	{
		writeData(data, type);
		writeData(data, counter);
		unsigned short metaDataSize = 0;
		permaAssert(metaData.size() < USHRT_MAX); 

		metaDataSize = metaData.size();
		writeData(data, metaDataSize);
		
		writeData(data, metaData.data(), sizeof(unsigned char) * metaData.size());

		//if (hasDurability())
		//{
		//	if (metaData.size() == 2)
		//	{
		//		unsigned short durability = 0;
		//		readDataUnsafe(metaData.data(), durability);
		//		writeData(data, durability);
		//	}
		//	else
		//	{
		//		permaAssert(0);
		//	}
		//}

	}

	static_assert(sizeof(unsigned short) == sizeof(type));
	static_assert(sizeof(unsigned short) == sizeof(counter));
}

int Item::readFromData(void *data, size_t size)
{
	*this = {};

	if (size < sizeof(unsigned short))
	{
		return -1;
	}

	readDataUnsafe(data, type);

	if (!type)
	{
		return sizeof(unsigned short);
	}
	else
	{
		if (size < 4)
		{
			return -1;
		}

		readDataUnsafe((unsigned char*)data + 2, counter);

		if (size < 6)
		{
			return -1;
		}

		unsigned short metaDataSize = 0;
		readDataUnsafe((unsigned char *)data + 4, metaDataSize);

		if (size - 6 < metaDataSize)
		{
			return -1; 
		}

		metaData.resize(metaDataSize);
		readDataIntoVectorUnsafeUnresized((unsigned char *)data + 7, 0, metaDataSize, metaData);

		//if (hasDurability())
		//{
		//	if (size < 5)
		//	{
		//		return -1;
		//	}
		//	unsigned short durability = 0;
		//	readDataUnsafe((unsigned char *)data + 3, durability);
		//	writeData(metaData, durability);
		//
		//	return 5;
		//}
		//else

		{
			return 6 + metaDataSize; //one short + one char + one short
		}

	}


	//unreachable
	assert(0);
}

void Item::sanitize()
{
	if (counter == 0)
	{
		type = 0;
		metaData.clear();
	}
	else
	{
		if (counter > getStackSize())
		{
			counter = getStackSize();
		}
	}

}

unsigned short Item::getStackSize()
{
	if (isTool() || isPaint() || isWeapon())
	{
		return 1;
	}
	else
	{
		return 999;
	}

}

bool Item::isTool()
{
	if (type == wooddenSword
		|| type == wooddenPickaxe
		|| type == wooddenAxe
		|| type == wooddenShovel
		)
	{
		return 1;
	}
	else
	{
		return false;
	}
}

bool Item::isPaint()
{
	return type >= soap && type <= pinkPaint;
}

bool Item::isAxe()
{
	return type == wooddenAxe;
}

bool Item::isPickaxe()
{
	return type == wooddenPickaxe;
}

bool Item::isShovel()
{
	return type == wooddenShovel;
}

bool Item::canAttack()
{
	if (
		type == trainingScythe ||
		type == trainingSword ||
		type == trainingWarHammer ||
		type == trainingFlail ||
		type == trainingSpear ||
		type == trainingKnife ||
		type == trainingBattleAxe)
	{
		return true;
	}

	return 0;
}

bool Item::isWeapon()
{
	return 
		isBattleAxe() ||
		isSword() ||
		isHammer() ||
		isDagger() ||
		isScythe() ||
		isFlail() ||
		isSpear();
}

bool Item::isBattleAxe()
{
	return type == trainingBattleAxe;
}

bool Item::isSword()
{
	return type == trainingSword;
}

bool Item::isHammer()
{
	return type == trainingWarHammer;
}

bool Item::isDagger()
{
	return type == trainingKnife;
}

bool Item::isScythe()
{
	return type == trainingScythe;
}

bool Item::isFlail()
{
	return type == trainingFlail;
}

bool Item::isSpear()
{
	return type == trainingSpear;
}

std::string Item::formatMetaDataToString()
{

	std::string rez = getItemName();

	if (counter > 1)
	{
		rez += " x";
		rez += std::to_string(counter);
	}

	if (metaData.size())
	{
		rez += "\nYes";
	}

	return rez;
}

WeaponStats Item::getWeaponStats()
{

	WeaponStats stats;

	switch (type)
	{
		 case trainingScythe:
		 {
			 stats.damage = 20;
		 }
		 break;

		 case trainingKnife:
		 {
			 stats.damage = 20;
		 }
		 break;

		 case trainingSword:
		 {
			 stats.damage = 200;
			 stats.range = 10;
		 }
		 break;

		 case trainingWarHammer:
		 {
			 stats.damage = 20;
		 }
		 break;

		 case trainingFlail:
		 {
			 stats.damage = 20;
		 }
		 break;

		 case trainingSpear:
		 {
			 stats.damage = 20;
		 }
		 break;

		 case trainingBattleAxe:
		 {
			 stats.damage = 20;
		 }
		 break;

		 stats.normalize();

		 break;
	}

	return stats;
}


Item *PlayerInventory::getItemFromIndex(int index)
{
	if (index < 0) { return 0; }

	if (index < PlayerInventory::INVENTORY_CAPACITY)
	{
		return &items[index];
	}
	else if (index == PlayerInventory::INVENTORY_CAPACITY)
	{
		return &heldInMouse;
	}
	else if (index == PlayerInventory::ARMOUR_START_INDEX)
	{
		return &headArmour;
	}
	else if (index == PlayerInventory::ARMOUR_START_INDEX + 1)
	{
		return &chestArmour;
	}
	else if (index == PlayerInventory::ARMOUR_START_INDEX + 2)
	{
		return &bootsArmour;
	}

	return nullptr;
}

void PlayerInventory::formatIntoData(std::vector<unsigned char> &data)
{

	data.reserve(INVENTORY_CAPACITY * sizeof(Item)); //rough estimate

	data.push_back(revisionNumber);

	for (int i = 0; i < INVENTORY_CAPACITY; i++)
	{
		items[i].formatIntoData(data);
	}

	heldInMouse.formatIntoData(data);


}

bool PlayerInventory::readFromData(void *data, size_t size)
{
	*this = {};

	size_t currentAdvance = 0;

	auto readOne = [&](Item &item)
	{
		int rez = item.readFromData((void *)((unsigned char *)data + currentAdvance), size - currentAdvance);

		if (rez > 0)
		{
			currentAdvance += rez;
		}
		else
		{
			return false;
		}

		return true;
	};
	
	if (size < 1) { return 0; }

	//we first read the revision number
	revisionNumber = ((unsigned char *)data)[0];
	currentAdvance++;

	for (int i = 0; i < INVENTORY_CAPACITY; i++)
	{
		if (!readOne(items[i])) { return 0; }
	}

	if (!readOne(heldInMouse)) { return 0; }

	
	return true;
}

void PlayerInventory::sanitize()
{

	for (int i = 0; i < INVENTORY_CAPACITY; i++)
	{
		items[i].sanitize();
	}

	heldInMouse.sanitize();

}

int PlayerInventory::tryPickupItem(const Item &item)
{
	int currentCounter = 0;
	int itemCounter = item.counter;

	for (int i = 0; i < INVENTORY_CAPACITY; i++)
	{
		if (!items[i].type)
		{
			items[i] = item;
			return itemCounter + currentCounter;
		}
		else if (items[i].type == item.type)
		{

			if (items[i].counter < items[i].getStackSize())
			{

				if (items[i].counter + itemCounter <= items[i].getStackSize())
				{
					items[i].counter += itemCounter;
					return itemCounter + currentCounter;
				}
				else
				{
					int taken = items[i].getStackSize() - items[i].counter;
					currentCounter += taken;
					items[i].counter = items[i].getStackSize();
					itemCounter -= taken;
					//take as much as possible and continue
				}

			}

		}
	}

	return 0;
}



//for textures
const char *itemsNamesTextures[] = 
{
	"stick.png",
	"coal.png",
	"wooden_sword.png",
	"wooden_pickaxe.png",
	"wooden_axe.png",
	"wooden_shovel.png",

	"trainingScythe.png",
	"trainingSword.png",
	"trainingWarHammer.png",
	"trainingFlail.png",
	"trainingSpear.png",
	"trainingKnife.png",
	"trainingBattleAxe.png",

	"", //eggs
	"",
	"",
	"",

	"food/apple.png",
	"food/appleJuice.png",

	"armour/leatherBoots.png",
	"armour/leatherTunic.png",
	"armour/leatherCap.png",

	"soap.png",
	"whitePaint.png",
	"lightGrayPaint.png",
	"darkGrayPaint.png",
	"blackPaint.png",
	"brownPaint.png",
	"redPaint.png",
	"orangePaint.png",
	"yellowPaint.png",
	"limePaint.png",
	"greenPaint.png",
	"turqoisePaint.png",
	"cyanPaint.png",
	"bluePaint.png",
	"purplePaint.png",
	"magentaPaint.png",
	"pinkPaint.png",
};

const char *itemsNames[] =
{
	"stick",
	"coal",
	"wooden_sword",
	"wooden_pickaxe",
	"wooden_axe",
	"wooden_shovel",

	"trainingScythe",
	"trainingSword",
	"trainingWarHammer",
	"trainingFlail",
	"trainingSpear",
	"trainingKnife",
	"trainingBattleAxe",

	"zombie spawn egg", //eggs
	"pig spawn egg",
	"cat spawn egg",
	"goblin spawn egg",

	"apple",
	"apple juice",

	"leather boots",
	"leather tunic",
	"leather cap",

	"soap",
	"white paint",
	"lightGray paint",
	"darkGray paint",
	"black paint",
	"brown paint",
	"red paint",
	"orange paint",
	"yellow paint",
	"lime paint",
	"green paint",
	"turqoise paint",
	"cyan paint",
	"blue paint",
	"purple paint",
	"magenta paint",
	"pink paint",

};

const char *getItemTextureName(int itemId)
{
	static_assert(sizeof(itemsNamesTextures) / sizeof(itemsNamesTextures[0]) == lastItem - ItemsStartPoint);
	static_assert(sizeof(itemsNames) / sizeof(itemsNames[0]) == lastItem - ItemsStartPoint);

	return itemsNamesTextures[itemId-ItemsStartPoint];
}


//doesn't compare size
bool areItemsTheSame(Item &a, Item &b)
{
	if (a.type != b.type)
	{
		return 0;
	}

	if (a.metaData != b.metaData)
	{
		return 0;
	}

	return 1;
}


bool isItem(unsigned short type)
{
	return type >= ItemsStartPoint && type < ItemTypes::lastItem;
}


bool canItemBeMovedToAndMoveIt(Item &from, Item &to)
{

	if (to.type == 0)
	{
		to = std::move(from);
		from = Item();
		return true;
	}
	else if(areItemsTheSame(from, to))
	{

		int totalSize = from.counter + to.counter;

		if (totalSize > from.getStackSize())
		{
			return 0;
		}
		else
		{
			to.counter += from.counter;
			from = Item();
			return 1;
		}
	}

	return 0;

}


//create item createItem
Item itemCreator(unsigned short type, unsigned short counter)
{
	if (!counter) { return {}; }

	Item ret(type);
	ret.counter = counter;

	ret.sanitize();

	return ret;
}

float computeMineDurationTime(BlockType type, Item &item)
{

	float timer = getBlockBaseMineDuration(type);

	const float PENALTY = 3;

	bool canBeMinedHand = canBeMinedByHand(type);

	
	if (item.isPickaxe())
	{
		if (canBeMinedByPickaxe(type))
		{
			//todo add speed here;
			timer /= 2;
		}
		else if (!canBeMinedHand)
		{
			timer *= PENALTY;
		}
	}
	else if (item.isShovel())
	{
		if (canBeMinedByShovel(type))
		{
			//todo add speed here;
			timer /= 2;
		}
		else if (!canBeMinedHand)
		{
			timer *= PENALTY;
		}
	}
	else if (item.isAxe())
	{
		if (canBeMinedByAxe(type))
		{
			//todo add speed here;
			timer /= 2;
		}
		else if (!canBeMinedHand)
		{
			timer *= PENALTY;
		}
	}
	else //hand or no tool
	{
		if (!canBeMinedHand)
		{
			timer *= PENALTY;
		}
	}
	
	std::cout << timer << "\n";
	return timer;
}


char *blockNames[] = {
	"Air",
	"Grass Block",
	"Dirt",
	"Stone",
	"Ice",
	"Wood Log",
	"Wooden Plank",
	"Cobblestone",
	"Gold Block",
	"Bricks",
	"Sand",
	"Sandstone",
	"Snowy Dirt",
	"Leaves",
	"Gold Ore",
	"Coal Ore",
	"Stone Brick",
	"Iron Ore",
	"Diamond Ore",
	"Bookshelf",
	"Birch Log",
	"Gravel",
	"Grass",
	"Rose",
	"Water",
	"Jungle Log",
	"Jungle Leaves",
	"Palm Log",
	"Palm Leaves",
	"Cactus Bud",
	"Dead Bush",
	"Jungle Planks",
	"Clay",
	"Clay Bricks",
	"Mud",
	"Red Clay",
	"Red Clay Bricks",
	"Control Block 1",
	"Control Block 2",
	"Control Block 3",
	"Control Block 4",
	"Snow Block",
	"Birch Leaves",
	"Spruce Log",
	"Spruce Leaves",
	"Red Spruce Leaves",
	"Glowstone",
	"Glass",
	"Test Block",
	"Torch",
	"Crafting Table",
	"Coarse Dirt",
	"Birch Planks",
	"Path Block",
	"Planked Stone Block",
	"Planked Stone Stairs",
	"Planked Stone",
	"Terracotta",
	"Terracotta Stairs",
	"Terracotta Slabs",
	"Terracotta Wall",
	"Opaque Glass",
	"Vitral 1",
	"Vitral 2",
	"Opaque Glass 2",
	"Glass Variant 2",
	"Mossy Cobblestone",
	"Magenta Stained Glass",
	"Pink Stained Glass",
	"Cloth Block",
	"Wooden Stairs",
	"Wooden Slab",
	"Wooden Wall",
	"Tiled Stone Bricks",
	"Stone Stairs",
	"Stone Slabs",
	"Stone Wall",
	"Cobblestone Stairs",
	"Cobblestone Slabs",
	"Cobblestone Wall",
	"Tiled Stone Brick Stairs",
	"Tiled Stone Brick Slabs",
	"Tiled Stone Brick Wall",
	"Brick Stairs",
	"Brick Slabs",
	"Brick Wall",
	"Stone Brick Stairs",
	"Stone Brick Slabs",
	"Stone Brick Wall",
	"Structure Base",
	"Hard Sandstone",
	"Hard Sandstone Stairs",
	"Hard Sandstone Slabs",
	"Hard Sandstone Wall",
	"Sandstone Stairs",
	"Sandstone Slabs",
	"Sandstone Wall",
	"Dungeon Bricks",
	"Dungeon Brick Stairs",
	"Dungeon Brick Slabs",
	"Dungeon Brick Wall",
	"Volcanic Hot Rock",
	"Volcanic Rock",
	"Volcanic Rock Stairs",
	"Volcanic Rock Slabs",
	"Volcanic Rock Wall",
	"Smooth Stone",
	"Smooth Stone Stairs",
	"Smooth Stone Slabs",
	"Smooth Stone Wall",
	"Limestone",
	"Smooth Limestone",
	"Smooth Limestone Stairs",
	"Smooth Limestone Slabs",
	"Smooth Limestone Wall",
	"Marble Block",
	"Marble Block Stairs",
	"Marble Block Slabs",
	"Marble Block Wall",
	"Smooth Marble Block",
	"Marble Bricks",
	"Marble Brick Stairs",
	"Marble Brick Slabs",
	"Marble Brick Wall",
	"Marble Pillar",
	"Log Wall",
	"Yellow Grass",
	"Blue Bricks",
	"Blue Brick Stairs",
	"Blue Brick Slabs",
	"Blue Brick Wall",
	"Oak Chair",
	"Oak Log Chair",
	"Mug",
	"Goblet",
	"Wine Bottle",
	"Skull",
	"Skull Torch",
	"Book",
	"Candle Holder",
	"Pot",
	"Jar",
	"Globe",
	"Keg",
	"Workbench",
	"Oak Table",
	"Oak Log Table",
	"Crafting Items",
	"Oak Big Chair",
	"Oak Log Big Chair",
	"Cooking Pot",
	"Chicken Carcass",
	"Chicken Wings Plate",
	"Fish Plate",
	"Ladder",
	"Vines",
	"Cloth Stairs",
	"Cloth Slabs",
	"Cloth Wall",
	"Birch Plank Stairs",
	"Birch Plank Slabs",
	"Birch Plank Wall",
	"Oak Log Slab",
	"Small Rock",
	"Stripped Oak Log",
	"Stripped Birch Log",
	"Stripped Spruce Log",
	"Spruce Plank",
	"Spruce Plank Stairs",
	"Spruce Plank Slabs",
	"Spruce Plank Wall",
	"Dungeon Stone",
	"Dungeon Stone Stairs",
	"Dungeon Stone Slabs",
	"Dungeon Stone Wall",
	"Dungeon Cobblestone",
	"Dungeon Cobblestone Stairs",
	"Dungeon Cobblestone Slabs",
	"Dungeon Cobblestone Wall",
	"Dungeon Smooth Stone",
	"Dungeon Smooth Stone Stairs",
	"Dungeon Smooth Stone Slabs",
	"Dungeon Smooth Stone Wall",
	"Dungeon Pillar",
	"Dungeon Skull Block",
	"Chiseled Dungeon Brick",
	"Dungeon Glass",
	"Wooden Chest",
	"Goblin Chest",
	"Copper Chest",
	"Iron Chest",
	"Silver Chest",
	"Gold Chest",
	"Crate",
	"Small Crate",
	"Lamp",
	"Wood Torch",

	"Mossy Cobblestone Stairs",
	"Mossy Cobblestone Slab",
	"Mossy Cobblestone Wall",
	"Cobweb",
	"Hay balde",
	"Training dummy",
	"Target",
};

std::string Item::getItemName()
{

	static_assert(sizeof(blockNames) / sizeof(blockNames[0]) == BlocksCount);
	if (isItem(type))
	{
		return itemsNames[type - ItemsStartPoint];
	}
	else
	{
		//return std::string(magic_enum::enum_name((BlockTypes)type));
		return blockNames[type];
	}
}

