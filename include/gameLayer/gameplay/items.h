#pragma once
#include <blocks.h>
#include <vector>
#include <string>


constexpr static unsigned short ItemsStartPoint = 4'096;

enum ItemTypes
{

	stick = ItemsStartPoint,
	coal,
	wooddenSword,
	wooddenPickaxe,
	wooddenAxe,
	zombieSpawnEgg,
	pigSpawnEgg,
	catSpawnEgg,
	lastItem,

};

const char *getItemTextureName(int itemId);

struct Item
{
	Item() {};
	Item(unsigned short type, unsigned char counter = 1):type(type), counter(counter) {};


	std::vector<unsigned char> metaData;
	unsigned short type = 0;
	unsigned char counter = 1;

	bool isBlock();

	bool isItemThatCanBeUsed();

	bool isConsumedAfterUse();

	//this is used for pakcet sending and saving
	void formatIntoData(std::vector<unsigned char> &data);

	//returns how much has been read or -1 if fail
	int readFromData(void *data, size_t size);

	void sanitize();

	unsigned char getStackSize();

	bool canHaveMetaData();

	bool hasDurability();

	bool isTool();

	unsigned short getDurability();

	void setDurability(unsigned short durability);

	std::string formatMetaDataToString();
};

template<class T>
void addMetaData(std::vector<unsigned char> &vector, T data)
{
	vector.resize(vector.size() + sizeof(T));
	std::memcpy(&vector[vector.size() - sizeof(T)], &data, sizeof(T));
}


Item itemCreator(unsigned short type, unsigned char counter = 1);

struct PlayerInventory
{
	
	constexpr static int INVENTORY_CAPACITY = 36;
	constexpr static int CURSOR_INDEX = 36;
	constexpr static int CRAFTING_INDEX = 37;
	constexpr static int CRAFTING_RESULT_INDEX = 46;
	Item items[INVENTORY_CAPACITY] = {};
	
	Item heldInMouse = {};

	Item crafting[9] = {};

	Item *getItemFromIndex(int index);

	//doesn't clear data vector!!
	void formatIntoData(std::vector<unsigned char> &data);

	bool readFromData(void *data, size_t size);

	void sanitize();

	//returns how many items were picked!
	int tryPickupItem(const Item &item);

	//removes crafting ingredients from crafting slot
	void craft4(int count = 1);

	//removes crafting ingredients from crafting slot
	void craft9(int count = 1);

	unsigned char revisionNumber = 0;
};
