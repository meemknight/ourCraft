#include "rendering/UiEngine.h"
#include <platform/platformInput.h>
#include <gameplay/items.h>
#include <blocksLoader.h>
#include <gameplay/life.h>
#include <gamePlayLogic.h>
#include <gameplay/player.h>
#include <gameplay/blocks/structureBaseBlock.h>
#include <gameplay/crafting.h>
#include <audioEngine.h>
#include <iostream>
#include <magic_enum.hpp>
#include <safeSave.h>
#include <chunkSystem.h>
#include <sstream>
#include <iomanip>

float determineTextSize(gl2d::Renderer2D &renderer, const std::string &str,
	gl2d::Font &f, glm::vec4 transform, bool minimize = true)
{
	float size = 64;

	auto s = renderer.getTextSize(str.c_str(), f, size);

	float ratioX = transform.z / s.x;
	float ratioY = transform.w / s.y;


	if (ratioX > 1 && ratioY > 1)
	{

		///keep size
		//return size;

		//else
		//{
		//	if (ratioX > ratioY)
		//	{
		//		return size * ratioY;
		//	}
		//	else
		//	{
		//		return size * ratioX;
		//	}
		//}

	}
	else
	{
		if (ratioX < ratioY)
		{
			size *= ratioX;
		}
		else
		{
			size *= ratioY;
		}
	}

	if (minimize)
	{
		size *= 0.9;
	}
	else
	{
		size *= 0.9;
	}

	return size;
}

void renderTextIntoBox(gl2d::Renderer2D &renderer, const std::string &str,
	gl2d::Font &f, glm::vec4 transform, glm::vec4 color, bool minimize = true, bool alignLeft = false)
{
	if (!str.length()) { return; }

	auto newS = determineTextSize(renderer, str, f, transform, minimize);

	glm::vec2 pos = glm::vec2(transform);

	if (!alignLeft)
	{
		pos.x += transform.z / 2.f;
		pos.y += transform.w / 3.f;
		renderer.renderText(pos, str.c_str(), f, color, newS);
	}
	else
	{
		//pos.x += transform.z * 0.02;
		pos.y += transform.w * 0.4;
		renderer.renderText(pos, str.c_str(), f, color, newS, 4, 3, false);
	}

}

glm::vec4 shrinkRectanglePercentage(glm::vec4 in, float perc)
{
	float shrinkX = in.z * perc;
	float shrinkY = in.w * perc;

	in.x += shrinkX / 2.f;
	in.y += shrinkY / 2.f;

	in.z -= shrinkX;
	in.w -= shrinkY;

	return in;
}

glm::vec4 shrinkRectanglePercentage(glm::vec4 in, float percX, float percY)
{
	float shrinkX = in.z * percX;
	float shrinkY = in.w * percY;

	in.x += shrinkX / 2.f;
	in.y += shrinkY / 2.f;

	in.z -= shrinkX;
	in.w -= shrinkY;

	return in;
}

glm::vec4 shrinkRectanglePercentageMoveDown(glm::vec4 in, float perc)
{
	float shrinkX = in.z * perc;
	float shrinkY = in.w * perc;

	in.x += shrinkX / 2.f;
	in.y += shrinkY;

	in.z -= shrinkX;
	in.w -= shrinkY;

	return in;
}


void UiENgine::init()
{
	renderer2d.create();

	

}

void UiENgine::loadTextures(std::string path)
{
	if (!font.texture.id)
	{
		font.createFromFile((path + "font.ttf").c_str());
	}
	
	if(!uiTexture.id)
	uiTexture.loadFromFile((path + "ui0.png").c_str(), true, true);
	
	if(!buttonTexture.id)
	buttonTexture.loadFromFile((path + "button.png").c_str(), true, true);

	if (!coinIconTexture.id)
		loadFromFileAndAddPadding(coinIconTexture, (path + "icons/coin.png").c_str());

	if(!arrowIconTexture.id) loadFromFileAndAddPadding(arrowIconTexture, (path + "icons/arrow.png").c_str());
	if (!bootsIconTexture.id) loadFromFileAndAddPadding(bootsIconTexture, (path + "icons/boots.png").c_str());
	if (!chestPlateIconTexture.id) loadFromFileAndAddPadding(chestPlateIconTexture, (path + "icons/chestPlate.png").c_str());
	if (!helmetIconTexture.id) loadFromFileAndAddPadding(helmetIconTexture, (path + "icons/helmet.png").c_str());


	if (!itemsBar.id)
	{
		itemsBar.loadFromFile((path + "ui1.png").c_str(), true, true);
		itemsBarSize = itemsBar.GetSize();
	}

	if (!itemsHighlighter.id)
	{
		itemsHighlighter.loadFromFile((path + "ui2.png").c_str(), true, true);
		itemsHighlighterSize = itemsHighlighter.GetSize();
	}

	if (!background.id)
	{
		background.loadFromFile((path + "background.jpg").c_str(), true, true);
	}

	if (!vignete.id)
	{
		vignete.loadFromFile((path + "vignette.png").c_str(), true, true);
	}

	if (!itemsBarInventory.id)
	{
		itemsBarInventory.loadFromFile((path + "ui3.png").c_str(), true, true);
		itemsBarInventorySize = itemsBarInventory.GetSize();
	}

	if (!arrowUI.id)
	{
		arrowUI.loadFromFile((path + "arrow.png").c_str(), true, true);
	}

	if (!oneInventorySlot.id)
	{
		oneInventorySlot.loadFromFile((path + "ui4.png").c_str(), true, true);
		oneInventorySlotSize = oneInventorySlot.GetSize();
	}

	if (!playerCell.id)
	{
		playerCell.loadFromFile((path + "ui5.png").c_str(), true, true);
		playerCellSize = playerCell.GetSize();
	}

	std::string newPath = path + "battle/";
	for (int i = 0; i < battleTexturesCount; i++)
	{

		std::string n(magic_enum::enum_name((BattleTextures)i).substr());

		if (!battleTextures[i].id)
		{
			battleTextures[i].loadFromFile((newPath + n + ".png").c_str(), true, true);
		}
	}

	newPath = path + "effects/";
	for (int i = 0; i < Effects::Effects_Count; i++)
	{

		std::string n(magic_enum::enum_name((Effects::EffectsNames)i).substr());

		if (!effectsTexture[i].id)
		{
			effectsTexture[i].loadFromFile((newPath + n + ".png").c_str(), true, true);
		}
	}
}

