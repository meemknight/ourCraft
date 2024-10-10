#pragma once
#include <gameplay/items.h>
#include <array>






struct CraftingRecepie
{

	Item items[5] = {};
	Item result = {};


	CraftingRecepie() {};


};

struct CraftingRecepieIndex
{
	CraftingRecepie recepie;
	int index = 0;
};

std::vector< CraftingRecepieIndex> getAllPossibleRecepies(PlayerInventory &playerInventory);


bool recepieExists(int recepieIndex);

CraftingRecepie getRecepieFromIndexUnsafe(int recepieIndex);

//struct CraftingRecepie
//{
//
//	//add flags and stuff
//
//	Item items[9] = {};
//
//	Item result = {};
//
//	CraftingRecepie() {};
//
//	bool anyWoodType = 0;
//
//};
//
//CraftingRecepie recepie(Item result, std::array<Item, 9> items, bool anyWoodType = 0,
//	bool applyItemCreator = 1);

