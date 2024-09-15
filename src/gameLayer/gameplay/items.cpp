#include <gameplay/items.h>
#include <serializing.h>
#include <platformTools.h>
#include <iostream>

//todo can be placed

bool Item::isBlock()
{
	return ::isBlock(type);
}

bool Item::isItemThatCanBeUsed()
{
	if (type == pigSpawnEgg || type == zombieSpawnEgg || type == catSpawnEgg)
	{
		return true;
	}

	return false;
}

bool Item::isConsumedAfterUse()
{
	if (type == pigSpawnEgg || type == zombieSpawnEgg || type == catSpawnEgg)
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
		//todo check if item should have meta data
		if (!canHaveMetaData())
		{
			metaData.clear();
		}

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

bool Item::canHaveMetaData()
{
	if (isTool())
	{
		return 1;
	}
	else
	{
		return false;
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

std::string Item::formatMetaDataToString()
{
	if (metaData.size())
	{
		return "Yes";
	}

	return "";
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
	else if (index < PlayerInventory::CRAFTING_INDEX + 9)
	{
		return &crafting[index - PlayerInventory::CRAFTING_INDEX];
	}if (index == PlayerInventory::CRAFTING_RESULT_INDEX)
	{
		return 0;
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

	for (int i = 0; i < 4; i++)
	{
		crafting[i].formatIntoData(data);
	}

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

	for (int i = 0; i < 4; i++)
	{
		if (!readOne(crafting[i])) { return 0; }
	}

	
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

void PlayerInventory::craft4(int count)
{
	for (int i = 0; i < 4; i++)
	{
		if (crafting[i].counter < count)
		{
			crafting[i] = {};
			permaAssertComment(0, "Error in craft method in player inventory");
		}
		else
		{
			crafting[i].counter -= count;

			if (crafting[i].counter <= 0)
			{
				crafting[i] = {};
			}
		}
	}
}

void PlayerInventory::craft9(int count)
{
	for (int i = 0; i < 9; i++)
	{
		if (crafting[i].counter < count)
		{
			crafting[i] = {};
			permaAssertComment(0, "Error in craft method in player inventory");
		}
		else
		{
			crafting[i].counter -= count;

			if (crafting[i].counter <= 0)
			{
				crafting[i] = {};
			}
		}
	}
}


const char *itemsNames[] = 
{
	"stick.png",
	"coal.png",
	"wooden_sword.png",
	"wooden_pickaxe.png",
	"wooden_axe.png",
	"wooden_shovel.png",
	"", //eggs
	"",
	"",

};

const char *getItemTextureName(int itemId)
{
	static_assert(sizeof(itemsNames) / sizeof(itemsNames[0]) == lastItem - ItemsStartPoint);

	return itemsNames[itemId-ItemsStartPoint];
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