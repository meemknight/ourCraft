#pragma once
#include <gameplay/items.h>
#include <array>



struct CraftingRecepie
{

	Item items[5] = {};
	Item result = {};

	bool anyWood = 0;
	bool requiresWorkBench = 0;
	bool requiresFurnace = 0;
	bool requiresGoblin = 0;

	CraftingRecepie() {};

	CraftingRecepie &setAnyWood() { anyWood = true; return *this; }
	CraftingRecepie &setRequiresWorkBench() { requiresWorkBench = true; return *this; }
	CraftingRecepie &setRequiresGoblin() { requiresGoblin = true; return *this; }
	CraftingRecepie &setRequiresFurnace() { requiresFurnace = true; return *this; }
};

struct CraftingRecepieIndex
{
	CraftingRecepie recepie;
	int index = 0;
};


std::vector<CraftingRecepieIndex> getAllPossibleRecepies(PlayerInventory &playerInventory, int craftingStation);


bool recepieExists(int recepieIndex);

CraftingRecepie getRecepieFromIndexUnsafe(int recepieIndex);

bool canItemBeCrafted(CraftingRecepie &recepie, PlayerInventory &inventory);


//removes items from the inventory in order to craft the recepie
void craftItemUnsafe(CraftingRecepie &recepie, PlayerInventory &inventory);