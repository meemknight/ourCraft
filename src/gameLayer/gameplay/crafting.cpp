#include <gameplay/crafting.h>




//static CraftingRecepie recepies[] =
//{
//	recepie(Item(ItemTypes::stick, 4), 
//		{Item(),Item(),Item(),
//		Item(BlockTypes::wooden_plank), Item(), Item(),
//		Item(BlockTypes::wooden_plank), Item(), Item()}, true),
//
//	recepie(Item(BlockTypes::wooden_plank, 4),
//		{Item(),Item(),Item(),
//		Item(), Item(), Item(),
//		Item(BlockTypes::woodLog), Item(), Item()}),
//
//	recepie(Item(BlockTypes::wooden_plank, 4),
//		{Item(),Item(),Item(),
//		Item(), Item(), Item(),
//		Item(BlockTypes::spruce_log), Item(), Item()}),
//
//	recepie(Item(BlockTypes::birch_planks, 4),
//		{Item(),Item(),Item(),
//		Item(), Item(), Item(),
//		Item(BlockTypes::birch_log), Item(), Item()}),
//
//	recepie(Item(BlockTypes::jungle_planks, 4),
//		{Item(),Item(),Item(),
//		Item(), Item(), Item(),
//		Item(BlockTypes::jungle_log), Item(), Item()}),
//
//	recepie(Item(BlockTypes::craftingTable, 1),
//		{Item(),Item(),Item(),
//		Item(BlockTypes::wooden_plank), Item(BlockTypes::wooden_plank), Item(),
//		Item(BlockTypes::wooden_plank), Item(BlockTypes::wooden_plank), Item()}, true),
//
//	recepie(Item(BlockTypes::sand_stone, 1),
//		{Item(),Item(),Item(),
//		Item(BlockTypes::sand), Item(BlockTypes::sand), Item(),
//		Item(BlockTypes::sand), Item(BlockTypes::sand), Item()}),
//
//	recepie(Item(ItemTypes::wooddenSword, 1),
//		{Item(BlockTypes::wooden_plank),Item(),Item(),
//		Item(BlockTypes::wooden_plank), Item(), Item(),
//		Item(ItemTypes::stick), Item(), Item()}, true),
//
//
//	recepie(Item(ItemTypes::wooddenPickaxe, 1),
//		{Item(BlockTypes::wooden_plank),Item(BlockTypes::wooden_plank),Item(BlockTypes::wooden_plank),
//		Item(), Item(ItemTypes::stick), Item(),
//		Item(), Item(ItemTypes::stick), Item()}, true),
//
//	recepie(Item(ItemTypes::wooddenAxe, 1),
//		{Item(BlockTypes::wooden_plank),Item(BlockTypes::wooden_plank),Item(),
//		Item(BlockTypes::wooden_plank), Item(ItemTypes::stick), Item(),
//		Item(), Item(ItemTypes::stick), Item()}, true),
//
//	recepie(Item(ItemTypes::wooddenAxe, 1),
//		{Item(BlockTypes::wooden_plank),Item(BlockTypes::wooden_plank), Item(),
//		Item(ItemTypes::stick), Item(BlockTypes::wooden_plank), Item(),
//		Item(ItemTypes::stick), Item(), Item()}, true),
//
//	recepie(Item(ItemTypes::wooddenShovel, 1),
//		{Item(BlockTypes::wooden_plank), Item(),Item(),
//		Item(ItemTypes::stick), Item(), Item(),
//		Item(ItemTypes::stick), Item(), Item()}, true),
//
//	recepie(Item(BlockTypes::stoneBrick, 4),
//		{Item(),Item(),Item(),
//		Item(BlockTypes::stone), Item(BlockTypes::stone), Item(),
//		Item(BlockTypes::stone), Item(BlockTypes::stone), Item()}),
//
//	recepie(Item(BlockTypes::torch, 4),
//		{Item(),Item(),Item(),
//		Item(ItemTypes::coal), Item(), Item(),
//		Item(ItemTypes::stick), Item(), Item()}),
//
//};


