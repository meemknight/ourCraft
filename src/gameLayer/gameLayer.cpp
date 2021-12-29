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
#include "threadStuff.h"
#include <thread>

#define GPU_ENGINE 0
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = GPU_ENGINE;
}

gl2d::Renderer2D renderer2d;
Renderer renderer;
GyzmosRenderer gyzmosRenderer;

gl2d::Font font;
gl2d::Texture texture;

struct GameData
{

	Camera c;

}gameData;

ChunkSystem chunkSystem;
int facesCount = 0;
float timeCounter = 0;
int frameCounter = 0;
int currentFps = 0;

bool initGame()
{
	enableReportGlErrors();
	
	gl2d::setVsync(false);
	renderer2d.create();
	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");
	texture.loadFromFile(RESOURCES_PATH "blocks.png");
	renderer.create();
	gyzmosRenderer.create();

	if(!platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData)))
	{
		gameData = GameData();
	}

	std::thread serverThread(serverFunction);
	serverThread.detach();

	std::vector<int> blockData;
	chunkSystem.createChunks(32, blockData);

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

#pragma region fps

	timeCounter += deltaTime;
	if(timeCounter >= 1.f)
	{
		timeCounter -= 1;
		currentFps = frameCounter;
		frameCounter = 0;
	}
	frameCounter++;

#pragma endregion


	static float moveSpeed = 500.f;

#pragma region input
	{
		float speed = moveSpeed * deltaTime;
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

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	gameData.c.decomposePosition(posFloat, posInt);
	static std::vector<int> data;

	chunkSystem.update(posInt.x, posInt.z, data);
	glNamedBufferData(renderer.vertexBuffer, sizeof(int) * data.size(), data.data(), GL_DYNAMIC_DRAW);
	facesCount = data.size() / 4;


	glBindVertexArray(renderer.vao);
	texture.bind(0);

	renderer.defaultShader.bind();

	auto mvp = gameData.c.getProjectionMatrix() * glm::lookAt({0,0,0}, gameData.c.viewDirection, gameData.c.up);

	//mvp[3][0] = 0.f;
	//mvp[3][1] = 0.f;
	//mvp[3][2] = 0.f;

	glUniformMatrix4fv(renderer.u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
	

	glUniform3fv(renderer.u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(renderer.u_positionInt, 1, &posInt[0]);
	glUniform1i(renderer.u_typesCount, 22);
	
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, facesCount);

	glBindVertexArray(0);
	
	gyzmosRenderer.drawCube(0,60,0);
	gyzmosRenderer.render(gameData.c, posInt, posFloat);


	ImGui::Begin("camera controll");
	ImGui::DragScalarN("camera pos", ImGuiDataType_Double, &gameData.c.position[0], 3, 0.01);
	ImGui::DragFloat("camera speed", &moveSpeed);
	ImGui::Text("fps: %d", currentFps);
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
