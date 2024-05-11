#include "rendering/UiEngine.h"
#include <platform/platformInput.h>
#include <gameplay/items.h>
#include <blocksLoader.h>



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
	bool insideInventory, int &cursorItemIndex)
{
	cursorItemIndex = -2;
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

			auto inventoryBox = glui::Box().xCenter().yCenter().xDimensionPixels(minDimenstion / 1.5f).
				yAspectRatio(1.f)();

			//render inventory box
			renderer2d.render9Patch(inventoryBox,
				24, {1,1,1,1}, {}, 0.f, buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
			//renderer2d.renderRectangle(
			//	inventoryBox, buttonTexture);

			if (glui::aabb(inventoryBox, mousePos))
			{
				cursorItemIndex = -1;
			}

			int oneItemSize = 0;

			{

				auto checkInside = [&](int start, glm::ivec4 box)
				{
					auto itemBox = box;
					itemBox.z = itemBox.w;
					for (int i = start; i < start + 9; i++)
					{
						itemBox.x = box.x + itemBox.z * (i - start);
						if (glui::aabb(itemBox, mousePos))
						{
							cursorItemIndex = i;
						}
					}
				};


				glui::Frame insideInventory(inventoryBox);


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

					auto armourBox = glui::Box().xLeft().yTopPerc(0.1).xDimensionPercentage(1.f / 9.f).
						yAspectRatio(1.f)();
					glm::vec4 playerBox = armourBox;
					playerBox.x += playerBox.z;
					playerBox.w *= 4;
					playerBox.z = (playerBox.w / playerCellSize.y) * playerCellSize.x;

					renderer2d.renderRectangle(armourBox, oneInventorySlot);

					armourBox.y += armourBox.w;
					renderer2d.renderRectangle(armourBox, oneInventorySlot);

					armourBox.y += armourBox.w;
					renderer2d.renderRectangle(armourBox, oneInventorySlot);

					armourBox.y += armourBox.w;
					renderer2d.renderRectangle(armourBox, oneInventorySlot);

					renderer2d.renderRectangle(playerBox, playerCell);



					//crafting
					auto craftingStart = glui::Box().xLeftPerc(0.6).yTopPerc(0.2).xDimensionPercentage(1.f / 9.f).
						yAspectRatio(1.f)();
					renderer2d.renderRectangle(craftingStart, oneInventorySlot);
					auto secondCrafting = craftingStart; secondCrafting.x += craftingStart.z;
					renderer2d.renderRectangle(secondCrafting, oneInventorySlot);
					auto thirdCrafting = craftingStart; thirdCrafting.y += craftingStart.w;
					renderer2d.renderRectangle(thirdCrafting, oneInventorySlot);
					auto fourthCrafting = craftingStart; fourthCrafting.x += craftingStart.z; fourthCrafting.y += craftingStart.w;
					renderer2d.renderRectangle(fourthCrafting, oneInventorySlot);



				

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

			glm::vec4 itemPos(mousePos.x - oneItemSize/2.f, mousePos.y - oneItemSize/2.f,
				oneItemSize, oneItemSize);
			renderOneItem(itemPos, inventory.heldInMouse, 0);


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