void UiENgine::clearOnlyTextures()
{
	font.cleanup();
	uiTexture.cleanup();
	buttonTexture.cleanup();
	coinIconTexture.cleanup();
	arrowIconTexture.cleanup();
	bootsIconTexture.cleanup();
	chestPlateIconTexture.cleanup();
	helmetIconTexture.cleanup();

	itemsBar.cleanup();
	background.cleanup();
	itemsHighlighter.cleanup();
	vignete.cleanup();

	itemsBarInventory.cleanup();
	arrowUI.cleanup();
	itemsBarInventorySize = {};

	oneInventorySlot.cleanup();
	oneInventorySlotSize = {};

	playerCell.cleanup();
	playerCellSize = {};

	for (int i = 0; i < battleTexturesCount; i++)
	{
		battleTextures[i].cleanup();
	}

	for (int i = 0; i < Effects::Effects_Count; i++)
	{
		effectsTexture[i].cleanup();
	}

}

const int INVENTORY_TAB_DEFAULT = 0;
const int INVENTORY_TAB_CRAFTING = 1;
const int INVENTORY_TAB_BLOCKS = 2;
const int INVENTORY_TAB_ITEMS = 3;

//render ui renderui
void UiENgine::renderGameUI(float deltaTime, int w, int h
	, int itemSelected, PlayerInventory &inventory, BlocksLoader &blocksLoader,
	bool insideInventory, int &cursorItemIndex,
	bool insideCraftingTable, int &currentInventoryTab, bool isCreative,
	unsigned short &selectedItem, Life &playerHealth, ProgramData &programData, LocalPlayer &player
	, int &craftingSlider, int &outCraftingRecepieGlobalIndex, bool showUI
)
{

	outCraftingRecepieGlobalIndex = -1;
	if (!isCreative) 
	{
		if (currentInventoryTab == INVENTORY_TAB_BLOCKS || currentInventoryTab == INVENTORY_TAB_ITEMS)
		{
			currentInventoryTab = INVENTORY_TAB_DEFAULT;
		}

	}

	cursorItemIndex = -1;
	glm::vec4 cursorItemIndexBox = {};
	std::optional<Item> currentItemHovered = {};
	auto mousePos = platform::getRelMousePosition();

	auto renderOneItem = [&](glm::vec4 itemBox, Item &item, float in = 0, float color = 1, gl2d::Texture icon = {})
	{
		if (item.type == 0)
		{
			if (icon.id)
			{
				renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, -0.20), icon, {1,1,1,0.45});
			}

			return;
		}

		if (item.type < BlocksCount)
		{

			gl2d::Texture t = blocksLoader.blockUiTextures[item.type];
			float size = 0.2;

			if (t.id == 0)
			{
				t.id = blocksLoader.texturesIds[getGpuIdIndexForBlock(item.type, 0)];
				size = 0.25;
			}

			//we have a block
			renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, in + size), t, {color, color, color, 1});

		}
		else
		{
			//we have an item
			gl2d::Texture t;
			t.id = blocksLoader.texturesIdsItems[item.type - ItemsStartPoint];

			renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, in - 0.35), t, {color, color, color, 1});
		}

		if (item.counter != 1)
		{
			itemBox = shrinkRectanglePercentage(itemBox, in);
			itemBox.x += itemBox.z / 1.4;
			itemBox.y += itemBox.w / 1.4;

			std::string s = std::to_string(item.counter);
			if (item.counter < 10) { s = " " + s; }

			renderer2d.renderText({itemBox}, s.c_str(),
				font, {1,1,1,1}, 40 * (itemBox.z/100.f));

		}

	};


	if (w != 0 && h != 0)
	{

		glui::Frame f({0,0, w, h});

		if (insideInventory)
		{


			renderer2d.renderRectangle({0,0,w,h}, {0.1,0.1,0.1,0.05});

			float minDimenstion = std::min(w * 0.65f, h * 0.85f);

			float aspectIncrease = 0;

			if (currentInventoryTab == INVENTORY_TAB_ITEMS || currentInventoryTab == INVENTORY_TAB_BLOCKS)
			{
				aspectIncrease = 0.10;
			}
			aspectIncrease = 0.2;


			auto inventoryBox = glui::Box().xCenter().yCenter().yDimensionPixels(minDimenstion).
				xAspectRatio(1.f + aspectIncrease)();
			//{
			//	auto inventoryBox2 = glui::Box().xCenter().yCenter().xDimensionPixels(minDimenstion * 0.9f).
			//		yAspectRatio(1.f - aspectIncrease)();
			//
			//	if (inventoryBox2.w < inventoryBox.w)
			//	{
			//		inventoryBox = inventoryBox2;
			//	}
			//}


			//render inventory box
			renderer2d.render9Patch(inventoryBox,
				24, {1,1,1,0.5}, {}, 0.f, buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
			//renderer2d.renderRectangle(inventoryBox, {1,0,0,1});

			if (glui::aabb(inventoryBox, mousePos))
			{
				cursorItemIndex = -2;
			}


			int oneItemSize = 0;

			{
				glui::Frame insideUiCell(inventoryBox);


				{

					glui::Frame insideInventoryLeft(glui::Box().xLeft().yTop().
						yDimensionPercentage(1.f).xAspectRatio(1.f)());

					auto checkInsideOneElement = [&](glm::vec4 box, int i)
					{
						if (glui::aabb(box, mousePos))
						{
							cursorItemIndex = i;
							auto rez = inventory.getItemFromIndex(cursorItemIndex);
							if (rez)currentItemHovered = *rez;
							cursorItemIndexBox = box;
							renderer2d.renderRectangle(shrinkRectanglePercentage(box, (2.f / 22.f)),
								{0.7,0.7,0.7,0.5});
						}
					};

					auto checkInside = [&](int start, glm::vec4 box, int rowCount, bool horizontal)
					{
						auto itemBox = box;
						itemBox.z = itemBox.w;
						for (int i = start; i < start + rowCount; i++)
						{
							if (horizontal)
							{
								itemBox.x = box.x + itemBox.z * (i - start);
							}
							else
							{
								itemBox.y = box.y - itemBox.w * (i - start);
							}
							checkInsideOneElement(itemBox, i);
						}
					};

					auto checkInsideCreativeMenu = [&](int start, glm::vec4 box)
					{
						auto itemBox = box;
						itemBox.z = itemBox.w;
						for (int i = start; i < start + 9; i++)
						{
							if (!isItem(i) && !isBlock(i)) { continue; }

							itemBox.x = box.x + itemBox.z * (i - start);
							if (glui::aabb(itemBox, mousePos))
							{
								selectedItem = i;
								auto rez = inventory.getItemFromIndex(cursorItemIndex);
								if (rez)currentItemHovered = *rez;
								cursorItemIndexBox = itemBox;
								renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, (2.f / 22.f)),
									{0.7,0.7,0.7,0.5});
							}
						}
					};

					auto checkInsideCreativeMenuBlocks = [&](int start, glm::vec4 box)
					{
						auto itemBox = box;
						itemBox.z = itemBox.w;
						for (int i = start; i < start + 9; i++)
						{
							if (!isItem(i) && !isBlock(i)) { continue; }

							itemBox.x = box.x + itemBox.z * (i - start);
							if (glui::aabb(itemBox, mousePos))
							{
								selectedItem = getBlockReorder(i);
								currentItemHovered = Item(selectedItem);
								//auto rez = inventory.getItemFromIndex(cursorItemIndex);
								//if (rez)currentItem = *rez;
								cursorItemIndexBox = itemBox;
								renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, (2.f / 22.f)),
									{0.7,0.7,0.7,0.5});
							}
						}
					};


					auto checkInsideOneCell = [&](int start, glm::vec4 box)
					{
						auto itemBox = box;
						itemBox.z = itemBox.w;

						if (glui::aabb(itemBox, mousePos))
						{
							cursorItemIndex = start;
							auto rez = inventory.getItemFromIndex(cursorItemIndex);
							if (rez)currentItemHovered = *rez;
							cursorItemIndexBox = itemBox;
							renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, (2.f / 22.f)),
								{0.7,0.7,0.7,0.5});
						}
					};

					auto hotBarBox = glui::Box().xCenter().yBottomPerc(-0.05).xDimensionPercentage(0.9).
						yAspectRatio(itemsBarInventorySize.y / itemsBarInventorySize.x)();

					renderer2d.renderRectangle(hotBarBox, itemsBarInventory);
					oneItemSize = hotBarBox.w;
					float maxSmallFontSize = oneItemSize / 3;


					glm::ivec4 healthPotionBox = {};
					glm::ivec4 manaPotionBox = {};
					healthPotionBox = hotBarBox;
					healthPotionBox.z = oneItemSize;
					healthPotionBox.x += hotBarBox.z + oneItemSize * (2.f / 16.f);

					manaPotionBox = healthPotionBox;
					manaPotionBox.x += oneItemSize + oneItemSize * (2.f / 16.f);
					renderer2d.renderRectangle(healthPotionBox, oneInventorySlot);
					renderer2d.renderRectangle(manaPotionBox, oneInventorySlot);


					//render items
					auto renderItems = [&](int start, glm::ivec4 box, int rowCount, bool horizontal, gl2d::Texture icon = {})
					{
						auto itemBox = box;
						itemBox.z = itemBox.w;
						for (int i = start; i < start + rowCount; i++)
						{
							if (inventory.items[i].type)
							{
								if (horizontal)
								{
									itemBox.x = box.x + itemBox.z * (i - start);
								}
								else
								{
									itemBox.y = box.y - itemBox.w * (i - start);
								}
								renderOneItem(itemBox, inventory.items[i], 4.f / 22.f);
							}
							else if (icon.id)
							{
								if (horizontal)
								{
									itemBox.x = box.x + itemBox.z * (i - start);
								}
								else
								{
									itemBox.y = box.y - itemBox.w * (i - start);
								}
								renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, -0.20), icon, {1,1,1,0.45});
							}

						}
					};
		

					glm::vec4 tabBox = inventoryBox;
					tabBox.z = oneItemSize;
					tabBox.w = oneItemSize / 2;
					tabBox.x += oneItemSize / 4.f;
					tabBox.y -= oneItemSize / 2.f;

					const int BARS_COUNT = 7;

					//render side bar
					auto renderSideSlider = [&](glm::ivec4 box)
					{
						glm::vec4 barBox = box;
						barBox.y -= box.w * (BARS_COUNT - 1);
						barBox.x += box.z;
						barBox.z = barBox.w;
						barBox.w = box.w * (BARS_COUNT);

						//renderer2d.renderRectangle(barBox, Colors_Red);

						glm::ivec4 topBox = barBox;
						topBox.w = topBox.z;

						glm::ivec4 bottomBox = barBox;
						bottomBox.y += barBox.w - barBox.z;
						bottomBox.w = bottomBox.z;

						int slider = 0;

						if (glui::drawButton(renderer2d, topBox, Colors_White, "",
							font, buttonTexture, platform::getRelMousePosition(),
							platform::isLMouseHeld(), platform::isLMouseReleased()))
						{
							slider--;
						}

						if (glui::drawButton(renderer2d, bottomBox, Colors_White, "",
							font, buttonTexture, platform::getRelMousePosition(),
							platform::isLMouseHeld(), platform::isLMouseReleased()))
						{
							slider++;
						}

						slider -= platform::getScroll();

						return slider;
					};

					auto renderSmallTextOnTopOfCell = [&](glm::vec4 box, const char *text)
					{
						glm::vec4 textBox = box;
						textBox.y -= textBox.w/4;
						textBox.w /= 3;

						glui::renderText(renderer2d, text, font, textBox,
							Colors_White, true, false, false, maxSmallFontSize);
					};

					if (currentInventoryTab == INVENTORY_TAB_DEFAULT ||
						currentInventoryTab == INVENTORY_TAB_CRAFTING
						)
					{

						//upper part
						auto inventoryBars = glui::Box().xCenter().yBottomPerc(-0.17).xDimensionPercentage(0.9).
							yAspectRatio(itemsBarInventorySize.y / itemsBarInventorySize.x)();
						renderer2d.renderRectangle(inventoryBars, itemsBarInventory);

						auto inventoryBars2 = inventoryBars;
						inventoryBars2.y -= inventoryBars2.w;
						renderer2d.renderRectangle(inventoryBars2, itemsBarInventory);

						auto inventoryBars3 = inventoryBars2;
						inventoryBars3.y -= inventoryBars3.w;
						renderer2d.renderRectangle(inventoryBars3, itemsBarInventory);

						auto inventoryBars4 = inventoryBars3;
						inventoryBars4.y -= inventoryBars4.w;
						renderer2d.renderRectangle(inventoryBars4, itemsBarInventory);

						checkInside(9, inventoryBars, 9, true);
						checkInside(18, inventoryBars2, 9, true);
						checkInside(27, inventoryBars3, 9, true);
						checkInside(36, inventoryBars4, 9, true);

						//coins
						glm::ivec4 coinsBox[4] = {};
						glm::ivec4 arrowsBox[4] = {};

						arrowsBox[0] = inventoryBars;
						arrowsBox[0].z = arrowsBox[0].w;
						arrowsBox[0].x += inventoryBars.z + arrowsBox[0].z * (2.f/16.f);

						for (int i = 1; i < 4; i++)
						{
							arrowsBox[i] = arrowsBox[i - 1];
							arrowsBox[i].y -= arrowsBox[i].w;
						}



						for (int i = 0; i < 4; i++)
						{
							coinsBox[i] = arrowsBox[i];
							coinsBox[i].x += oneItemSize + oneItemSize * (2.f / 16.f);

							renderer2d.renderRectangle(coinsBox[i], oneInventorySlot);
							renderer2d.renderRectangle(arrowsBox[i], oneInventorySlot);
						}

						checkInside(PlayerInventory::COINS_START_INDEX, coinsBox[0], 4, false);
						checkInside(PlayerInventory::ARROWS_START_INDEX, arrowsBox[0], 4, false);


						renderSmallTextOnTopOfCell(coinsBox[3], "Coins");
						renderSmallTextOnTopOfCell(arrowsBox[3], "Ammo");

						//bottom part


						//crafting box
						if (currentInventoryTab == INVENTORY_TAB_CRAFTING)
						{
							auto allItems = getAllPossibleRecepies(inventory);


							glui::Frame insideUpperPart(glui::Box().xCenter().yTopPerc(0.1).
								xDimensionPercentage(0.90).yDimensionPercentage(0.45)());

							//highlight
							//renderer2d.renderRectangle(glui::Box().xLeft().yTop().xDimensionPercentage(1.f).
							//yDimensionPercentage(1.f)(), {1,0,0,0.5});

							auto craftingItems = glui::Box().xCenter().yTopPerc(0).xDimensionPercentage(1.f).
								yAspectRatio(itemsBarInventorySize.y / itemsBarInventorySize.x)();
							//renderer2d.renderRectangle(craftingItems, itemsBarInventory);

							craftingItems.y += craftingItems.w * 0.8;
							//craftingItems.w -= craftingItems.w;
							//craftingItems = shrinkRectanglePercentage(craftingItems, 0.05);

							auto border = craftingItems;
							border.w *= 2;
							border.z += craftingItems.w * 2;
							border = shrinkRectanglePercentage(border, -0.02, -0.2);
							renderer2d.render9Patch(border,
								24, Colors_White, {}, 0.f, buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

							if (allItems.size())
							{
								int currentPos = craftingSlider;
								craftingSlider -= platform::getScroll();
								int minVal = 0;
								if (allItems.size() <= 9)
								{
									minVal = -(allItems.size() - 2);
								}

								craftingSlider = glm::clamp(craftingSlider, -1, std::max((int)allItems.size() + 9 - 11, minVal));

								if (currentPos != craftingSlider && platform::getScroll())
								{
									AudioEngine::playSound(AudioEngine::uiSlider, UI_SOUND_VOLUME);
								}
							}

							if (allItems.size())
							{
								
								int start = craftingSlider;

								std::optional<int> currentIndexSelected;

								auto doOneRender = [&](int i, glm::vec4 color = Colors_White)
								{
									if (allItems.size() > i)
									{
										glm::ivec4 itemBox;
										if (i >= (start + 11))
										{
											itemBox = craftingItems;
											itemBox.z = itemBox.w;
											itemBox.x = craftingItems.x + itemBox.z * (i - start - 11);
											itemBox.y += itemBox.z * 1.1;
										}
										else
										{
											itemBox = craftingItems;
											itemBox.z = itemBox.w;
											itemBox.x = craftingItems.x + itemBox.z * (i - start);
										}

										int distance = glm::abs(1 - (i - start));
										distance = glm::min(distance, 1) * 3.f;

										renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, 0.0), oneInventorySlot, color);

										renderOneItem(itemBox, allItems[i].recepie.result, (float)distance / 22.f, 1.f - distance / 22.f);

										if (glui::aabb(itemBox, mousePos))
										{
											currentIndexSelected = i;
										}
									}
								};


								for (int i = start; i < start + 22; i++)
								{
									if (i != start + 1)
									{
										doOneRender(i, Colors_White);
									}
								}

								{
									auto itemBox = craftingItems;
									itemBox.z = itemBox.w;
									itemBox.x = craftingItems.x + itemBox.z * (1);
									//auto box = itemBox; box.x = craftingItems.x + itemBox.z * (1);
									//auto itemBox = box;
									//itemBox.z = itemBox.w;

									//renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, -0.3), oneInventorySlot);

									if (glui::aabb(itemBox, mousePos))
									{
										//cursorItemIndex = i;
										cursorItemIndexBox = itemBox;
										currentItemHovered = allItems[start + 1].recepie.result;
										renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, -0.3 + (0.3f / 4.f)),
											{0.7,0.7,0.7,0.5});

										//crafting selected
										if (allItems.size() > start + 1)
										{
											outCraftingRecepieGlobalIndex = allItems[start + 1].index;
										}
									}

								}

								doOneRender(start + 1, {0,1,1,1});

								if (currentIndexSelected)
								{
									if (platform::isLMousePressed() && *currentIndexSelected != start + 1)
									{
										craftingSlider -= (start + 1) - *currentIndexSelected;

										int minVal = 0;
										if (allItems.size() <= 7)
										{
											minVal = -(allItems.size() - 2);
										}
										craftingSlider = glm::clamp(craftingSlider, -1, std::max((int)allItems.size() + 7 - 9, minVal));
									}
								}

							}


							{

								auto craftingWindowBox = glui::Box().xCenter().yTopPerc(0.05).
									xDimensionPercentage(1).yDimensionPercentage(1)();
								craftingWindowBox.y -= craftingItems.w * 0.8f;
								//craftingWindowBox.w -= craftingItems.w;
								craftingWindowBox = shrinkRectanglePercentage(craftingWindowBox, 0.05);
								
								glui::Frame craftingWindow(craftingWindowBox);

								{
									//renderer2d.render9Patch(glui::Box().xLeft().yTop().xDimensionPercentage(1.f).yDimensionPercentage(1.f)(),
									//	24, {0.3,0.3,0.6,1}, {}, 0.f, buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

									//render recepie requests

									int index = craftingSlider + 1;

									if (allItems.size() > index)
									{
										auto craftingItems = glui::Box().xCenter().yTopPerc(0).xDimensionPercentage(1.f).
											yAspectRatio(itemsBarInventorySize.y / itemsBarInventorySize.x)();
										//renderer2d.renderRectangle(craftingItems, itemsBarInventory);
										auto &recepie = allItems[index];

										auto box = craftingItems;
										box.z = oneItemSize;
										box.w = oneItemSize;
										box.x += box.z * 0.8;

										box = shrinkRectanglePercentage(box, 0.05);

										for (int i = 0; i < sizeof(recepie.recepie.items) / sizeof(recepie.recepie.items[0]); i++)
										{

											if (recepie.recepie.items[i].type == 0)
											{
												break;
											}

											if (glui::aabb(box, mousePos))
											{
												//cursorItemIndex = i;
												cursorItemIndexBox = box;
												currentItemHovered = recepie.recepie.items[i];
											}

											renderer2d.renderRectangle(box, oneInventorySlot);
											renderOneItem(box, recepie.recepie.items[i]);


											box.x += box.z * 1.1;

										}

									}


								}
							}

							//arrow
							{
								auto itemBox = craftingItems;
								itemBox.z = itemBox.w;
								itemBox.x = craftingItems.x + itemBox.z * (1);
								itemBox.y -= itemBox.w * 0.35;

								renderer2d.renderRectangle(itemBox, arrowUI, Colors_White, {}, 180.f);
							}


						}


						//highlight
						//renderer2d.renderRectangle(glui::Box().xLeft().yTop().xDimensionPercentage(1.f).
						//	yDimensionPercentage(1.f)(), {1,0,0,0.5});

						//player stuff
						if (currentInventoryTab == INVENTORY_TAB_DEFAULT)
						{
							glui::Frame insideUpperPart(glui::Box().xCenter().yTopPerc(0.05).
								xDimensionPercentage(0.9).yDimensionPercentage(0.45)());

							auto armourBox = glui::Box().xLeft().yTopPerc(0.1).xDimensionPercentage(1.f / 9.f).
								yAspectRatio(1.f)();
							auto start = armourBox;
							glm::vec4 playerBox = armourBox;
							playerBox.w *= 3;
							playerBox.z = (playerBox.w / playerCellSize.y) * playerCellSize.x;

							//render player
							renderer2d.renderRectangle(playerBox, playerCell);

							//render armour stuff to the right
							armourBox = start;
							armourBox.x = playerBox.x + playerBox.z;
							renderer2d.renderRectangle(armourBox, oneInventorySlot);
							checkInsideOneElement(armourBox, inventory.ARMOUR_START_INDEX);
							renderOneItem(armourBox, inventory.headArmour, 0, 1, helmetIconTexture);

							renderSmallTextOnTopOfCell(armourBox, "Armour");

							armourBox.y += armourBox.w;
							renderer2d.renderRectangle(armourBox, oneInventorySlot);
							checkInsideOneElement(armourBox, inventory.ARMOUR_START_INDEX + 1);
							renderOneItem(armourBox, inventory.chestArmour, 0, 1, chestPlateIconTexture);

							armourBox.y += armourBox.w;
							renderer2d.renderRectangle(armourBox, oneInventorySlot);
							checkInsideOneElement(armourBox, inventory.ARMOUR_START_INDEX + 2);
							renderOneItem(armourBox, inventory.bootsArmour, 0, 1, bootsIconTexture);


						}


						renderItems(9, inventoryBars, 9, true);
						renderItems(18, inventoryBars2, 9, true);
						renderItems(27, inventoryBars3, 9, true);
						renderItems(36, inventoryBars4, 9, true);

						renderItems(PlayerInventory::COINS_START_INDEX, coinsBox[0], 4, false, coinIconTexture);
						renderItems(PlayerInventory::ARROWS_START_INDEX, arrowsBox[0], 4, false, arrowIconTexture);




					}
					else if (currentInventoryTab == INVENTORY_TAB_BLOCKS)
					{

						auto inventoryBars = glui::Box().xCenter().yBottomPerc(-0.17).xDimensionPercentage(0.9).
							yAspectRatio(itemsBarInventorySize.y / itemsBarInventorySize.x)();

						static int currentStartRow = 0;
						currentStartRow += renderSideSlider(inventoryBars);
						currentStartRow = glm::clamp(currentStartRow, 0,
							(((int)BlocksCount / 9) - BARS_COUNT) + 1);
						if (currentStartRow < 0) { currentStartRow = 0; }

						//render items
						auto renderCreativeBlocks = [&](int start, glm::ivec4 box)
						{
							auto itemBox = box;
							itemBox.z = itemBox.w;
							for (int i = start; i < start + 9; i++)
							{
								if (i < BlocksCount)
								{
									itemBox.x = box.x + itemBox.z * (i - start);
									renderOneItem(itemBox, Item(getBlockReorder(i)), 4.f / 22.f);
								}
							}
						};

						for (int i = 0; i < BARS_COUNT; i++)
						{
							renderer2d.renderRectangle(inventoryBars, itemsBarInventory);

							checkInsideCreativeMenuBlocks((6 - i) * 9 + 1 + currentStartRow * 9, inventoryBars);
							renderCreativeBlocks((6 - i) * 9 + 1 + currentStartRow * 9, inventoryBars);

							inventoryBars.y -= inventoryBars.w;
						}

					}
					else if (currentInventoryTab == INVENTORY_TAB_ITEMS)
					{

						//render items
						auto renderCreativeItems = [&](int start, glm::ivec4 box)
						{
							auto itemBox = box;
							itemBox.z = itemBox.w;
							for (int i = start; i < start + 9; i++)
							{
								if (i < lastItem)
								{
									itemBox.x = box.x + itemBox.z * (i - start);
									renderOneItem(itemBox, Item(i), 4.f / 22.f);
								}
							}
						};

						auto inventoryBars = glui::Box().xCenter().yBottomPerc(-0.17).xDimensionPercentage(0.9).
							yAspectRatio(itemsBarInventorySize.y / itemsBarInventorySize.x)();

						static int currentStartRow = 0;
						currentStartRow += renderSideSlider(inventoryBars);
						currentStartRow = glm::clamp(currentStartRow, 0,
							(((int)(lastItem - ItemsStartPoint) / 9) - BARS_COUNT) + 1);
						if (currentStartRow < 0) { currentStartRow = 0; }

						for (int i = 0; i < BARS_COUNT; i++)
						{
							renderer2d.renderRectangle(inventoryBars, itemsBarInventory);

							checkInsideCreativeMenu((6 - i) * 9 + ItemsStartPoint + currentStartRow * 9, inventoryBars);
							renderCreativeItems((6 - i) * 9 + ItemsStartPoint + currentStartRow * 9, inventoryBars);

							inventoryBars.y -= inventoryBars.w;
						}



					}

					checkInside(0, hotBarBox, 9, true);
					renderItems(0, hotBarBox, 9, true);

					checkInside(PlayerInventory::HEALTH_POTION_INDEX, healthPotionBox, 1, true);
					checkInside(PlayerInventory::MANA_POTION_INDEX, manaPotionBox, 1, true);

					renderItems(PlayerInventory::HEALTH_POTION_INDEX, healthPotionBox, 1, true);
					renderItems(PlayerInventory::MANA_POTION_INDEX, manaPotionBox, 1, true);

					renderSmallTextOnTopOfCell(healthPotionBox, "Health");
					renderSmallTextOnTopOfCell(manaPotionBox, "Mana");


					//if (isCreative)
					{

						int slotsCounter = 2;
						if (isCreative) { slotsCounter = 4; }

						GLuint textures[4] = {
							blocksLoader.texturesIdsItems[copperAxe - ItemsStartPoint],
							blocksLoader.texturesIds[getGpuIdIndexForBlock(craftingTable, 0)],
							blocksLoader.texturesIds[getGpuIdIndexForBlock(grassBlock, 0)],
							blocksLoader.texturesIdsItems[stick - ItemsStartPoint],
						};

						for (int i = 0; i < slotsCounter; i++)
						{
							glm::vec4 selected = {};
							if (i == currentInventoryTab)
							{
								selected = glm::vec4(0, 0, 0, 12);
							}

							glm::vec4 color = {0.8,0.8,0.8,1};

							if (i != currentInventoryTab && glui::aabb(tabBox, platform::getRelMousePosition()))
							{
								color = {1.2,1.2,1.2,1};
							}
							else if (i == currentInventoryTab)
							{
								color = {1,1,1,1};
							}


							renderer2d.render9Patch(tabBox + selected,
								24, color, {}, 0.f, buttonTexture,
								{0,1,1,0.5}, {0.2,0.8,0.8,0.5});


							if (glui::aabb(tabBox, platform::getRelMousePosition()) &&
								platform::isLMousePressed()
								)
							{
								currentInventoryTab = i;
							}

							{
								auto newBox = tabBox;

								gl2d::Texture t; t.id = textures[i];
								if (i == currentInventoryTab)
								{
									newBox.w *= 2;
									newBox = shrinkRectanglePercentage(newBox, 0.3);
									renderer2d.renderRectangle(newBox, t, Colors_White,
										{}, 0);
								}
								else
								{
									newBox = shrinkRectanglePercentageMoveDown(newBox, 0.3);
									renderer2d.renderRectangle(newBox, t, Colors_White,
										{}, 0, {0,1,1,0.5});
								}
							}


							tabBox.x += oneItemSize + oneItemSize * (0.1f);
						}

					}






				}

				if (aspectIncrease)
				{

					glui::Frame insideInventoryRight(glui::Box().xRight().yTop().
						yDimensionPercentage(1.f).xDimensionPercentage(aspectIncrease)());

					//renderer2d.renderRectangle(glui::Box().xLeft().yTop().yDimensionPercentage(1.f).
					//	xDimensionPercentage(1.f)(), {1,0,0,0.2});

				}


				//render held item
				glm::vec4 itemPos(mousePos.x - oneItemSize / 2.f, mousePos.y - oneItemSize / 2.f,
					oneItemSize, oneItemSize);
				renderOneItem(itemPos, inventory.heldInMouse, 0);

				//render hovered item stuff
				if (currentItemHovered && !inventory.heldInMouse.type)
				{

					auto item = currentItemHovered;

					if (item->type)
					{

						auto box = cursorItemIndexBox;

						box.x += box.z * 0.5;
						box.y += box.w * 0.8;
						box.w *= 1;
						box.z *= 1.8;

						renderer2d.render9Patch(box,
							24, {0.3,0.2,0.2,0.8}, {}, 0.f, buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

						std::string text = item->formatMetaDataToString();

						renderTextIntoBox(renderer2d, text, font, box, Colors_White, true, true);

					}

				}

			}

		#pragma region render effects
			{
				auto effectsBox = glui::Box().xLeftPerc(0.04).yCenter().yDimensionPixels(minDimenstion).
					xDimensionPercentage(0.18)();

				glui::Frame insideUiCell(effectsBox);

				//renderer2d.renderRectangle(effectsBox, {1,0,0,0.8});

				auto &effects = player.effects;

				int effectsCount = 0;
				for (int i = 0; i < Effects::Effects_Count; i++)
				{
					if (effects.allEffects[i].timerMs > 0)
					{
						effectsCount++;
					}
				}

				if (effectsCount)
				{
					float height = oneItemSize;

					height = std::min(height, effectsBox.w / (float)effectsCount);

					float currentPos = effectsBox.y;

					for (int i = 0; i < Effects::Effects_Count; i++)
					{
						if (effects.allEffects[i].timerMs > 0)
						{

							glm::vec4 box = glm::vec4(effectsBox.x, currentPos,
								effectsBox.z, oneItemSize);

							box = shrinkRectanglePercentage(box, 0.1);

							//renderer2d.render9Patch(box,
							//	24, {0.9,0.9,0.9,0.5}, {}, 0.f, buttonTexture,
							//	GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

							//render effect image
							glm::vec4 effectBox = box;
							effectBox.z = box.w;
							effectBox.x = box.x + box.z - effectBox.z;
							effectBox = shrinkRectanglePercentage(effectBox, 0.1);

							renderer2d.renderRectangle(effectBox, effectsTexture[i]);


							//render time
							int secconds = effects.allEffects[i].timerMs / 1000;
							int minutes = secconds / 60;
							secconds -= minutes * 60;

							glm::vec2 textPos;
							textPos.y = box.y + box.w / 2.f;
							textPos.x = box.x + (box.z) / 2.f;

							std::stringstream ss;
							ss << std::setw(2) << std::setfill('0') << minutes << ":"
								<< std::setw(2) << std::setfill('0') << secconds;

							renderer2d.renderText(textPos, ss.str().c_str(), font, Colors_White, 
								box.x /2.f);


							currentPos += oneItemSize;
						}
					}

				}
				
				

			}
		#pragma endregion

		}
		else if(showUI)
		{

			//effects
			{

				float size = std::min(w * 0.05, h * 0.05);

				glm::vec4 box = glm::vec4(size * 0.1, size * 0.1, size, size);
				auto &effects = player.effects;

				for (int i = 0; i < Effects::Effects_Count; i++)
				{
					if (effects.allEffects[i].timerMs > 0)
					{

						renderer2d.renderRectangle(box, effectsTexture[i]);

						//render time
						//int secconds = effects.allEffects[i].timerMs / 1000;
						//int minutes = secconds / 60;
						//secconds -= minutes * 60;
						//
						//glm::vec2 textPos;
						//textPos.y = box.y + box.w / 2.f;
						//textPos.x = box.x + (box.z) / 2.f;
						//
						//std::stringstream ss;
						//ss << std::setw(2) << std::setfill('0') << minutes << ":"
						//	<< std::setw(2) << std::setfill('0') << secconds;
						//
						//renderer2d.renderText(textPos, ss.str().c_str(), font, Colors_White,
						//	box.x / 2.f);

						box.x += size * 1.1f;
					}
				}

			}




			//cross
			renderer2d.renderRectangle(
				glui::Box().xCenter().yCenter().xDimensionPixels(30).yAspectRatio(1.f),
				uiTexture, Colors_White, {}, 0,
				uiAtlas.get(2, 0)
			);



			//items
			auto itemsBarBox = glui::Box().xCenter().yBottom().xDimensionPercentage(0.35)
				.yAspectRatio(itemsBarSize.y / itemsBarSize.x)();
			renderer2d.renderRectangle(itemsBarBox, itemsBar, Colors_White, {}, 0);

			float itemBoxTexelSize = itemsBarBox.z / itemsBarSize.x;
			float itemBoxAdvance = (itemsBarBox.z - itemBoxTexelSize * 2) / 9.f;


			//icons
			auto itemBox = itemsBarBox;
			itemBox.z = itemBox.w;
			for (int i = 0; i < 9; i++)
			{
				if (inventory.items[i].type)
				{
					itemBox.x = itemsBarBox.x + itemBoxAdvance * i;
					renderOneItem(itemBox, inventory.items[i]);
				}
			}

			auto selectedBox = itemsBarBox;
			selectedBox.z = selectedBox.w;
			selectedBox.x += itemBoxAdvance * itemSelected;
			renderer2d.renderRectangle(selectedBox, itemsHighlighter, Colors_White, {}, 0);

			if (!isCreative)
			{
				auto heartBox = itemsBarBox;
				heartBox.z = itemsBarBox.w / 2.5;
				heartBox.w = itemsBarBox.w / 2.5;
				heartBox.y -= heartBox.w;

				int background = 0;

				static Oscilator lifeFlash(0.1, 2);
				bool tookDamage = 0;

				int maxLife = playerHealth.maxLife / 10;
				int life = playerHealth.life / 10;
				int lastLife = player.lastLife.life / 10;

				if (player.justRecievedDamageTimer > 0)
				{
					player.justRecievedDamageTimer -= deltaTime;
					tookDamage = true;
					background = 1;
				}
				else if (player.justHealedTimer > 0)
				{
					player.justHealedTimer -= deltaTime;
					background = 2;
				}

				if (background == 1)
				{
					background = lifeFlash.currentFaze;
					lifeFlash.update(deltaTime);
				}
				else if (background == 2)
				{
					background = lifeFlash.currentFaze == 1 ? 2 : 0;
					lifeFlash.update(deltaTime);
				}else
				{
					lifeFlash.reset();
				}


				{
					auto heartBoxCopy = heartBox;
					for (int i = 0; i < maxLife; i += 2)
					{
						renderer2d.renderRectangle(heartBoxCopy, programData.heartsTexture, Colors_White, {}, 0,
							programData.heartsAtlas.get(background, 0));
						heartBoxCopy.x += heartBoxCopy.z;
					}
				}

				if (tookDamage)
				{
					auto heartBoxCopy = heartBox;
					for (int i = 0; i < maxLife; i += 2)
					{
						if (i < lastLife)
						{
							if (i + 1 >= lastLife)
							{
								renderer2d.renderRectangle(heartBoxCopy, programData.heartsTexture, {1,1,1,0.3}, {}, 0,
									programData.heartsAtlas.get(4, 0));
							}
							else
							{
								renderer2d.renderRectangle(heartBoxCopy, programData.heartsTexture, {1,1,1,0.3}, {}, 0,
									programData.heartsAtlas.get(3, 0));
							}
						}
						heartBoxCopy.x += heartBoxCopy.z;
					}
				}
				else
				{
					player.lastLife = player.life;
				}

				for (int i = 0; i < maxLife; i += 2)
				{
					

					if (i < life)
					{
						if (i + 1 >= life)
						{
							renderer2d.renderRectangle(heartBox, programData.heartsTexture, Colors_White, {}, 0,
								programData.heartsAtlas.get(4, 0));
						}
						else
						{
							renderer2d.renderRectangle(heartBox, programData.heartsTexture, Colors_White, {}, 0,
								programData.heartsAtlas.get(3, 0));
						}
					}

					heartBox.x += heartBox.z;
				}


			}

		}

	}


}

