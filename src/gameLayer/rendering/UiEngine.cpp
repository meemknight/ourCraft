#include "rendering/UiEngine.h"
#include <platform/platformInput.h>
#include <gameplay/items.h>
#include <blocksLoader.h>

float determineTextSize(gl2d::Renderer2D &renderer, const std::string &str,
	gl2d::Font &f, glm::vec4 transform, bool minimize = true)
{
	float size = 4;

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


void UiENgine::init()
{
	renderer2d.create();

	font.createFromFile(RESOURCES_PATH "assets/roboto_black.ttf");
	uiTexture.loadFromFile(RESOURCES_PATH "assets/textures/ui/ui0.png", true, true);
	buttonTexture.loadFromFile(RESOURCES_PATH "assets/textures/ui/button.png", true, true);
	

	itemsBar.loadFromFile(RESOURCES_PATH "assets/textures/ui/ui1.png", true, true);
	itemsBarSize = itemsBar.GetSize();

	itemsHighlighter.loadFromFile(RESOURCES_PATH "assets/textures/ui/ui2.png", true, true);
	itemsHighlighterSize = itemsHighlighter.GetSize();

	itemsBarInventory.loadFromFile(RESOURCES_PATH "assets/textures/ui/ui3.png", true, true);
	itemsBarInventorySize = itemsBarInventory.GetSize();

	oneInventorySlot.loadFromFile(RESOURCES_PATH "assets/textures/ui/ui4.png", true, true);
	oneInventorySlotSize = oneInventorySlot.GetSize();

	playerCell.loadFromFile(RESOURCES_PATH "assets/textures/ui/ui5.png", true, true);
	playerCellSize = playerCell.GetSize();

}

void UiENgine::renderGameUI(float deltaTime, int w, int h
	, int itemSelected, PlayerInventory &inventory, BlocksLoader &blocksLoader,
	bool insideInventory, int &cursorItemIndex, Item &itemToCraft)
{
	cursorItemIndex = -1;
	glm::vec4 cursorItemIndexBox = {};
	auto mousePos = platform::getRelMousePosition();

	auto renderOneItem = [&](glm::vec4 itemBox, Item & item, float in = 8.f / 22.f)
	{
		if (item.type == 0)return;

		if (item.type < BlocksCount)
		{

			gl2d::Texture t;
			t.id = blocksLoader.texturesIds[getGpuIdIndexForBlock(item.type, 0)];

			//we have a block
			renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, in), t);


		}
		else
		{
			//we have an item
			gl2d::Texture t;
			t.id = blocksLoader.texturesIdsItems[item.type - ItemsStartPoint];

			//we have a block
			renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, in), t);
		}

		if (item.counter != 1)
		{
			itemBox = shrinkRectanglePercentage(itemBox, in);
			itemBox.x += itemBox.z / 1.4;
			itemBox.y += itemBox.w / 1.4;

			std::string s = std::to_string(item.counter);
			if (item.counter < 10) { s = " " + s; }

			renderer2d.renderText({itemBox}, s.c_str(),
				font, {1,1,1,1}, 0.9 * (itemBox.z/100.f));

		}

	};


	if (w != 0 && h != 0)
	{

		glui::Frame f({0,0, w, h});

		if (insideInventory)
		{

			renderer2d.renderRectangle({0,0,w,h}, {0.1,0.1,0.1,0.1});

			float minDimenstion = std::min(w, h);

			float aspectIncrease = 0.10;
			aspectIncrease = 0;

			auto inventoryBox = glui::Box().xCenter().yCenter().xDimensionPixels(minDimenstion * 0.9f).
				yAspectRatio(1.f - aspectIncrease)();

			//render inventory box
			renderer2d.render9Patch(inventoryBox,
				24, {1,1,1,1}, {}, 0.f, buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
			//renderer2d.renderRectangle(
			//	inventoryBox, buttonTexture);

			if (glui::aabb(inventoryBox, mousePos))
			{
				cursorItemIndex = -2;
			}

			int oneItemSize = 0;

			glui::Frame insideUiCell(inventoryBox);

			{

				glui::Frame insideInventoryLeft(glui::Box().xLeft().yTop().
					yDimensionPercentage(1.f).xAspectRatio(1.f)());

				auto checkInside = [&](int start, glm::vec4 box)
				{
					auto itemBox = box;
					itemBox.z = itemBox.w;
					for (int i = start; i < start + 9; i++)
					{
						itemBox.x = box.x + itemBox.z * (i - start);
						if (glui::aabb(itemBox, mousePos))
						{
							cursorItemIndex = i;
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
						cursorItemIndexBox = itemBox;
						renderer2d.renderRectangle(shrinkRectanglePercentage(itemBox, (2.f / 22.f)),
							{0.7,0.7,0.7,0.5});
					}
				};

				auto hotBarBox = glui::Box().xCenter().yBottomPerc(-0.05).xDimensionPercentage(0.9).
					yAspectRatio(itemsBarInventorySize.y / itemsBarInventorySize.x)();

				renderer2d.renderRectangle(hotBarBox, itemsBarInventory);

				auto inventoryBars = glui::Box().xCenter().yBottomPerc(-0.17).xDimensionPercentage(0.9).
					yAspectRatio(itemsBarInventorySize.y / itemsBarInventorySize.x)();
				renderer2d.renderRectangle(inventoryBars, itemsBarInventory);

				auto inventoryBars2 = inventoryBars;
				inventoryBars2.y -= inventoryBars2.w;
				renderer2d.renderRectangle(inventoryBars2, itemsBarInventory);

				auto inventoryBars3 = inventoryBars2;
				inventoryBars3.y -= inventoryBars3.w;
				renderer2d.renderRectangle(inventoryBars3, itemsBarInventory);

				checkInside(0, hotBarBox);
				checkInside(9, inventoryBars);
				checkInside(18, inventoryBars2);
				checkInside(27, inventoryBars3);

				//upper part
				{
					glui::Frame insideUpperPart(glui::Box().xCenter().yTopPerc(0.05).
						xDimensionPercentage(0.9).yDimensionPercentage(0.45)());
					
					//highlight
					//renderer2d.renderRectangle(glui::Box().xLeft().yTop().xDimensionPercentage(1.f).
					//	yDimensionPercentage(1.f)(), {1,0,0,0.5});

					//player stuff
					{
						auto armourBox = glui::Box().xLeft().yTopPerc(0.1).xDimensionPercentage(1.f / 9.f).
							yAspectRatio(1.f)();
						auto start = armourBox;
						glm::vec4 playerBox = armourBox;
						playerBox.x += playerBox.z;
						playerBox.w *= 4;
						playerBox.z = (playerBox.w / playerCellSize.y) * playerCellSize.x;

						//render armour stuff
						renderer2d.renderRectangle(armourBox, oneInventorySlot);

						armourBox.y += armourBox.w;
						renderer2d.renderRectangle(armourBox, oneInventorySlot);

						armourBox.y += armourBox.w;
						renderer2d.renderRectangle(armourBox, oneInventorySlot);

						armourBox.y += armourBox.w;
						renderer2d.renderRectangle(armourBox, oneInventorySlot);

						//render player
						renderer2d.renderRectangle(playerBox, playerCell);

						//render armour stuff to the right
						armourBox = start;
						armourBox.x = playerBox.x + playerBox.z;
						renderer2d.renderRectangle(armourBox, oneInventorySlot);

						armourBox.y += armourBox.w;
						renderer2d.renderRectangle(armourBox, oneInventorySlot);

						armourBox.y += armourBox.w;
						renderer2d.renderRectangle(armourBox, oneInventorySlot);

						armourBox.y += armourBox.w;
						renderer2d.renderRectangle(armourBox, oneInventorySlot);

					}


					//crafting
					{
						glm::vec4 craftingStart = glui::Box().xLeftPerc(0.56).yTopPerc(0.2).xDimensionPercentage(1.f / 9.f).
							yAspectRatio(1.f)();
						renderer2d.renderRectangle(craftingStart, oneInventorySlot);
						glm::vec4 secondCrafting = craftingStart; secondCrafting.x += craftingStart.z;
						renderer2d.renderRectangle(secondCrafting, oneInventorySlot);
						glm::vec4 thirdCrafting = craftingStart; thirdCrafting.y += craftingStart.w;
						renderer2d.renderRectangle(thirdCrafting, oneInventorySlot);
						glm::vec4 fourthCrafting = craftingStart; fourthCrafting.x += craftingStart.z; fourthCrafting.y += craftingStart.w;
						renderer2d.renderRectangle(fourthCrafting, oneInventorySlot);


						//result
						glm::vec4 resultCrafting = craftingStart; resultCrafting.x += craftingStart.z * 3; resultCrafting.y += craftingStart.w * 0.5;
						renderer2d.renderRectangle(resultCrafting, oneInventorySlot);


						checkInsideOneCell(PlayerInventory::CRAFTING_INDEX, craftingStart);
						checkInsideOneCell(PlayerInventory::CRAFTING_INDEX + 1, secondCrafting);
						checkInsideOneCell(PlayerInventory::CRAFTING_INDEX + 2, thirdCrafting);
						checkInsideOneCell(PlayerInventory::CRAFTING_INDEX + 3, fourthCrafting);

						renderOneItem(craftingStart, inventory.crafting[0], 4.f / 22.f);
						renderOneItem(secondCrafting, inventory.crafting[1], 4.f / 22.f);
						renderOneItem(thirdCrafting, inventory.crafting[2], 4.f / 22.f);
						renderOneItem(fourthCrafting, inventory.crafting[3], 4.f / 22.f);
						
						renderOneItem(resultCrafting, itemToCraft, 4.f / 22.f);
					
					}
					


				}

				//render items
				auto renderItems = [&](int start, glm::ivec4 box)
				{
					auto itemBox = box;
					itemBox.z = itemBox.w;
					for (int i = start; i < start+9; i++)
					{
						if (inventory.items[i].type)
						{
							itemBox.x = box.x + itemBox.z * (i-start);
							renderOneItem(itemBox, inventory.items[i], 4.f / 22.f);
						}
					}
				};

				oneItemSize = hotBarBox.w;
				
				renderItems(0, hotBarBox);
				renderItems(9, inventoryBars);
				renderItems(18, inventoryBars2);
				renderItems(27, inventoryBars3);

				

			}

			if(aspectIncrease)
			{

				glui::Frame insideInventoryRight(glui::Box().xRight().yTop().
					yDimensionPercentage(1.f).xDimensionPercentage(aspectIncrease)());

				renderer2d.renderRectangle(glui::Box().xLeft().yTop().yDimensionPercentage(1.f).
					xDimensionPercentage(1.f)(), {1,0,0,0.2});

			}


			//render held item
			glm::vec4 itemPos(mousePos.x - oneItemSize/2.f, mousePos.y - oneItemSize/2.f,
				oneItemSize, oneItemSize);
			renderOneItem(itemPos, inventory.heldInMouse, 0);

			//render hovered item stuff
			if (cursorItemIndex >= 0 && !inventory.heldInMouse.type)
			{

				auto item = inventory.getItemFromIndex(cursorItemIndex);

				if (item && item->type && item->metaData.size())
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
		else
		{
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
		}

	}


}
