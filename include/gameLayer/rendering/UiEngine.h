#pragma once
#include "glm/vec4.hpp"
#include "glm/vec2.hpp"
#include <glui/glui.h>

struct PlayerInventory;
struct BlocksLoader;


struct UiENgine
{

	gl2d::Renderer2D renderer2d;
	glui::RendererUi menuRenderer;

	void init();

	gl2d::TextureAtlas uiAtlas{6, 1};


	gl2d::Font font;
	gl2d::Texture uiTexture;
	gl2d::Texture buttonTexture;

	gl2d::Texture itemsBar;
	gl2d::Texture itemsHighlighter;
	gl2d::Texture itemsBarInventory;
	gl2d::Texture oneInventorySlot;

	glm::vec2 itemsBarSize;
	glm::vec2 itemsHighlighterSize;
	glm::vec2 itemsBarInventorySize;
	glm::vec2 oneInventorySlotSize;



	void renderGameUI(float deltaTime, 
		int w, int h, int itemSelected, PlayerInventory &inventory,
		BlocksLoader &blocksLoader, bool insideInventory);

};