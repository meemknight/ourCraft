#pragma once
#include "glm/vec4.hpp"
#include "glm/vec2.hpp"
#include <glui/glui.h>



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

	glm::vec2 itemsBarSize;
	glm::vec2 itemsHighlighterSize;


	void renderGameUI(float deltaTime, 
		int w, int h, int itemSelected);

};