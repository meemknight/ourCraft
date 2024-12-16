#pragma once
#include <blocks.h>
#include <vector>
#include <string>
#include <gameplay/weaponStats.h>

constexpr static unsigned short ItemsStartPoint = 1'024;

enum ItemTypes : unsigned short
{

	stick = ItemsStartPoint, //keep the stick the first item!
	coal,
	wooddenSword,
	wooddenPickaxe,
	wooddenAxe,
	wooddenShovel,

	trainingScythe,
	trainingSword,
	trainingWarHammer,
	trainingFlail,
	trainingSpear,
	trainingKnife,
	trainingBattleAxe,


	zombieSpawnEgg,
	pigSpawnEgg,
	catSpawnEgg,
	goblinSpawnEgg,
	lastItem,

};

const char *getItemTextureName(int itemId);

struct Item
{
	Item() {}; //todo maybe remove, it is dangerous, use item creator, or just don't add metadata by default
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

	bool isTool();

	bool isAxe();

	bool isPickaxe();

	bool isShovel();

	bool canAttack();

	bool isWeapon();

	bool isBattleAxe();
	bool isSword();
	bool isHammer();
	bool isDagger();
	bool isScythe();
	bool isFlail();
	bool isSpear();

	std::string getItemName();

	std::string formatMetaDataToString();

	WeaponStats getWeaponStats();
};

//doesn't compare size
bool areItemsTheSame(Item &a, Item &b);

template<class T>
void addMetaData(std::vector<unsigned char> &vector, T data)
{
	vector.resize(vector.size() + sizeof(T));
	std::memcpy(&vector[vector.size() - sizeof(T)], &data, sizeof(T));
}

bool isItem(unsigned short type);

//check if you can put from on top of to completely and does it
bool canItemBeMovedToAndMoveIt(Item &from, Item &to);

Item itemCreator(unsigned short type, unsigned char counter = 1);

float computeMineDurationTime(BlockType type, Item &item);

struct PlayerInventory
{
	
	constexpr static int INVENTORY_CAPACITY = 36;
	constexpr static int CURSOR_INDEX = 36;
	Item items[INVENTORY_CAPACITY] = {};
	
	Item heldInMouse = {};


	Item *getItemFromIndex(int index);

	//doesn't clear data vector!!
	void formatIntoData(std::vector<unsigned char> &data);

	bool readFromData(void *data, size_t size);

	void sanitize();

	//returns how many items were picked!
	int tryPickupItem(const Item &item);

	unsigned char revisionNumber = 0;
};
