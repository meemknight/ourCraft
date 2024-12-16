#include <gameplay/items.h>
#include <serializing.h>
#include <platformTools.h>
#include <iostream>
#include <magic_enum.hpp>

//todo can be placed

bool Item::isBlock()
{
	return ::isBlock(type);
}

bool Item::isItemThatCanBeUsed()
{
	if (type == pigSpawnEgg || type == zombieSpawnEgg 
		|| type == catSpawnEgg || type == goblinSpawnEgg)
	{
		return true;
	}

	return false;
}

bool Item::isConsumedAfterUse()
{
	if (type == pigSpawnEgg || type == zombieSpawnEgg || type == catSpawnEgg
		|| type == goblinSpawnEgg)
	{
		return true;
	}

	return false;
}


void Item::formatIntoData(std::vector<unsigned char> &data)
{
	if (type == 0 || counter == 0)
	{
		writeData(data, unsigned short(0));
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
	static_assert(sizeof(unsigned char) == sizeof(counter));
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
		if (size < 3)
		{
			return -1;
		}

		readDataUnsafe((unsigned char*)data + 2, counter);

		if (size < 5)
		{
			return -1;
		}

		unsigned short metaDataSize = 0;
		readDataUnsafe((unsigned char *)data + 3, metaDataSize);

		if (size - 5 < metaDataSize)
		{
			return -1; 
		}

		metaData.resize(metaDataSize);
		readDataIntoVectorUnsafeUnresized((unsigned char *)data + 5, 0, metaDataSize, metaData);

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
			return 5 + metaDataSize; //one short + one char + one short
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

unsigned char Item::getStackSize()
{
	if (isTool())
	{
		return 1;
	}
	else
	{
		return 64;
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
		 case trainingSword:
		 case trainingWarHammer:
		 case trainingFlail:
		 case trainingSpear:
		 case trainingKnife:
		 case trainingBattleAxe:

		 stats.damage = 2;

		 break;
	}

	return stats;
}


Item *PlayerInventory::getItemFromIndex(int index)
{
	if (index < PlayerInventory::INVENTORY_CAPACITY)
	{
		return &items[index];
	}
	else if (index == PlayerInventory::INVENTORY_CAPACITY)
	{
		return &heldInMouse;
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
Item itemCreator(unsigned short type, unsigned char counter)
{
	if (!counter) { return {}; }

	Item ret(type);
	ret.counter = counter;

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



std::string Item::getItemName()
{
	if (isItem(type))
	{
		return itemsNames[type - ItemsStartPoint];
	}
	else
	{
		return std::string(magic_enum::enum_name((BlockTypes)type));
	}
}

