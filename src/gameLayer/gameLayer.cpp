#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "camera.h"
#include "errorReporting.h"
#include "renderer.h"

gl2d::Renderer2D renderer2d;
Renderer renderer;

gl2d::Font font;
gl2d::Texture texture;

struct GameData
{

	Camera c;

}gameData;


bool initGame()
{
	enableReportGlErrors();

	renderer2d.create();
	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");
	texture.loadFromFile(RESOURCES_PATH "test.jpg");
	renderer.create();

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
	w = platform::getWindowSizeX();
	h = platform::getWindowSizeY();

	renderer2d.updateWindowMetrics(w, h);
	renderer2d.clearScreen();

	gameData.c.aspectRatio = (float)w / h;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
#pragma endregion


#pragma region input
	{
		float speed = 4 * deltaTime;
		glm::vec3 moveDir = {};
		if (platform::isKeyHeld(platform::Button::Up)
			|| platform::isKeyHeld(platform::Button::W)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::Up].held
			)
		{
			moveDir.z -= speed;
		}
		if (platform::isKeyHeld(platform::Button::Down)
			|| platform::isKeyHeld(platform::Button::S)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::Down].held
			)
		{
			moveDir.z += speed;
		}
		if (platform::isKeyHeld(platform::Button::Left)
			|| platform::isKeyHeld(platform::Button::A)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::Left].held
			)
		{
			moveDir.x -= speed;

		}
		if (platform::isKeyHeld(platform::Button::Right)
			|| platform::isKeyHeld(platform::Button::D)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::Right].held
			)
		{
			moveDir.x += speed;
		}
		if (platform::isKeyHeld(platform::Button::Q)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::RBumper].held
			)
		{
			moveDir.y -= speed;
		}
		if (platform::isKeyHeld(platform::Button::E)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::LBumper].held
			)
		{
			moveDir.y += speed;
		}

		gameData.c.moveFPS(moveDir);

		if (platform::isKeyPressedOn(platform::Button::Enter))
		{
			platform::setFullScreen(!platform::isFullScreen());
		}

		gameData.c.rotateFPS(platform::getRelMousePosition(), 0.6f * deltaTime, platform::isRMouseHeld());

	}
#pragma endregion


	glBindVertexArray(renderer.vao);


	renderer.defaultShader.bind();
	glUniformMatrix4fv(renderer.u_viewProjection, 1, GL_FALSE, &gameData.c.getViewProjectionMatrix()[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);


#pragma region set finishing stuff
	renderer2d.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
