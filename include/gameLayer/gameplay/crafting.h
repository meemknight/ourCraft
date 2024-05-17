#pragma once
#include <gameplay/items.h>
#include <array>


//0 1
//2 3
Item craft4(Item items[4]);


//0 1 2
//3 4 5
//6 7 8
Item craft9(Item items[9]);




struct CraftingRecepie
{

	//add flags and stuff

	Item items[9] = {};

	Item result = {};

	CraftingRecepie() {};



};

CraftingRecepie recepie(Item result, std::array<Item, 9> items);

