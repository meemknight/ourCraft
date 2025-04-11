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
	{LootEntry{1, 2, 5, ItemTypes::wheat}},
	glm::ivec2{ 3, 25 },

	0.5,
	{LootEntry{1, 1, 3, ItemTypes::cloth}, LootEntry{10, 1, 1, ItemTypes::apple}},

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