bool UiENgine::renderBaseBlockUI(float deltaTime, int w, int h,
	ProgramData &programData, BaseBlock &baseBlock, glm::ivec3 blockPos
	, ChunkSystem &chunkSystem, UndoQueue &undoQueue, LightSystem &lightSystem,
	ClientEntityManager &clientEntityManager)
{

	if (w != 0 && h != 0)
	{

		glui::Frame f({0,0, w, h});

		auto textBox = glui::Box().xCenter().yTopPerc(0.3).xDimensionPercentage(0.8).
			yDimensionPixels(150)();

		renderer2d.renderRectangle({0,0,w,h}, {0.1,0.1,0.1,0.1});



		//glui::renderTextInput(renderer2d, "", baseBlock.name,
		//	sizeof(baseBlock.name), platform::getTypedInput(), font,
		//	textBox, Colors_Gray, buttonTexture, false, true);

		menuRenderer.Begin(5391);
		menuRenderer.SetAlignModeFixedSizeWidgets({0,150});

		menuRenderer.InputText("Name:", baseBlock.name, sizeof(baseBlock.name), Colors_Gray,
			buttonTexture);


		menuRenderer.sliderint8("Offset X", &baseBlock.offsetX, -120, 120, Colors_White, buttonTexture, Colors_Gray, buttonTexture, Colors_White);
		menuRenderer.sliderint8("Offset Y", &baseBlock.offsetY, -120, 120, Colors_White, buttonTexture, Colors_Gray, buttonTexture, Colors_White);
		menuRenderer.sliderint8("Offset Z", &baseBlock.offsetZ, -120, 120, Colors_White, buttonTexture, Colors_Gray, buttonTexture, Colors_White);

		menuRenderer.sliderUint8("Size X", &baseBlock.sizeX, 0, 120, Colors_White, buttonTexture, Colors_Gray, buttonTexture, Colors_White);
		menuRenderer.sliderUint8("Size Y", &baseBlock.sizeY, 0, 120, Colors_White, buttonTexture, Colors_Gray, buttonTexture, Colors_White);
		menuRenderer.sliderUint8("Size Z", &baseBlock.sizeZ, 0, 120, Colors_White, buttonTexture, Colors_Gray, buttonTexture, Colors_White);

		bool rez = menuRenderer.Button("Update Settings", Colors_Gray, buttonTexture);
		
		bool save = menuRenderer.Button("Save Structure To Disk", Colors_Gray, buttonTexture);
		bool load = menuRenderer.Button("Load Structure from disk", Colors_Gray, buttonTexture);

		menuRenderer.End();

		glm::ivec3 startPos = blockPos + glm::ivec3(1,0,1) + glm::ivec3(baseBlock.offsetX, baseBlock.offsetY, baseBlock.offsetZ);
		glm::ivec3 size = glm::ivec3(baseBlock.sizeX, baseBlock.sizeY, baseBlock.sizeZ);

		std::string filePath = RESOURCES_PATH;
		filePath += "/gameData/structures/";
		filePath += baseBlock.name;
		filePath += ".structure";

		//todo make the save button red if changes!!!

		if (save)
		{
			std::vector<unsigned char> data;
			data.resize(sizeof(StructureData) + 2 * sizeof(BlockType) * size.x * size.y * size.z);
		
			StructureData *s = (StructureData *)data.data();
		
			s->sizeNotRotated = size;
			s->unused = 0;
		
			for (int x = 0; x < size.x; x++)
				for (int z = 0; z < size.z; z++)
					for (int y = 0; y < size.y; y++)
					{
						glm::ivec3 pos = startPos + glm::ivec3(x, y, z);
		
						auto rez = chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);
		
						if (rez)
						{
							s->unsafeGet(x, y, z) = *rez;
						}
						else
						{
							s->unsafeGet(x, y, z).setType(BlockTypes::air);
							s->unsafeGet(x, y, z).colorAndOtherFlags = 0;
						}
		
					}

			sfs::writeEntireFile(s, data.size(), filePath.c_str());
		}

		if (load)
		{
			std::vector<char> data;
			if (sfs::readEntireFile(data, filePath.c_str()) ==
				sfs::noError)
			{
				StructureData *s = (StructureData *)data.data();
				baseBlock.sizeX = s->sizeNotRotated.x;
				baseBlock.sizeY = s->sizeNotRotated.y;
				baseBlock.sizeZ = s->sizeNotRotated.z;

				for (int x = 0; x < s->sizeNotRotated.x; x++)
					for (int z = 0; z < s->sizeNotRotated.z; z++)
						for (int y = 0; y < s->sizeNotRotated.y; y++)
						{
							glm::ivec3 pos = startPos + glm::ivec3(x, y, z);
		
							Block block = s->unsafeGet(x, y, z);
							block.lightLevel = 0;

							//todo implement the bulk version...
							chunkSystem.placeBlockByClientForce(pos,
								block, undoQueue,lightSystem, clientEntityManager);
		
						}
		
			}
		
		}


		return rez || load || save;
	}

	return false;
}

void Oscilator::update(float deltaTime)
{
	currentTimer -= deltaTime;
	if (currentTimer < 0)
	{
		currentTimer = maxFazeTime;
		currentFaze++;

		if (currentFaze >= maxFazes)
		{
			currentFaze = 0;
		}
	}
}

void Oscilator::reset()
{
	currentTimer = 0;
	currentFaze = 0;
}
