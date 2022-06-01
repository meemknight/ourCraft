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
#include <ctime>
#include "server.h"
#include "createConnection.h"
#include <enet/enet.h>
#include "Ui.h"

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
gl2d::Texture uiTexture;
gl2d::TextureAtlas uiAtlas{6, 1};

struct GameData
{

	Camera c;

}gameData;

ChunkSystem chunkSystem;
int facesCount = 0;
float timeCounter = 0;
int frameCounter = 0;
int currentFps = 0;
bool escapePressed = 0;

bool initGame()
{
	enableReportGlErrors();
	
	gl2d::setVsync(false);
	renderer2d.create();
	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");
	texture.loadFromFile(RESOURCES_PATH "blocks.png");
	uiTexture.loadFromFile(RESOURCES_PATH "ui0.png");

	renderer.create();
	gyzmosRenderer.create();

	if(!platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData)))
	{
		gameData = GameData();
	}

	//todo create error function
	if (enet_initialize() != 0)
	{
		std::cout << "problem starting enet\n";
		return false;
	}

	if (!startServer())
	{
		std::cout << "problem starting server\n";
		return false;
	}

	if (!createConnection())
	{
		std::cout << "problem joining server\n";
		return false;
	}

	std::vector<int> blockData;
	chunkSystem.createChunks(32, blockData);

	glNamedBufferData(renderer.vertexBuffer, sizeof(int) * blockData.size(), blockData.data(), GL_DYNAMIC_DRAW);
	facesCount = blockData.size() / 4;

	glEnable(GL_LINE_WIDTH);
	glLineWidth(4);

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

#pragma region server stuff

	clientMessageLoop();

#pragma endregion

#pragma region input

	static float moveSpeed = 10.f;

	if (platform::isKeyReleased(platform::Button::Escape))
	{
		escapePressed = !escapePressed;
		std::cout << "esc\n";
	}

	platform::showMouse(escapePressed);

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

		bool rotate = !escapePressed;
		if (platform::isRMouseHeld()){rotate = true; }
		gameData.c.rotateFPS(platform::getRelMousePosition(), 0.6f * deltaTime, rotate);

		if (!escapePressed)
		{
			platform::setRelMousePosition(w / 2, h / 2);
			gameData.c.lastMousePos = {w / 2, h / 2};
		}

	}
#pragma endregion

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	gameData.c.decomposePosition(posFloat, posInt);
	static std::vector<int> data;

	renderer.updateDynamicBlocks();

	chunkSystem.update(posInt.x, posInt.z, data, deltaTime);
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
	glUniform1f(renderer.u_time, std::clock() / 400.f);
	
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, facesCount);

	glBindVertexArray(0);
	
	glm::ivec3 rayCastPos = {};
	std::optional<glm::ivec3> blockToPlace = std::nullopt;

	glm::dvec3 cameraRayPos = gameData.c.position;
	cameraRayPos.y += 0.5;
	cameraRayPos.x += 0.5;
	cameraRayPos.z += 0.5;
	//if (gameData.c.position.x >= 0){cameraRayPos.x += 0.5;}else{cameraRayPos.x -= 0.5;}
	//if (gameData.c.position.z >= 0){cameraRayPos.z += 0.5;}else{cameraRayPos.z -= 0.5;}

	if (chunkSystem.rayCast(cameraRayPos, gameData.c.viewDirection, rayCastPos, 5, blockToPlace))
	{
		gyzmosRenderer.drawCube(rayCastPos);

		
	}

	if (platform::isRMouseReleased())
	{
		if (blockToPlace)
			chunkSystem.placeBlock(*blockToPlace, BlockTypes::stone);
	}
	else if (platform::isLMouseReleased())
	{
		chunkSystem.placeBlock(rayCastPos, BlockTypes::air);
	}

	
	gyzmosRenderer.render(gameData.c, posInt, posFloat);

#pragma region imgui

	if (escapePressed && ImGui::Begin("camera controll", &escapePressed))
	{
		ImGui::DragScalarN("camera pos", ImGuiDataType_Double, &gameData.c.position[0], 3, 0.01);
		ImGui::DragFloat("camera speed", &moveSpeed);
		ImGui::Text("fps: %d", currentFps);
	}
	ImGui::End();
#pragma endregion

#pragma region ui
	{
		Ui::Frame f({0,0, w, h});

		renderer2d.renderRectangle(
			Ui::Box().xCenter().yCenter().xDimensionPixels(30).yAspectRatio(1.f), {}, 0,
			uiTexture, uiAtlas.get(2, 0)
		);

	}
#pragma endregion


#pragma region set finishing stuff
	gl2d::enableNecessaryGLFeatures();
	renderer2d.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
