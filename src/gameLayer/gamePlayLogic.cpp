#include "gamePlayLogic.h"
#include "camera.h"
#include "platformInput.h"
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
#include "glui/glui.h"
#include "gamePlayLogic.h"
#include <imgui.h>
#include <iostream>

struct GameData
{
	Camera c;
	ChunkSystem chunkSystem;
	bool escapePressed = 0;

}gameData;


bool initGameplay(ProgramData &programData)
{
	if (!createConnection())
	{
		std::cout << "problem joining server\n";
		return false;
	}
	
	gameData = GameData();
	gameData.c.position = glm::vec3(100, 65, 100);

	gameData.chunkSystem.createChunks(32);

	return true;
}


bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData)
{

	gameData.c.aspectRatio = (float)w / h;


#pragma region server stuff

	clientMessageLoop();

#pragma endregion

#pragma region input

	static float moveSpeed = 10.f;

	if (platform::isKeyReleased(platform::Button::Escape))
	{
		gameData.escapePressed = !gameData.escapePressed;
	}

	platform::showMouse(gameData.escapePressed);

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

		if (platform::isKeyPressedOn(platform::Button::P))
		{
			platform::setFullScreen(!platform::isFullScreen());
		}

		bool rotate = !gameData.escapePressed;
		if (platform::isRMouseHeld()) { rotate = true; }
		gameData.c.rotateFPS(platform::getRelMousePosition(), 0.6f * deltaTime, rotate);

		if (!gameData.escapePressed)
		{
			platform::setRelMousePosition(w / 2, h / 2);
			gameData.c.lastMousePos = {w / 2, h / 2};
		}

	}
#pragma endregion

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};

	{
		gameData.c.decomposePosition(posFloat, posInt);
		static std::vector<int> data;

		programData.renderer.updateDynamicBlocks();

		gameData.chunkSystem.update(posInt.x, posInt.z, data, deltaTime);
		glNamedBufferData(programData.renderer.vertexBuffer, sizeof(int) * data.size(), data.data(), GL_DYNAMIC_DRAW);
		int facesCount = data.size() / 4;


		glBindVertexArray(programData.renderer.vao);
		programData.texture.bind(0);

		programData.renderer.defaultShader.bind();

		auto mvp = gameData.c.getProjectionMatrix() * glm::lookAt({0,0,0}, gameData.c.viewDirection, gameData.c.up);

		//mvp[3][0] = 0.f;
		//mvp[3][1] = 0.f;
		//mvp[3][2] = 0.f;

		glUniformMatrix4fv(programData.renderer.u_viewProjection, 1, GL_FALSE, &mvp[0][0]);

		glUniform3fv(programData.renderer.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(programData.renderer.u_positionInt, 1, &posInt[0]);
		glUniform1i(programData.renderer.u_typesCount, 22);
		glUniform1f(programData.renderer.u_time, std::clock() / 400.f);

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, facesCount);

		glBindVertexArray(0);
	}

	glm::ivec3 rayCastPos = {};
	std::optional<glm::ivec3> blockToPlace = std::nullopt;

	glm::dvec3 cameraRayPos = gameData.c.position;
	cameraRayPos.y += 0.5;
	cameraRayPos.x += 0.5;
	cameraRayPos.z += 0.5;
	//if (gameData.c.position.x >= 0){cameraRayPos.x += 0.5;}else{cameraRayPos.x -= 0.5;}
	//if (gameData.c.position.z >= 0){cameraRayPos.z += 0.5;}else{cameraRayPos.z -= 0.5;}

	if (gameData.chunkSystem.rayCast(cameraRayPos, gameData.c.viewDirection, rayCastPos, 5, blockToPlace))
	{
		programData.gyzmosRenderer.drawCube(rayCastPos);


	}

	if (platform::isRMouseReleased())
	{
		if (blockToPlace)
			gameData.chunkSystem.placeBlock(*blockToPlace, BlockTypes::stone);
	}
	else if (platform::isLMouseReleased())
	{
		gameData.chunkSystem.placeBlock(rayCastPos, BlockTypes::air);
	}


	programData.gyzmosRenderer.render(gameData.c, posInt, posFloat);

#pragma region imgui

	if (gameData.escapePressed)
	{
		if (ImGui::Begin("camera controll", &gameData.escapePressed))
		{
			ImGui::DragScalarN("camera pos", ImGuiDataType_Double, &gameData.c.position[0], 3, 0.01);
			ImGui::DragFloat("camera speed", &moveSpeed);
			ImGui::Text("fps: %d", programData.currentFps);
		}
		ImGui::End();
	}

#pragma endregion

#pragma region ui
	{
		Ui::Frame f({0,0, w, h});

		programData.renderer2d.renderRectangle(
			Ui::Box().xCenter().yCenter().xDimensionPixels(30).yAspectRatio(1.f), {}, 0,
			programData.uiTexture, programData.uiAtlas.get(2, 0)
		);

	}
#pragma endregion

	return true;
}

void closeGameLogic()
{

}