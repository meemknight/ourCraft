#include <gameplay/items.h>
#include <serializing.h>
#include <platformTools.h>
#include <iostream>
#include <climits>
#include <magic_enum.hpp>
#include <gameplay/crafting.h>
#include <gameplay/blocks/chestBlock.h>

//todo can be placed

bool Item::isBlock()
{
	return ::isBlock(type);
}

bool Item::isItemThatCanBeUsed()
{
	if (type == pigSpawnEgg || type == zombieSpawnEgg 
		|| type == catSpawnEgg || type == goblinSpawnEgg || type == scareCrowSpawnEgg
		|| isEatable() || isPaint() 
		)
	{
		return true;
	}

	return false;
}

bool Item::isConsumedAfterUse()
{
	if (type == pigSpawnEgg || type == zombieSpawnEgg || type == catSpawnEgg
		|| type == goblinSpawnEgg || type == scareCrowSpawnEgg
		|| isEatable() 
		)
	{
		return true;
	}

	return false;
}

bool Item::isEatable()
{
	return type == apple ||
		type == blackBerrie ||
		type == blueBerrie ||
		type == cherries ||
		type == chilliPepper ||
		type == cocconut ||
		type == grapes ||
		type == lime ||
		type == peach ||
		type == pinapple ||
		type == strawberry ||
		type == applePie
		|| isPotion()
		
		;
}

bool Item::isArrow()
{
	return
		type == arrow ||
		type == flamingArrow ||
		type == goblinArrow ||
		type == boneArrow;
}

bool Item::isAmmo()
{
	return isArrow();
}


std::size_t Item::formatIntoData(std::vector<unsigned char> &data)
{
	if (type == 0 || counter == 0)
	{
		writeData(data, (unsigned short)(0));
		return sizeof(unsigned short);
	}
	else
	{
		size_t s = data.size();

		writeData(data, type);
		writeData(data, counter);
		unsigned short metaDataSize = 0;
		permaAssert(metaData.size() < USHRT_MAX); 

		metaDataSize = metaData.size();
		writeData(data, metaDataSize);
		
		writeData(data, metaData.data(), sizeof(unsigned char) * metaData.size());

		return data.size() - s;

		//if (hasDurability())
		//{
		//	if (metaData.size() == 2)
		//	{
		//		unsigned short durability = 0;
		//		readDataUnsafe(metaData.data(), durability);
		//		writeData(data, durability);
		//	}
		//	else
		//	{
		//		permaAssert(0);
		//	}
		//}

	}

	static_assert(sizeof(unsigned short) == sizeof(type));
	static_assert(sizeof(unsigned short) == sizeof(counter));
}

int Item::readFromData(void *data, size_t size)
{
	*this = {};

	if (size < sizeof(unsigned short))
	{
		return -1;
	}

	readDataUnsafe(data, type);

	if (!type)
	{
		return sizeof(unsigned short);
	}
	else
	{
		if (size < 4)
		{
			return -1;
		}

		readDataUnsafe((unsigned char*)data + 2, counter);

		if (size < 6)
		{
			return -1;
		}

		unsigned short metaDataSize = 0;
		readDataUnsafe((unsigned char *)data + 4, metaDataSize);

		if (size - 6 < metaDataSize)
		{
			return -1; 
		}

		metaData.resize(metaDataSize);
		readDataIntoVectorUnsafeUnresized((unsigned char *)data + 7, 0, metaDataSize, metaData);

		//if (hasDurability())
		//{
		//	if (size < 5)
		//	{
		//		return -1;
		//	}
		//	unsigned short durability = 0;
		//	readDataUnsafe((unsigned char *)data + 3, durability);
		//	writeData(metaData, durability);
		//
		//	return 5;
		//}
		//else

		{
			return 6 + metaDataSize; //one short + one char + one short
		}

	}


	//unreachable
	assert(0);
}

void Item::sanitize()
{
	if (counter == 0)
	{
		*this = {};
	}
	else
	{
		if (counter > getStackSize())
		{
			counter = getStackSize();
		}
	}

}

unsigned short Item::getStackSize()
{
	if (isAmmo())
	{
		return 999;
	}else if (isTool() || isPaint() || isWeapon() || isArmour() || isPotion()
		|| isEquipement()
		)
	{
		return 1;
	}
	else if (isCoin())
	{
		return 100;
	}
	else
	{
		return 999;
	}

}

