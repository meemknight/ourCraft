#include "rendering/UiEngine.h"
#include <platform/platformInput.h>




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

}

void UiENgine::renderGameUI(float deltaTime, int w, int h
	, int itemSelected)
{
	if (w != 0 && h != 0)
	{

		glui::Frame f({0,0, w, h});

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

		auto selectedBox = itemsBarBox;
		selectedBox.z = selectedBox.w;
		selectedBox.x += itemBoxAdvance * itemSelected;
		renderer2d.renderRectangle(selectedBox, itemsHighlighter, Colors_White, {}, 0);


	}


}
