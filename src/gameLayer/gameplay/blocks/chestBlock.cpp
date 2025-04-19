#include <gameplay/blocks/chestBlock.h>




size_t ChestBlock::formatIntoData(std::vector<unsigned char> &appendTo)
{

	appendTo.reserve(appendTo.size() + CHEST_CAPACITY * sizeof(Item)); //rough estimate
	size_t size = 0;

	for (int i = 0; i < CHEST_CAPACITY; i++)
	{
		size += items[i].formatIntoData(appendTo);
	}

	return size;
}

bool ChestBlock::readFromBuffer(unsigned char *data, size_t size, size_t &outReadSize)
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

	for (int i = 0; i < CHEST_CAPACITY; i++)
	{
		if (!readOne(items[i])) { return 0; }
	}

	outReadSize = currentAdvance;
	normalize();

	return true;
}

bool ChestBlock::isDataValid()
{
	return true;
}

void ChestBlock::normalize()
{
	for (int i = 0; i < CHEST_CAPACITY; i++)
	{
		items[i].sanitize();
	}
}