//CraftingRecepie recepie(Item result, std::array<Item, 9> items,
//	bool anyWoodType, bool applyItemCreator)
//{
//	CraftingRecepie ret;
//
//	if (applyItemCreator)
//	{
//		ret.result = itemCreator(result.type);
//		ret.result.counter = result.counter;
//	}
//	else
//	{
//		ret.result = result;
//	}
//	
//	for (int i = 0; i < 9; i++)
//	{
//		ret.items[i] = items[i];
//	}
//
//	ret.anyWoodType = anyWoodType;
//
//	return ret;
//}

template<long long I>
CraftingRecepie recepie(Item result, std::array<Item, I> items, bool applyItemCreator = 0)
{

	CraftingRecepie ret;
	
	if (applyItemCreator)
	{
		ret.result = itemCreator(result.type);
		ret.result.counter = result.counter;
	}
	else
	{
		ret.result = result;
	}
	
	for (int i = 0; i < sizeof(items)/sizeof(items[0]); i++)
	{
		ret.items[i] = items[i];
	}
	
	return ret;
}


static CraftingRecepie recepies[] =
{

	recepie<1>(Item(BlockTypes::wooden_plank, 4), {Item(BlockTypes::woodLog)}),
	recepie<1>(Item(BlockTypes::wooden_plank, 4), {Item(BlockTypes::spruce_log)}),
	recepie<1>(Item(BlockTypes::birchPlanks, 4), {Item(BlockTypes::birch_log)}),
	recepie<1>(Item(BlockTypes::jungle_planks, 4),{Item(BlockTypes::jungle_log)}),
	recepie<1>(Item(BlockTypes::craftingTable, 1),{Item(BlockTypes::wooden_plank, 4)}).setAnyWood(),
	recepie<1>(Item(BlockTypes::sand_stone, 1),{Item(BlockTypes::sand, 2)}),
	recepie<1>(Item(BlockTypes::hardSandStone, 1),{Item(BlockTypes::sand, 2)}),
	recepie<1>(Item(BlockTypes::stoneBrick, 1),{Item(BlockTypes::stone, 2)}),

	//coins
	recepie<1>(Item(ItemTypes::silverCoin, 1), {Item(ItemTypes::copperCoin, 100)}),
	recepie<1>(Item(ItemTypes::goldCoin, 1), {Item(ItemTypes::silverCoin, 100)}),
	recepie<1>(Item(ItemTypes::diamondCoin, 1), {Item(ItemTypes::goldCoin, 100)}),

	recepie<1>(Item(ItemTypes::goldCoin, 100), {Item(ItemTypes::diamondCoin, 1)}),
	recepie<1>(Item(ItemTypes::silverCoin, 100), {Item(ItemTypes::goldCoin, 1)}),
	recepie<1>(Item(ItemTypes::copperCoin, 100), {Item(ItemTypes::silverCoin, 1)}),


	recepie<2>(Item(BlockTypes::torchWood, 4), {Item(BlockTypes::wooden_plank, 1), Item(ItemTypes::cloth, 1)}).setAnyWood(),

	//arrows
	recepie<2>(Item(ItemTypes::arrow, 10), {Item(BlockTypes::wooden_plank, 1), Item(BlockTypes::cobblestone, 1)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::flamingArrow, 5), {Item(ItemTypes::arrow, 5), Item(BlockTypes::torchWood, 1)}),
	recepie<2>(Item(ItemTypes::goblinArrow, 5), {Item(ItemTypes::arrow, 5), Item(ItemTypes::fang, 1)}),
	recepie<2>(Item(ItemTypes::boneArrow, 5), {Item(ItemTypes::arrow, 5), Item(ItemTypes::bone, 1)}),

	//bars

	//recepie<1>(Item(ItemTypes::copperIngot, 1), {Item(BlockTypes::copperOre, 2)}),
	//recepie<1>(Item(ItemTypes::leadIngot, 1), {Item(BlockTypes::leadOre, 2)}),
	recepie<1>(Item(ItemTypes::ironIngot, 1), {Item(BlockTypes::iron_ore, 3)}),
	//recepie<1>(Item(ItemTypes::silverIngot, 1), {Item(BlockTypes::silverOre, 3)}),
	recepie<1>(Item(ItemTypes::goldIngot, 1), {Item(BlockTypes::gold_ore, 4)}),


	//tools
	recepie<2>(Item(ItemTypes::copperPickaxe, 1), {Item(ItemTypes::copperIngot, 4), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::copperAxe, 1), {Item(ItemTypes::copperIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::copperShovel, 1), {Item(ItemTypes::copperIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),

	recepie<2>(Item(ItemTypes::leadPickaxe, 1), {Item(ItemTypes::leadIngot, 4), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::leadAxe, 1), {Item(ItemTypes::leadIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::leadShovel, 1), {Item(ItemTypes::leadIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),

	recepie<2>(Item(ItemTypes::ironPickaxe, 1), {Item(ItemTypes::ironIngot, 4), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::ironAxe, 1), {Item(ItemTypes::ironIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::ironShovel, 1), {Item(ItemTypes::ironIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),

	recepie<2>(Item(ItemTypes::silverPickaxe, 1), {Item(ItemTypes::silverIngot, 4), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::silverAxe, 1), {Item(ItemTypes::silverIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::silverShovel, 1), {Item(ItemTypes::silverIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),

	recepie<2>(Item(ItemTypes::goldPickaxe, 1), {Item(ItemTypes::goldIngot, 4), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::goldAxe, 1), {Item(ItemTypes::goldIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),
	recepie<2>(Item(ItemTypes::goldShovel, 1), {Item(ItemTypes::goldIngot, 3), Item(BlockTypes::wooden_plank, 3)}).setAnyWood(),


	//armour

	recepie<1>(Item(ItemTypes::copperHelmet, 1), {Item(ItemTypes::copperIngot, 6)}),
	recepie<1>(Item(ItemTypes::copperChestPlate, 1), {Item(ItemTypes::copperIngot, 8)}),
	recepie<1>(Item(ItemTypes::copperBoots, 1), {Item(ItemTypes::copperIngot, 5)}),

	recepie<1>(Item(ItemTypes::leadHelmet, 1), {Item(ItemTypes::leadIngot, 6)}),
	recepie<1>(Item(ItemTypes::leadChestPlate, 1), {Item(ItemTypes::leadIngot, 8)}),
	recepie<1>(Item(ItemTypes::leadBoots, 1), {Item(ItemTypes::leadIngot, 5)}),

	recepie<1>(Item(ItemTypes::ironHelmet, 1), {Item(ItemTypes::ironIngot, 6)}),
	recepie<1>(Item(ItemTypes::ironChestPlate, 1), {Item(ItemTypes::ironIngot, 8)}),
	recepie<1>(Item(ItemTypes::ironBoots, 1), {Item(ItemTypes::ironIngot, 5)}),

	recepie<1>(Item(ItemTypes::silverHelmet, 1), {Item(ItemTypes::silverIngot, 6)}),
	recepie<1>(Item(ItemTypes::silverChestPlate, 1), {Item(ItemTypes::silverIngot, 8)}),
	recepie<1>(Item(ItemTypes::silverBoots, 1), {Item(ItemTypes::silverIngot, 5)}),

	recepie<1>(Item(ItemTypes::goldHelmet, 1), {Item(ItemTypes::goldIngot, 6)}),
	recepie<1>(Item(ItemTypes::goldChestPlate, 1), {Item(ItemTypes::goldIngot, 8)}),
	recepie<1>(Item(ItemTypes::goldBoots, 1), {Item(ItemTypes::goldIngot, 5)}),



};



std::vector<CraftingRecepieIndex> getAllPossibleRecepies(PlayerInventory &playerInventory)
{
	std::vector<CraftingRecepieIndex> rez;
	rez.reserve(sizeof(recepies) / sizeof(recepies[0]));

	for (int i = 0; i < sizeof(recepies) / sizeof(recepies[0]); i++)
	{

		if (canItemBeCrafted(recepies[i], playerInventory))
		{
			rez.push_back({recepies[i], i});
		}

	}

	return rez;
}



bool recepieExists(int recepieIndex)
{
	if (recepieIndex < 0) { return 0; }
	if (recepieIndex >= sizeof(recepies) / sizeof(recepies[0])) { return 0; }

	return 1;
}



CraftingRecepie getRecepieFromIndexUnsafe(int recepieIndex)
{
	return recepies[recepieIndex];
}




bool canItemBeCrafted(CraftingRecepie &recepie, PlayerInventory &inventory)
{

	Item neededItems[sizeof(recepie.items) / sizeof(recepie.items[0])];

	for (int i = 0; i < sizeof(recepie.items) / sizeof(recepie.items[0]); i++)
	{
		neededItems[i] = recepie.items[i];
	}

	for (int i = 0; i < sizeof(recepie.items) / sizeof(recepie.items[0]); i++)
	{

		if (neededItems[i].type == 0) { break; }

		for (int j = 0; j < PlayerInventory::INVENTORY_CAPACITY; j++)
		{
			bool allowedFromOtherRules = 0;

			if (recepie.anyWood)
			{
				if (neededItems[i].isBlock() && inventory.items[j].isBlock())
				{
					if (isWoodPlank(neededItems[i].type) && isWoodPlank(inventory.items[j].type))
					{
						allowedFromOtherRules = true;
					}
				}
			}

			if (areItemsTheSame(inventory.items[j], neededItems[i]) || allowedFromOtherRules)
			{
				if (neededItems[i].counter <= inventory.items[j].counter)
				{
					neededItems[i] = Item();
					break;
				}
				else
				{
					neededItems[i].counter -= inventory.items[j].counter;
				}
			}
		}
	}

	bool good = 1;
	for (int i = 0; i < sizeof(recepie.items) / sizeof(recepie.items[0]); i++)
	{
		if (neededItems[i].type != 0) { good = false; }
	}

	return good;
}


void craftItemUnsafe(CraftingRecepie &recepie, PlayerInventory &inventory)
{
	Item neededItems[sizeof(recepie.items) / sizeof(recepie.items[0])];

	for (int i = 0; i < sizeof(recepie.items) / sizeof(recepie.items[0]); i++)
	{
		neededItems[i] = recepie.items[i];
	}
	
	for (int i = 0; i < sizeof(recepie.items) / sizeof(recepie.items[0]); i++)
	{

		if (neededItems[i].type == 0) { break; }

		for (int j = 0; j < PlayerInventory::INVENTORY_CAPACITY; j++)
		{

			bool allowedFromOtherRules = 0;

			if (recepie.anyWood)
			{
				if (neededItems[i].isBlock() && inventory.items[j].isBlock())
				{
					if (isWoodPlank(neededItems[i].type) && isWoodPlank(inventory.items[j].type))
					{
						allowedFromOtherRules = true;
					}
				}
			}

			if (areItemsTheSame(inventory.items[j], neededItems[i]) || allowedFromOtherRules)
			{

				if (neededItems[i].counter == inventory.items[j].counter)
				{
					neededItems[i] = Item();
					inventory.items[j] = Item();
					break;

				}
				if (neededItems[i].counter < inventory.items[j].counter)
				{
					inventory.items[j].counter -= neededItems[i].counter;
					neededItems[i] = Item();
					break;
				}
				else
				{
					neededItems[i].counter -= inventory.items[j].counter;
					inventory.items[j] = Item();
				}
			}
		}
	}

}