#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>

gl2d::Renderer2D renderer;

gl2d::Font font;
gl2d::Texture texture;

struct GameData
{
	float posx=100;
	float posy=100;

	int test = 0;

}gameData;


bool initGame()
{
	renderer.create();
	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");
	texture.loadFromFile(RESOURCES_PATH "test.jpg");

	if(!platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData)))
	{
		gameData = GameData();
	}

	return true;
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w= platform::getWindowSizeX();
	h = platform::getWindowSizeY();
	
	renderer.updateWindowMetrics(w, h);
	renderer.clearScreen();
#pragma endregion


#pragma region input
	float speed = 400 * deltaTime;

	if(platform::isKeyHeld(platform::Button::Up) 
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Up].held
		)
	{
		gameData.posy -= speed;
	}
	if (platform::isKeyHeld(platform::Button::Down)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Down].held
		)
	{
		gameData.posy += speed;
	}
	if (platform::isKeyHeld(platform::Button::Left)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Left].held
		)
	{
		gameData.posx -= speed;
	}
	if (platform::isKeyHeld(platform::Button::Right)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Right].held
		)
	{
		gameData.posx += speed;
	}

	if (platform::isKeyTyped(platform::Button::NR1)
		)
	{
		gameData.test -= 1;
	}
	if (platform::isKeyTyped(platform::Button::NR2)
		)
	{
		gameData.test += 1;
	}


	if (platform::isKeyPressedOn(platform::Button::Enter))
	{
		platform::setFullScreen(!platform::isFullScreen());
	}
#pragma endregion

	glm::vec4 colors[4] = { Colors_Orange, Colors_Orange, Colors_Orange, Colors_Orange };

	{
		colors[0].r = platform::getControllerButtons().LT;
		colors[1].r = platform::getControllerButtons().RT;
		colors[2].r = platform::getControllerButtons().LStick.x;
		colors[3].r = platform::getControllerButtons().RStick.y;
	}

	renderer.renderRectangle({ 10,10, 100, 100 }, colors, {}, 30);

	
	renderer.renderRectangle({ gameData.posx,gameData.posy, 100, 100 }, { 0,0 }, 0, texture);

	renderer.renderText({10,200}, std::to_string(gameData.test).c_str(), font, Colors_White, 1.5, 4.0, 3, false);

	//ImGui::ShowDemoWindow();


#pragma region set finishing stuff
	renderer.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
