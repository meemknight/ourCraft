#pragma once
#include "glm/vec4.hpp"
#include "glm/vec2.hpp"
#include <glui/glui.h>

struct PlayerInventory;
struct BlocksLoader;
struct Item;
struct CraftingTableInventory;
struct Life;
struct ProgramData;
struct LocalPlayer;
struct BaseBlock;

struct UiENgine
{

	gl2d::Renderer2D renderer2d;
	glui::RendererUi menuRenderer;

	void init();

	void loadTextures(std::string path);

	void clearOnlyTextures();

	gl2d::TextureAtlas uiAtlas{6, 1};


	gl2d::Font font;
	gl2d::Texture uiTexture;
	gl2d::Texture buttonTexture;

	gl2d::Texture itemsBar;
	gl2d::Texture itemsHighlighter;
	gl2d::Texture itemsBarInventory;
	gl2d::Texture oneInventorySlot;
	gl2d::Texture playerCell;

	glm::vec2 itemsBarSize;
	glm::vec2 itemsHighlighterSize;
	glm::vec2 itemsBarInventorySize;
	glm::vec2 oneInventorySlotSize;
	glm::vec2 playerCellSize;


	enum BattleTextures
	{
		circle,
		leftButton,
		rightButton,
		leftButtonFrontAttack,
		rightButtonSwipeAttack,

		battleTexturesCount
	};

	gl2d::Texture battleTextures[battleTexturesCount];



	//todo simplify
	//cursorItemIndex returns -1 if outside the menu to throw items, and -2 if it is nowhere 
	//if inside crafting, supply craftingTableInventory
	void renderGameUI(float deltaTime, 
		int w, int h, int itemSelected, PlayerInventory &inventory,
		BlocksLoader &blocksLoader, bool insideInventory, int &cursorItemIndex,
		bool insideCraftingTable, int &currentInventoryTab, bool isCreative,
		unsigned short &selectedItem, Life &playerHealth, ProgramData &programData,
		LocalPlayer &player, int &craftingSlider, int &outCraftingRecepieGlobalIndex,
		bool showUI
		);

	bool renderBaseBlockUI(float deltaTime,
		int w, int h, ProgramData &programData, 
		BaseBlock &baseBlock
		);

};


struct Oscilator
{

	Oscilator() {};
	Oscilator(float t, int fazes = 2): maxFazeTime(t), maxFazes(fazes) {};


	float maxFazeTime  = 0;
	int maxFazes = 0;

	float currentTimer = 0;
	int currentFaze = 0;

	void update(float deltaTime);

	void reset();
};