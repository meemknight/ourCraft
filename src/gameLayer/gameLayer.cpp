#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "camera.h"
#include "errorReporting.h"
#include "renderer.h"
#include "chunkSystem.h"

gl2d::Renderer2D renderer2d;
Renderer renderer;

gl2d::Font font;
gl2d::Texture texture;

struct GameData
{

	Camera c;

}gameData;

Chunk* chunk;
int facesCount = 0;

bool initGame()
{
	enableReportGlErrors();

	renderer2d.create();
	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");
	texture.loadFromFile(RESOURCES_PATH "blocks.png");
	renderer.create();

	if(!platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData)))
	{
		gameData = GameData();
	}

	chunk = new Chunk;

	chunk->create();
	std::vector<int> blockData;
	chunk->bake(blockData);

	glNamedBufferData(renderer.vertexBuffer, sizeof(int) * blockData.size(), blockData.data(), GL_DYNAMIC_DRAW);

	facesCount = blockData.size() / 4;

	return true;
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getWindowSizeX();
	h = platform::getWindowSizeY();

	renderer2d.updateWindowMetrics(w, h);

	gameData.c.aspectRatio = (float)w / h;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
	texture.bind(0);

	renderer.defaultShader.bind();

	auto mvp = gameData.c.getProjectionMatrix() * glm::lookAt({0,0,0}, gameData.c.viewDirection, gameData.c.up);

	//mvp[3][0] = 0.f;
	//mvp[3][1] = 0.f;
	//mvp[3][2] = 0.f;

	glUniformMatrix4fv(renderer.u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	gameData.c.decomposePosition(posFloat, posInt);

	glUniform3fv(renderer.u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(renderer.u_positionInt, 1, &posInt[0]);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, facesCount);

	glBindVertexArray(0);

	ImGui::Begin("camera controll");
	ImGui::DragScalarN("camera pos", ImGuiDataType_Double, &gameData.c.position[0], 3, 0.01);

	ImGui::End();

#pragma region set finishing stuff
	renderer2d.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
