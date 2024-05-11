#include <gameplay/items.h>
#include <serializing.h>

bool Item::isBlock()
{
	return type > 0 && type < BlocksCount;
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

		//todo more stuff here
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

		return 3; //one short + one char
	}

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


		if (counter > getStackSize())
		{
			counter = getStackSize();
		}

	}

}

unsigned char Item::getStackSize()
{
	return 64;
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
		int rez = item.readFromData((void *)((unsigned char*)data + currentAdvance), size - currentAdvance);

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
