#pragma once
#include <blocks.h>
#include <vector>


constexpr static unsigned short ItemsStartPoint = 4'096;

enum ItemTypes
{

	stick = ItemsStartPoint,


};


struct Item
{
	Item() {};
	Item(unsigned short type, unsigned char counter = 1):type(type), counter(counter) {};

	unsigned short type = 0;
	std::vector<unsigned char> metaData;
	unsigned char counter = 1;

	bool isBlock();

	//this is used for pakcet sending and saving
	void formatIntoData(std::vector<unsigned char> &data);

	//returns how much has been read or -1 if fail
	int readFromData(void *data, size_t size);

};

struct PlayerInventory
{
	
	constexpr static int INVENTORY_CAPACITY = 36;
	Item items[INVENTORY_CAPACITY] = {};
	
	Item heldInMouse = {};


	//todo
	void sanitize() {};
	
	//doesn't clear data vector!!
	void formatIntoData(std::vector<unsigned char> &data);

	bool readFromData(void *data, size_t size);
};