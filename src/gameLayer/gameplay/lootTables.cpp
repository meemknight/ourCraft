#include <gameplay/lootTables.h>


//struct LootEntry
//{
//	float chance = 1; //draw one item every chance picks. Lower means more common
//	unsigned short minAmount = 1;
//	unsigned short maxAmount = 1;
//
//	Item item;
//};



LootTable scareCrawLootTable
{
	{LootEntry{1, 1, 3, ItemTypes::cloth}},
	glm::ivec2{ 3, 25 }
};


LootTable &getScareCrawLootTable()
{
	return scareCrawLootTable;
}


LootTable emptyLootTable
{
	{},
	glm::ivec2{ 0, 0 }
};


LootTable &getEmptyLootTable()
{
	return emptyLootTable;
}
