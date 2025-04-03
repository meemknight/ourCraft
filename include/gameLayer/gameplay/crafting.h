#pragma once
#include <gameplay/items.h>
#include <array>






struct CraftingRecepie
{

	Item items[5] = {};
	Item result = {};

	bool anyWood = 0;
	CraftingRecepie() {};

	CraftingRecepie &setAnyWood() { anyWood = true; return *this; }
};

struct CraftingRecepieIndex
{
	CraftingRecepie recepie;
	int index = 0;
};

std::vector< CraftingRecepieIndex> getAllPossibleRecepies(PlayerInventory &playerInventory);


bool recepieExists(int recepieIndex);

CraftingRecepie getRecepieFromIndexUnsafe(int recepieIndex);

bool canItemBeCrafted(CraftingRecepie &recepie, PlayerInventory &inventory);


//removes items from the inventory in order to craft the recepie
void craftItemUnsafe(CraftingRecepie &recepie, PlayerInventory &inventory);