bool Item::isTool()
{
	if (isAxe() || isPickaxe() || isShovel()
		)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int Item::isCraftingStation()
{
	return ::isCraftingStation(type);
}

bool Item::isPaint()
{
	return type >= soap && type <= pinkPaint;
}

float Item::isAxe()
{
	switch (type)
	{
		case copperAxe: return 25;
		case leadAxe: return 50;
		case ironAxe: return 70;
		case silverAxe: return 80;
		case goldAxe: return 100;

	}

	return 0;
}

float Item::isPickaxe()
{
	switch (type)
	{
		case copperPickaxe: return 25;
		case leadPickaxe: return 50;
		case ironPickaxe: return 70;
		case silverPickaxe: return 80;
		case goldPickaxe: return 100;
	}

	return 0;
}

float Item::isShovel()
{
	switch (type)
	{
		case copperShovel: return 25;
		case leadShovel: return 50;
		case ironShovel: return 70;
		case silverShovel: return 80;
		case goldShovel: return 100;

	}

	return 0;
}

bool Item::canAttack()
{
	if (
		type == trainingScythe ||
		type == trainingSword ||
		type == trainingWarHammer ||
		type == trainingSpear ||
		type == trainingKnife ||
		type == trainingBattleAxe)
	{
		return true;
	}

	return 0;
}

bool Item::isWeapon()
{

	return
		type >= copperSword && type <= goldBattleAxe;

	//return 
	//	isBattleAxe() ||
	//	isSword() ||
	//	isHammer() ||
	//	isDagger() ||
	//	isScythe() ||
	//	isFlail() ||
	//	isSpear();
}

bool Item::isCoin() const
{
	return
		type == copperCoin ||
		type == silverCoin ||
		type == goldCoin ||
		type == diamondCoin;
}

int Item::getCoinValue() const
{
	if (type == copperCoin) { return counter; }
	if (type == silverCoin) { return counter * 100; }
	if (type == goldCoin) { return counter * 100'00; }
	if (type == diamondCoin) { return counter * 100'00'00; }

	return 0;
}

//higher armour penetration, basically a less extreme version of the hammer
//bool Item::isBattleAxe()
//{
//	return type == trainingBattleAxe;
//}

//well balanced in general
//bool Item::isSword()
//{
//	return type == trainingSword ||
//		type == copperSword ||
//		type == leadSword ||
//		type == ironSword ||
//		type == silverSword ||
//		type == goldSword;
//
//}

//extra slow, heavy, extremely good armour penetration and knock back
//bool Item::isHammer()
//{
//	return type == trainingWarHammer;
//}

//extra fast, very high crit and surprize damage
//bool Item::isDagger()
//{
//	return type == trainingKnife;
//}

//high damage high crit, low armour penetration, so good for like "flesh" enemies
//bool Item::isScythe()
//{
//	return type == trainingScythe;
//}

//bool Item::isFlail()
//{
//	return 0;
//}

//high range
//bool Item::isSpear()
//{
//	return type == trainingSpear;
//}

bool Item::isHelmet()
{
	return
		type == leatherHelmet ||
		type == copperHelmet ||
		type == leadHelmet ||
		type == ironHelmet ||
		type == silverHelmet ||
		type == goldHelmet;
}

bool Item::isChestplate()
{
	return
		type == leatherChestPlate ||
		type == copperChestPlate ||
		type == leadChestPlate ||
		type == ironChestPlate ||
		type == silverChestPlate ||
		type == goldChestPlate;
}

bool Item::isBoots()
{
	return
		type == leatherBoots ||
		type == copperBoots ||
		type == leadBoots ||
		type == ironBoots ||
		type == silverBoots ||
		type == goldBoots;
}

bool Item::isArmour()
{
	return isHelmet() || isChestplate() || isBoots();
}

bool Item::isPotion()
{
	return (type >= healingPotion && type <= badLuckPotion);
}

bool Item::isEquipement()
{
	return (type >= gumBox && type <= vitamins);
}

std::string Item::formatMetaDataToString()
{

	std::string rez = getItemName();

	if (counter > 1)
	{
		rez += " x";
		rez += std::to_string(counter);
	}

	float pickaxe = isPickaxe();
	float axe = isAxe();
	float shovel = isShovel();

	if (isWeapon())
	{
		rez += getWeaponStats().formatDataToString();
	}

	if (pickaxe)
		{ rez += "\nPickaxe Power: " + std::to_string(int(pickaxe)) + "%"; }
	if (axe)
		{ rez += "\nAxe Power: " + std::to_string(int(axe)) + "%"; }
	if (shovel)
		{ rez += "\nShovel Power: " + std::to_string(int(shovel)) + "%"; }

	if (metaData.size())
	{
		rez += "\nHas metadata";
	}

	rez += getItemStats().formatDataToString();

	auto desc = "\"" + getItemDescription() + "\"";

	if (desc.size() > 2)
	{
		rez += "\n";
		rez += desc;
	}

	return rez;
}


EntityStats Item::getItemStats()
{
	EntityStats ret;

	switch (type)
	{

		case leatherBoots: ret.armour = 1; break;
		case leatherChestPlate: ret.armour = 1; break;		//1 + 1 defence set bonus
		case leatherHelmet: ret.armour = 1; break;

		case copperBoots: ret.armour = 1; break;
		case copperChestPlate: ret.armour = 2; break;		//2		+ 2 defence set bonus
		case copperHelmet: ret.armour = 1; break;

		case leadBoots: ret.armour = 2; break;
		case leadChestPlate: ret.armour = 4; break;			//3		+ 1 defence 
		case leadHelmet: ret.armour = 2; break;

		case ironBoots: ret.armour = 3; break;
		case ironChestPlate: ret.armour = 5; break;			//4		+ 5% mele attack speed
		case ironHelmet: ret.armour = 2; ret.meleAttackSpeed = 3; break;

		case silverBoots: ret.armour = 5; break;
		case silverChestPlate: ret.armour = 6; break;		//5		
		case silverHelmet: ret.armour = 5; break;

		case goldBoots: ret.armour = 7; break;
		case goldChestPlate: ret.armour = 11; break;			//6
		case goldHelmet: ret.armour = 7; break;



		case pawKeychain: ret.stealthSound = 15; break;


	};


	return ret;
}


Item *PlayerInventory::getItemFromIndex(int index, ChestBlock *chestBlock)
{
	if (index < 0) { return 0; }

	if (index < PlayerInventory::INVENTORY_CAPACITY)
	{
		return &items[index];
	}
	else if (index == PlayerInventory::INVENTORY_CAPACITY)
	{
		return &heldInMouse;
	}
	else if (index == PlayerInventory::ARMOUR_START_INDEX)
	{
		return &headArmour;
	}
	else if (index == PlayerInventory::ARMOUR_START_INDEX + 1)
	{
		return &chestArmour;
	}
	else if (index == PlayerInventory::ARMOUR_START_INDEX + 2)
	{
		return &bootsArmour;
	}

	if (index >= CHEST_START_INDEX && (index < CHEST_START_INDEX + CHEST_CAPACITY) && chestBlock)
	{
		return &(chestBlock->items[index - CHEST_START_INDEX]);
	}

	return nullptr;
}


void PlayerInventory::formatIntoData(std::vector<unsigned char> &data)
{

	//TODO ARMOUR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	data.reserve(data.size() + INVENTORY_CAPACITY * sizeof(Item)); //rough estimate

	data.push_back(revisionNumber);

	for (int i = 0; i < INVENTORY_CAPACITY; i++)
	{
		items[i].formatIntoData(data);
	}

	heldInMouse.formatIntoData(data);


}

bool PlayerInventory::readFromData(void *data, size_t size)
{
	*this = {};

	size_t currentAdvance = 0;

	auto readOne = [&](Item &item)
	{
		int rez = item.readFromData((void *)((unsigned char *)data + currentAdvance), size - currentAdvance);

		if (rez > 0)
		{
			currentAdvance += rez;
		}
		else
		{
			return false;
		}

		return true;
	};
	
	if (size < 1) { return 0; }

	//we first read the revision number
	revisionNumber = ((unsigned char *)data)[0];
	currentAdvance++;

	for (int i = 0; i < INVENTORY_CAPACITY; i++)
	{
		if (!readOne(items[i])) { return 0; }
	}

	if (!readOne(heldInMouse)) { return 0; }

	
	return true;
}

void PlayerInventory::sanitize()
{

	for (int i = 0; i < ARMOUR_START_INDEX + 3; i++)
	{
		getItemFromIndex(i, nullptr)->sanitize();
	}

	//for (int i = 0; i < INVENTORY_CAPACITY; i++)
	//{
	//	items[i].sanitize();
	//}
	//
	//heldInMouse.sanitize();

}

int PlayerInventory::tryPickupItem(const Item &item)
{
	int currentCounter = 0;
	int itemCounter = item.counter;

	if (item.isCoin())
	{

		std::int64_t currentMoney = 0;
		std::int64_t maxCoinCapacity = 100 + 100'00 + 100'00'00 + 100'00'00'00;


		for (int i = 0; i < 4; i++)
		{
			currentMoney += items[COINS_START_INDEX + i].getCoinValue();
		}

		std::int64_t itemValue = item.getCoinValue();

		if (currentMoney + itemValue > maxCoinCapacity) 
		{ 
			std::int64_t oneItemValue = itemValue/ item.counter;
			if (currentMoney + oneItemValue > maxCoinCapacity)
			{
				//we can't do anything
			}
			else
			{
				//try pick it up....
				//TODO
				std::int64_t remainingSlots = maxCoinCapacity - currentMoney;
				std::int64_t canPickUpCount = remainingSlots / oneItemValue;

				currentCounter += canPickUpCount;
				itemCounter -= canPickUpCount;

				{
					currentMoney += canPickUpCount * oneItemValue;

					//we redistribute the money
					//if (currentMoney >= 100'00'00)
					{
						int diamondValue = currentMoney / 100'00'00; diamondValue = std::min(diamondValue, 100);
						items[COINS_START_INDEX + 3].type = ItemTypes::diamondCoin;
						items[COINS_START_INDEX + 3].counter = diamondValue;
						items[COINS_START_INDEX + 3].sanitize();
						currentMoney -= diamondValue * 100'00'00;
					}

					//if (currentMoney >= 100'00)
					{
						int goldValue = currentMoney / 100'00; goldValue = std::min(goldValue, 100);
						items[COINS_START_INDEX + 2].type = ItemTypes::goldCoin;
						items[COINS_START_INDEX + 2].counter = goldValue;
						items[COINS_START_INDEX + 2].sanitize();
						currentMoney -= goldValue * 100'00;
					}

					//if (currentMoney >= 100)
					{
						int silverValue = currentMoney / 100; silverValue = std::min(silverValue, 100);
						items[COINS_START_INDEX + 1].type = ItemTypes::silverCoin;
						items[COINS_START_INDEX + 1].counter = silverValue;
						items[COINS_START_INDEX + 1].sanitize();
						currentMoney -= silverValue * 100;
					}

					//if (currentMoney > 0)
					{
						items[COINS_START_INDEX].type = ItemTypes::copperCoin;
						items[COINS_START_INDEX].counter = currentMoney;
						items[COINS_START_INDEX].sanitize();
					}
				}
			}

		}
		else
		{
			currentMoney += itemValue;

			//we redistribute the money
			//if (currentMoney >= 100'00'00)
			{
				int diamondValue = currentMoney / 100'00'00; diamondValue = std::min(diamondValue, 100);
				items[COINS_START_INDEX + 3].type = ItemTypes::diamondCoin;
				items[COINS_START_INDEX + 3].counter = diamondValue;
				items[COINS_START_INDEX + 3].sanitize();
				currentMoney -= diamondValue * 100'00'00;
			}

			//if (currentMoney >= 100'00)
			{
				int goldValue = currentMoney / 100'00; goldValue = std::min(goldValue, 100);
				items[COINS_START_INDEX + 2].type = ItemTypes::goldCoin;
				items[COINS_START_INDEX + 2].counter = goldValue;
				items[COINS_START_INDEX + 2].sanitize();
				currentMoney -= goldValue * 100'00;
			}

			//if (currentMoney >= 100)
			{
				int silverValue = currentMoney / 100; silverValue = std::min(silverValue, 100);
				items[COINS_START_INDEX + 1].type = ItemTypes::silverCoin;
				items[COINS_START_INDEX + 1].counter = silverValue;
				items[COINS_START_INDEX + 1].sanitize();
				currentMoney -= silverValue * 100;
			}

			//if (currentMoney > 0)
			{
				items[COINS_START_INDEX].type = ItemTypes::copperCoin;
				items[COINS_START_INDEX].counter = currentMoney;
				items[COINS_START_INDEX].sanitize();
			}

			return itemCounter;
		}

	}

	for (int i = 0; i < INVENTORY_CAPACITY; i++)
	{
		if (!items[i].type)
		{
			items[i] = item;
			items[i].counter = itemCounter;
			return itemCounter + currentCounter;
		}
		else if (items[i].type == item.type)
		{

			if (items[i].counter < items[i].getStackSize())
			{

				if (items[i].counter + itemCounter <= items[i].getStackSize())
				{
					items[i].counter += itemCounter;
					return itemCounter + currentCounter;
				}
				else
				{
					int taken = items[i].getStackSize() - items[i].counter;
					currentCounter += taken;
					items[i].counter = items[i].getStackSize();
					itemCounter -= taken;
					//take as much as possible and continue
				}

			}

		}
	}

	return 0;
}

bool PlayerInventory::canItemFit(Item &item, int slot)
{

	if (slot == ARMOUR_START_INDEX && !item.isHelmet()) { return false; }
	if (slot == ARMOUR_START_INDEX + 1 && !item.isChestplate()) { return false; }
	if (slot == ARMOUR_START_INDEX + 2 && !item.isBoots()) { return false; }

	if (slot >= ARROWS_START_INDEX && slot < ARROWS_START_INDEX + 4)
	{
		if (!item.isAmmo())
		{
			return false;
		}
	}

	if (slot >= EQUIPEMENT_START_INDEX && slot < EQUIPEMENT_START_INDEX + MAX_EQUIPEMENT_SLOTS)
	{
		if (!item.isEquipement())
		{
			return false;
		}
	}

	if (slot >= COINS_START_INDEX && slot < COINS_START_INDEX + 4)
	{
		if (!item.isCoin())
		{
			return false;
		}
	}

	return true;
}



//for textures
const char *itemsNamesTextures[] = 
{
	"stick.png",
	"cloth.png",
	"fang.png",
	"bone.png",

	"copperIngot.png",
	"leadIngot.png",
	"ironIngot.png",
	"silverIngot.png",
	"goldIngot.png",

	"tools/copperPickaxe.png",
	"tools/copperAxe.png",
	"tools/copperShovel.png",
	"tools/leadPickaxe.png",
	"tools/leadAxe.png",
	"tools/leadShovel.png",
	"tools/ironPickaxe.png",
	"tools/ironAxe.png",
	"tools/ironShovel.png",
	"tools/silverPickaxe.png",
	"tools/silverAxe.png",
	"tools/silverShovel.png",
	"tools/goldPickaxe.png",
	"tools/goldAxe.png",
	"tools/goldShovel.png",


	"weapons/copperSword.png",
	"weapons/leadSword.png",
	"weapons/ironSword.png",
	"weapons/silverSword.png",
	"weapons/goldSword.png",

	"weapons/woodenScythe.png",
	"weapons/woodenSword.png",
	"weapons/woodenWarHammer.png",
	"weapons/woodenSpear.png",
	"weapons/woodenKnife.png",
	"weapons/woodenBattleAxe.png",

	"weapons/copperWarHammer.png",
	"weapons/copperSpear.png",
	"weapons/copperKnife.png",
	"weapons/copperBattleAxe.png",
	"weapons/leadWarHammer.png",
	"weapons/leadSpear.png",
	"weapons/leadKnife.png",
	"weapons/leadBattleAxe.png",
	"weapons/ironWarHammer.png",
	"weapons/ironSpear.png",
	"weapons/ironKnife.png",
	"weapons/ironBattleAxe.png",
	"weapons/silverWarHammer.png",
	"weapons/silverSpear.png",
	"weapons/silverKnife.png",
	"weapons/silverBattleAxe.png",
	"weapons/goldWarHammer.png",
	"weapons/goldSpear.png",
	"weapons/goldKnife.png",
	"weapons/goldBattleAxe.png",


	"", //eggs
	"",
	"",
	"spawnEggs/goblin.png",
	"spawnEggs/scareCrow.png",

	"food/apple.png",
	"food/blackBerrie.png",
	"food/blueBerrie.png",
	"food/cherries.png",
	"food/chilliPepper.png",
	"food/cocconut.png",
	"food/grapes.png",
	"food/lime.png",
	"food/peach.png",
	"food/pinapple.png",
	"food/strawberry.png",
	"food/applePie.png",

	"armour/leatherBoots.png",
	"armour/leatherChestplate.png",
	"armour/leatherHelmet.png",

	"armour/copperBoots.png",
	"armour/copperChestPlate.png",
	"armour/copperHelmet.png",
	"armour/leadBoots.png",
	"armour/leadChestPlate.png",
	"armour/leadHelmet.png",
	"armour/ironBoots.png",
	"armour/ironChestPlate.png",
	"armour/ironHelmet.png",
	"armour/silverBoots.png",
	"armour/silverChestPlate.png",
	"armour/silverHelmet.png",
	"armour/goldBoots.png",
	"armour/goldChestPlate.png",
	"armour/goldHelmet.png",

	"paint/soap.png",
	"paint/whitePaint.png",
	"paint/lightGrayPaint.png",
	"paint/darkGrayPaint.png",
	"paint/blackPaint.png",
	"paint/brownPaint.png",
	"paint/redPaint.png",
	"paint/orangePaint.png",
	"paint/yellowPaint.png",
	"paint/limePaint.png",
	"paint/greenPaint.png",
	"paint/turqoisePaint.png",
	"paint/cyanPaint.png",
	"paint/bluePaint.png",
	"paint/purplePaint.png",
	"paint/magentaPaint.png",
	"paint/pinkPaint.png",

	"copperCoin.png",
	"silverCoin.png",
	"goldCoin.png",
	"diamondCoin.png",

	"ammo/arrow.png",
	"ammo/flamingArrow.png",
	"ammo/goblinArrow.png",
	"ammo/boneArrow.png",
	"wheat.png",

	"potions/healingPotion.png",
	"potions/manaPotion.png",
	"potions/fireResistancePotion.png",
	"potions/jumpBoostPotion.png",
	"potions/luckPotion.png",
	"potions/manaRegenPotion.png",
	"potions/poisonPotion.png",
	"potions/recallPotion.png",
	"potions/regenerationPotion.png",
	"potions/shieldingPotion.png",
	"potions/speedPotion.png",
	"potions/stealthPotion.png",
	"potions/strengthPotion.png",
	"potions/venomusPotion.png",
	"potions/badLuckPotion.png",

	"equipement/gumBox.png",
	"equipement/bandage.png",
	"equipement/fruitPeeler.png",
	"equipement/pawKeychain.png",
	"equipement/vitamins.png",

};


//for textures
const char *item3DModelName[] =
{
	"stick",
	"cloth",
	"fang",
	"bone",

	"copperIngot",
	"leadIngot",
	"ironIngot",
	"silverIngot",
	"goldIngot",

	"copperPickaxe",
	"copperAxe",
	"copperShovel",
	"leadPickaxe",
	"leadAxe",
	"leadShovel",
	"ironPickaxe",
	"ironAxe",
	"ironShovel",
	"silverPickaxe",
	"silverAxe",
	"silverShovel",
	"goldPickaxe",
	"goldAxe",
	"goldShovel",


	"copperSword",
	"leadSword",
	"ironSword",
	"silverSword",
	"goldSword",

	"woodenScythe",
	"woodenSword",
	"woodenWarHammer",
	"woodenSpear",
	"woodenKnife",
	"woodenBattleAxe",

	"copperWarHammer",
	"copperSpear",
	"copperKnife",
	"copperBattleAxe",
	"leadWarHammer",
	"leadSpear",
	"leadKnife",
	"leadBattleAxe",
	"ironWarHammer",
	"ironSpear",
	"ironKnife",
	"ironBattleAxe",
	"silverWarHammer",
	"silverSpear",
	"silverKnife",
	"silverBattleAxe",
	"goldWarHammer",
	"goldSpear",
	"goldKnife",
	"goldBattleAxe",


	"", //eggs
	"",
	"",
	"goblin",
	"scareCrow",

	"apple",
	"blackBerrie",
	"blueBerrie",
	"cherries",
	"chilliPepper",
	"cocconut",
	"grapes",
	"lime",
	"peach",
	"pinapple",
	"strawberry",
	"applePie",

	"leatherBoots",
	"leatherChestplate",
	"leatherHelmet",

	"copperBoots",
	"copperChestPlate",
	"copperHelmet",
	"leadBoots",
	"leadChestPlate",
	"leadHelmet",
	"ironBoots",
	"ironChestPlate",
	"ironHelmet",
	"silverBoots",
	"silverChestPlate",
	"silverHelmet",
	"goldBoots",
	"goldChestPlate",
	"goldHelmet",

	"soap",
	"whitePaint",
	"lightGrayPaint",
	"darkGrayPaint",
	"blackPaint",
	"brownPaint",
	"redPaint",
	"orangePaint",
	"yellowPaint",
	"limePaint",
	"greenPaint",
	"turqoisePaint",
	"cyanPaint",
	"bluePaint",
	"purplePaint",
	"magentaPaint",
	"pinkPaint",

	"copperCoin",
	"silverCoin",
	"goldCoin",
	"diamondCoin",

	"arrow",
	"flamingArrow",
	"goblinArrow",
	"boneArrow",
	"wheat",

	"healingPotion",
	"manaPotion",
	"fireResistancePotion",
	"jumpBoostPotion",
	"luckPotion",
	"manaRegenPotion",
	"poisonPotion",
	"recallPotion",
	"regenerationPotion",
	"shieldingPotion",
	"speedPotion",
	"stealthPotion",
	"strengthPotion",
	"venomusPotion",
	"badLuckPotion",

	"gumBox",
	"bandage",
	"fruitPeeler",
	"pawKeychain",
	"vitamins",

};


const char *itemsNames[] =
{
	"stick",
	"cloth",
	"fang",
	"bone",

	"copperIngot",
	"leadIngot",
	"ironIngot",
	"silverIngot",
	"goldIngot",

	"copper pickaxe",
	"copper axe",
	"copper shovel",
	"lead pickaxe",
	"lead axe",
	"lead shovel",
	"iron pickaxe",
	"iron axe",
	"iron shovel",
	"silver pickaxe",
	"silver axe",
	"silver shovel",
	"gold pickaxe",
	"gold axe",
	"gold shovel",

	"copper sword",
	"lead sword",
	"iron sword",
	"silver sword",
	"gold sword",

	"trainingScythe",
	"trainingSword",
	"trainingWarHammer",
	"trainingSpear",
	"trainingKnife",
	"trainingBattleAxe",

	"Copper War Hammer",
	"Copper Spear",
	"Copper Knife",
	"Copper Battle Axe",
	"Lead WarHammer",
	"Lead Spear",
	"Lead Knife",
	"Lead BattleAxe",

	"Iron War Hammer",
	"Iron Spear",
	"Iron Knife",
	"Iron Battle Axe",


	"silver WarHammer",
	"silver Spear",
	"silver Knife",
	"silver BattleAxe",
	"gold WarHammer",
	"gold Spear",
	"gold Knife",
	"gold BattleAxe",



	"zombie spawn egg", //eggs
	"pig spawn egg",
	"cat spawn egg",
	"goblin spawn egg",
	"posessed scarecrow spawn egg",

	"apple",
	"blackBerrie",
	"blueBerrie",
	"cherries",
	"chilliPepper",
	"cocconut",
	"grapes",
	"lime",
	"peach",
	"pinapple",
	"strawberry",	

	"Apple Pie",


	"leather boots",
	"leather ChestPlate",
	"leather cap",
	"copper boots",
	"copper ChestPlate",
	"copper cap",
	"lead boots",
	"lead ChestPlate",
	"lead cap",
	"iron boots",
	"iron ChestPlate",
	"iron cap",
	"silver boots",
	"silver ChestPlate",
	"silver cap",
	"gold boots",
	"gold ChestPlate",
	"gold cap",

	"soap",
	"white paint",
	"lightGray paint",
	"darkGray paint",
	"black paint",
	"brown paint",
	"red paint",
	"orange paint",
	"yellow paint",
	"lime paint",
	"green paint",
	"turqoise paint",
	"cyan paint",
	"blue paint",
	"purple paint",
	"magenta paint",
	"pink paint",
	
	"copper coin",
	"silver coin",
	"gold coin",
	"diamond coin",

	"wooden arrow",
	"flaming arrow",
	"goblin arrow",
	"bone arrow",
	"wheat",

	"Healing Potion",
	"Mana Potion",

	"Fire Resistance Potion",
	"JumpBoost Potion",
	"Luck Potion",
	"Mana Regeneration Potion",
	"Poison Potion",
	"Recall Potion",
	"Regeneration Potion",
	"Shielding Potion",
	"Speed Potion",
	"Stealth Potion",
	"Strength Potion",
	"Venomus Potion",
	"Bad Luck Potion",

	"Gum Box",
	"Bandage",
	"Fruit Peeler",
	"Paw Keychain",
	"Vitamins",
};

const char *getItemTextureName(int itemId)
{
	static_assert(sizeof(itemsNamesTextures) / sizeof(itemsNamesTextures[0]) == lastItem - ItemsStartPoint);
	static_assert(sizeof(itemsNames) / sizeof(itemsNames[0]) == lastItem - ItemsStartPoint);

	return itemsNamesTextures[itemId-ItemsStartPoint];
}

const char *getItem3DModelName(int itemId)
{
	static_assert(sizeof(item3DModelName) / sizeof(item3DModelName[0]) == lastItem - ItemsStartPoint);

	return item3DModelName[itemId - ItemsStartPoint];
}



//doesn't compare size
bool areItemsTheSame(Item &a, Item &b)
{
	if (a.type != b.type)
	{
		return 0;
	}

	if (a.metaData != b.metaData)
	{
		return 0;
	}

	return 1;
}


bool isItem(unsigned short type)
{
	return type >= ItemsStartPoint && type < ItemTypes::lastItem;
}


bool canItemBeMovedToAndMoveIt(Item &from, Item &to)
{

	if (to.type == 0)
	{
		to = std::move(from);
		from = Item();
		return true;
	}
	else if(areItemsTheSame(from, to))
	{

		int totalSize = from.counter + to.counter;

		if (totalSize > from.getStackSize())
		{
			return 0;
		}
		else
		{
			to.counter += from.counter;
			from = Item();
			return 1;
		}
	}

	return 0;

}


//create item createItem
Item itemCreator(unsigned short type, unsigned short counter)
{
	if (!counter) { return {}; }

	Item ret(type);
	ret.counter = counter;

	ret.sanitize();

	return ret;
}

float computeMineDurationTime(BlockType type, Item &item)
{

	float timer = getBlockBaseMineDuration(type);

	const float PENALTY = 3;

	bool canBeMinedHand = canBeMinedByHand(type);

	float pickaxe = (item.isPickaxe()*5 + 50) / 50.f;
	float shovel = (item.isShovel()*1 + 50) / 50.f;
	float axe = (item.isAxe()*2 + 50) / 50.f;
	
	if (pickaxe > 1)
	{
		if (canBeMinedByPickaxe(type))
		{
			//todo add speed here;
			timer /= pickaxe;
		}
		else if (!canBeMinedHand)
		{
			timer *= PENALTY;
		}
	}
	else if (shovel > 1)
	{
		if (canBeMinedByShovel(type))
		{
			//todo add speed here;
			timer /= shovel;
		}
		else if (!canBeMinedHand)
		{
			timer *= PENALTY;
		}
	}
	else if (axe > 1)
	{
		if (canBeMinedByAxe(type))
		{
			//todo add speed here;
			timer /= axe;
		}
		else if (!canBeMinedHand)
		{
			timer *= PENALTY;
		}
	}
	else //hand or no tool
	{
		if (!canBeMinedHand)
		{
			timer *= PENALTY;
		}
	}
	
	std::cout << timer << "\n";
	return timer;
}


char *blockNames[] = {
	"Air",
	"Grass Block",
	"Dirt",
	"Stone",
	"Ice",
	"Wood Log",
	"Wooden Plank",
	"Cobblestone",
	"Gold Block",
	"Bricks",
	"Sand",
	"Sandstone",
	"Snowy Dirt",
	"Leaves",
	"Gold Ore",
	"Copper Ore",
	"Stone Brick",
	"Iron Ore",
	"Silver Ore",
	"Bookshelf",
	"Birch Log",
	"Gravel",
	"Grass",
	"Rose",
	"Water",
	"Jungle Log",
	"Jungle Leaves",
	"Palm Log",
	"Palm Leaves",
	"Cactus Bud",
	"Dead Bush",
	"Jungle Planks",
	"Clay",
	"Clay Bricks",
	"Mud",
	"Red Clay",
	"Red Clay Bricks",
	"Control Block 1",
	"Control Block 2",
	"Control Block 3",
	"Control Block 4",
	"Snow Block",
	"Birch Leaves",
	"Spruce Log",
	"Spruce Leaves",
	"Red Spruce Leaves",
	"Glowstone",
	"Glass",
	"Test Block",
	"Torch",
	"Lead Ore",
	"Coarse Dirt",
	"Birch Planks",
	"Path Block",
	"Planked Stone Block",
	"Planked Stone Stairs",
	"Planked Stone",
	"Terracotta",
	"Terracotta Stairs",
	"Terracotta Slabs",
	"Terracotta Wall",
	"Opaque Glass",
	"Vitral 1",
	"Vitral 2",
	"Opaque Glass 2",
	"Glass Variant 2",
	"Mossy Cobblestone",
	"Magenta Stained Glass",
	"Pink Stained Glass",
	"Cloth Block",
	"Wooden Stairs",
	"Wooden Slab",
	"Wooden Wall",
	"Tiled Stone Bricks",
	"Stone Stairs",
	"Stone Slabs",
	"Stone Wall",
	"Cobblestone Stairs",
	"Cobblestone Slabs",
	"Cobblestone Wall",
	"Tiled Stone Brick Stairs",
	"Tiled Stone Brick Slabs",
	"Tiled Stone Brick Wall",
	"Brick Stairs",
	"Brick Slabs",
	"Brick Wall",
	"Stone Brick Stairs",
	"Stone Brick Slabs",
	"Stone Brick Wall",
	"Structure Base",
	"Hard Sandstone",
	"Hard Sandstone Stairs",
	"Hard Sandstone Slabs",
	"Hard Sandstone Wall",
	"Sandstone Stairs",
	"Sandstone Slabs",
	"Sandstone Wall",
	"Dungeon Bricks",
	"Dungeon Brick Stairs",
	"Dungeon Brick Slabs",
	"Dungeon Brick Wall",
	"Volcanic Hot Rock",
	"Volcanic Rock",
	"Volcanic Rock Stairs",
	"Volcanic Rock Slabs",
	"Volcanic Rock Wall",
	"Smooth Stone",
	"Smooth Stone Stairs",
	"Smooth Stone Slabs",
	"Smooth Stone Wall",
	"Limestone",
	"Smooth Limestone",
	"Smooth Limestone Stairs",
	"Smooth Limestone Slabs",
	"Smooth Limestone Wall",
	"Marble Block",
	"Marble Block Stairs",
	"Marble Block Slabs",
	"Marble Block Wall",
	"Smooth Marble Block",
	"Marble Bricks",
	"Marble Brick Stairs",
	"Marble Brick Slabs",
	"Marble Brick Wall",
	"Marble Pillar",
	"Log Wall",
	"Yellow Grass",
	"Blue Bricks",
	"Blue Brick Stairs",
	"Blue Brick Slabs",
	"Blue Brick Wall",
	"Oak Chair",
	"Oak Log Chair",
	"Mug",
	"Goblet",
	"Wine Bottle",
	"Skull",
	"Skull Torch",
	"Book",
	"Candle Holder",
	"Pot",
	"Jar",
	"Globe",
	"Keg",
	"Workbench",
	"Oak Table",
	"Oak Log Table",
	"Crafting Tools",
	"Oak Big Chair",
	"Oak Log Big Chair",
	"Cooking Pot",
	"Chicken Carcass",
	"Chicken Wings Plate",
	"Fish Plate",
	"Ladder",
	"Vines",
	"Cloth Stairs",
	"Cloth Slabs",
	"Cloth Wall",
	"Birch Plank Stairs",
	"Birch Plank Slabs",
	"Birch Plank Wall",
	"Oak Log Slab",
	"Small Rock",
	"Stripped Oak Log",
	"Stripped Birch Log",
	"Stripped Spruce Log",
	"Spruce Plank",
	"Spruce Plank Stairs",
	"Spruce Plank Slabs",
	"Spruce Plank Wall",
	"Dungeon Stone",
	"Dungeon Stone Stairs",
	"Dungeon Stone Slabs",
	"Dungeon Stone Wall",
	"Dungeon Cobblestone",
	"Dungeon Cobblestone Stairs",
	"Dungeon Cobblestone Slabs",
	"Dungeon Cobblestone Wall",
	"Dungeon Smooth Stone",
	"Dungeon Smooth Stone Stairs",
	"Dungeon Smooth Stone Slabs",
	"Dungeon Smooth Stone Wall",
	"Dungeon Pillar",
	"Dungeon Skull Block",
	"Chiseled Dungeon Brick",
	"Dungeon Glass",
	"Wooden Chest",
	"Goblin Chest",
	"Copper Chest",
	"Iron Chest",
	"Silver Chest",
	"Gold Chest",
	"Crate",
	"Small Crate",
	"Lamp",
	"Wood Torch",

	"Mossy Cobblestone Stairs",
	"Mossy Cobblestone Slab",
	"Mossy Cobblestone Wall",
	"Cobweb",
	"Hay balde",
	"Training dummy",
	"Target",

	"Furnace",

	"Goblin Workbench",
	"Goblin chair",
	"Goblin table",
	"Goblin torch",
	"Goblin stitching station",

	"Wooden Fence",

	"Hard Wood Fence",
	"Spruce Fence",
	"Hard Spruce Fence",
	"Birch Fence",
	"Hard Birch Fence",

};

std::string Item::getItemName()
{

	static_assert(sizeof(blockNames) / sizeof(blockNames[0]) == BlocksCount);
	if (isItem(type))
	{
		return itemsNames[type - ItemsStartPoint];
	}
	else
	{
		//return std::string(magic_enum::enum_name((BlockTypes)type));
		return blockNames[type];
	}
}
