#pragma once
#include <blocks.h>
#include <vector>
#include <string>
#include <cstring>
#include <gameplay/weaponStats.h>
#include <gameplay/entityStats.h>

struct ChestBlock;

constexpr static unsigned short ItemsStartPoint = 2'048;

enum ItemTypes : unsigned short
{

	stick = ItemsStartPoint, //keep the stick the first item!
	cloth,
	fang,
	bone,

	copperIngot,
	leadIngot,
	ironIngot,
	silverIngot,
	goldIngot,

	copperPickaxe,
	copperAxe,
	copperShovel,
	leadPickaxe,
	leadAxe,
	leadShovel,
	ironPickaxe,
	ironAxe,
	ironShovel,
	silverPickaxe,
	silverAxe,
	silverShovel,
	goldPickaxe,
	goldAxe,
	goldShovel,

	copperSword,
	leadSword,
	ironSword,
	silverSword,
	goldSword,


	trainingScythe,
	trainingSword,
	trainingWarHammer,
	trainingSpear,
	trainingKnife,
	trainingBattleAxe,

	copperWarHammer,
	copperSpear,
	copperKnife,
	copperBattleAxe,

	leadWarHammer,
	leadSpear,
	leadKnife,
	leadBattleAxe,

	ironWarHammer,
	ironSpear,
	ironKnife,
	ironBattleAxe,

	silverWarHammer,
	silverSpear,
	silverKnife,
	silverBattleAxe,

	goldWarHammer,
	goldSpear,
	goldKnife,
	goldBattleAxe,



	zombieSpawnEgg,
	pigSpawnEgg,
	catSpawnEgg,
	goblinSpawnEgg,
	scareCrowSpawnEgg,

	apple,
	blackBerrie,
	blueBerrie,
	cherries,
	chilliPepper,
	cocconut,
	grapes,
	lime,
	peach,
	pinapple,
	strawberry,

	applePie,

	leatherBoots,
	leatherChestPlate,
	leatherHelmet,
	
	copperBoots,
	copperChestPlate,
	copperHelmet,

	leadBoots,
	leadChestPlate,
	leadHelmet,

	ironBoots,
	ironChestPlate,
	ironHelmet,

	silverBoots,
	silverChestPlate,
	silverHelmet,

	goldBoots,
	goldChestPlate,
	goldHelmet,

	soap,
	whitePaint,
	lightGrayPaint,
	darkGrayPaint,
	blackPaint,
	brownPaint,
	redPaint,
	orangePaint,
	yellowPaint,
	limePaint,
	greenPaint,
	turqoisePaint,
	cyanPaint,
	bluePaint,
	purplePaint,
	magentaPaint,
	pinkPaint,

	copperCoin,
	silverCoin,
	goldCoin,
	diamondCoin,

	arrow,
	flamingArrow,
	goblinArrow,
	boneArrow,

	wheat,

	healingPotion,
	manaPotion,

	fireResistancePotion,
	jumpBoostPotion,
	luckPotion,
	manaRegenerationPotion,
	poisonPotion,
	recallPotion,
	regenerationPotion,
	shieldingPotion,
	speedPotion,
	stealthPotion,
	strengthPotion,
	venomusPotion,
	badLuckPotion,


	gumBox,
	bandage,
	fruitPeeler,
	pawKeychain,
	vitamins,

	lastItem,

};

const char *getItemTextureName(int itemId);




struct Item
{
	Item() {}; //todo maybe remove, it is dangerous, use item creator, or just don't add metadata by default
	Item(unsigned short type, unsigned short counter = 1):type(type), counter(counter) {};


	std::vector<unsigned char> metaData;
	unsigned short type = 0;
	unsigned short counter = 1;
	unsigned char notUsed1 = 0;
	unsigned char notUsed2 = 0;
	unsigned char notUsed3 = 0;
	unsigned char notUsed4 = 0;

	bool isBlock();

	bool isItemThatCanBeUsed();

	bool isConsumedAfterUse();

	bool isEatable();

	bool isArrow();

	bool isAmmo();

	//this is used for pakcet sending and saving
	std::size_t formatIntoData(std::vector<unsigned char> &data);

	//returns how much has been read or -1 if fail
	int readFromData(void *data, size_t size);

	void sanitize();

	unsigned short getStackSize();

	bool isTool();

	int isCraftingStation();

	bool isPaint();

	float isAxe();

	float isPickaxe();

	float isShovel();

	bool canAttack();

	bool isWeapon();

	bool isCoin() const;

	int getCoinValue() const;

	//bool isBattleAxe();
	//bool isSword();
	//bool isHammer();
	//bool isDagger();
	//bool isScythe();
	//bool isFlail();
	//bool isSpear();

	bool isHelmet();
	bool isChestplate();
	bool isBoots();
	bool isArmour();
	bool isPotion();
	bool isEquipement();


	std::string getItemName();

	std::string getItemDescription();

	std::string formatMetaDataToString();

	WeaponStats getWeaponStats();

	EntityStats getItemStats();
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

Item itemCreator(unsigned short type, unsigned short counter = 1);

float computeMineDurationTime(BlockType type, Item &item);

struct PlayerInventory
{
			
	constexpr static int MAX_EQUIPEMENT_SLOTS = 7;

											 //basic items    bonus slots   coins  amo  potions  ability equipement
	constexpr static int INVENTORY_CAPACITY = 36 +             9            + 4  +  4 +  2        + 1    + MAX_EQUIPEMENT_SLOTS;
	constexpr static int CURSOR_INDEX = INVENTORY_CAPACITY;
	constexpr static int ARMOUR_START_INDEX = INVENTORY_CAPACITY + 1;
	constexpr static int COINS_START_INDEX = 36 + 9; //COINS_START_INDEX is copper, than the rest
	constexpr static int ARROWS_START_INDEX = 36 + 9 + 4;
	constexpr static int HEALTH_POTION_INDEX = ARROWS_START_INDEX + 4;
	constexpr static int MANA_POTION_INDEX = HEALTH_POTION_INDEX + 1;
	constexpr static int ABILITY_INDEX = MANA_POTION_INDEX + 1;
	constexpr static int EQUIPEMENT_START_INDEX = ABILITY_INDEX + 1;

	constexpr static int CHEST_START_INDEX = 100;

	Item items[INVENTORY_CAPACITY] = {};
	
	Item heldInMouse = {};
	
	Item headArmour = {}; //ARMOUR_START_INDEX 
	Item chestArmour = {}; //ARMOUR_START_INDEX + 1
	Item bootsArmour = {}; //ARMOUR_START_INDEX + 2

	Item *getItemFromIndex(int index, ChestBlock *chestBlock);

	//doesn't clear data vector!!
	void formatIntoData(std::vector<unsigned char> &data);

	bool readFromData(void *data, size_t size);

	void sanitize();

	//returns how many items were picked!
	int tryPickupItem(const Item &item);

	//checks if an item can fit in a certain slot like for example coins go in coin slots only
	bool canItemFit(Item &item, int slot);

	unsigned char revisionNumber = 0;

};
