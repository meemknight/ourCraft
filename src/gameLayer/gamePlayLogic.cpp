#include "gamePlayLogic.h"
#include "rendering/camera.h"
#include "platformInput.h"
#include "errorReporting.h"
#include "rendering/renderer.h"
#include "chunkSystem.h"
#include "threadStuff.h"
#include <thread>
#include <ctime>
#include "multyPlayer/server.h"
#include "multyPlayer/createConnection.h"
#include <enet/enet.h>
#include "rendering/Ui.h"
#include "glui/glui.h"
#include <imgui.h>
#include <iostream>
#include "multyPlayer/undoQueue.h"
#include <platformTools.h>
#include <lightSystem.h>

struct GameData
{
	Camera c;
	ChunkSystem chunkSystem;
	bool escapePressed = 0;
	UndoQueue undoQueue;
	LightSystem lightSystem;

}gameData;


bool initGameplay(ProgramData &programData)
{
	if (!createConnection())
	{
		std::cout << "problem joining server\n";
		return false;
	}
	
	gameData = GameData();
	gameData.c.position = glm::vec3(0, 65, 0);

	gameData.chunkSystem.createChunks(64);

	return true;
}


bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData)
{

	gameData.c.aspectRatio = (float)w / h;

#pragma region server stuff
	{
		EventCounter validateEvent = 0;
		RevisionNumber inValidateRevision = 0;

		clientMessageLoop(validateEvent, inValidateRevision);

		if (validateEvent)
		{
			while (!gameData.undoQueue.events.empty())
			{
				if (validateEvent >= gameData.undoQueue.events[0].eventId.counter)
				{
					gameData.undoQueue.events.pop_front();
				}
				else
				{
					break;
				}
			}
		}

		if (inValidateRevision)
		{
			if (gameData.undoQueue.events.empty())
			{
				permaAssert(0); // undo queue is empty but I revieved an undo message.
			}

			RevisionNumber currentRevisionNumber = gameData.undoQueue.events[0].eventId.revision;
			for (auto &i : gameData.undoQueue.events)
			{
				if (i.eventId.revision != currentRevisionNumber)
				{
					permaAssert(0); // undo queue has inconsistent revisions
				}
			}

			if (inValidateRevision != gameData.undoQueue.currentEventId.revision)
			{
				permaAssert(0); //inconsistency between the server's revision and mine
			}

			for (int i = gameData.undoQueue.events.size() - 1; i >= 0; i--)
			{
				
				auto &e = gameData.undoQueue.events[i];

				if (e.type == Event::iPlacedBlock)
				{
					gameData.chunkSystem.placeBlockNoClient(e.blockPos, e.originalBlock, gameData.lightSystem);
				}
			}
			
			gameData.c.position = gameData.undoQueue.events[0].playerPos;

			gameData.undoQueue.events.clear();

			gameData.undoQueue.currentEventId.revision++;
		}
	}
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

		bool rotate = !gameData.escapePressed;
		if (platform::isRMouseHeld()) { rotate = true; }
		gameData.c.rotateFPS(platform::getRelMousePosition(), 0.3f * deltaTime, rotate);

		if (!gameData.escapePressed)
		{
			platform::setRelMousePosition(w / 2, h / 2);
			gameData.c.lastMousePos = {w / 2, h / 2};
		}

	}
#pragma endregion

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	programData.renderer.skyBoxRenderer.render(gameData.c);

	gameData.lightSystem.update(gameData.chunkSystem);

	{
		//static std::vector<int> data;

		gameData.c.decomposePosition(posFloat, posInt);
		
		programData.renderer.updateDynamicBlocks();
		
		gameData.chunkSystem.update(posInt.x, posInt.z, deltaTime, gameData.undoQueue,
			gameData.lightSystem);

		//programData.renderer.render(data, gameData.c, programData.texture);
		programData.renderer.renderFromBakedData(gameData.chunkSystem, gameData.c, programData.texture);

	}

	glm::ivec3 rayCastPos = {};
	std::optional<glm::ivec3> blockToPlace = std::nullopt;

	glm::dvec3 cameraRayPos = gameData.c.position;
	//if (gameData.c.position.x >= 0){cameraRayPos.x += 0.5;}else{cameraRayPos.x -= 0.5;}
	//if (gameData.c.position.z >= 0){cameraRayPos.z += 0.5;}else{cameraRayPos.z -= 0.5;}

	if (gameData.chunkSystem.rayCast(cameraRayPos, gameData.c.viewDirection, rayCastPos, 20, blockToPlace))
	{
		programData.gyzmosRenderer.drawCube(rayCastPos);

	}

	if (!gameData.escapePressed)
	{
		static int blockTypeToPlace = 1;
		if (platform::isKeyReleased(platform::Button::Z)) { blockTypeToPlace--; }
		if (platform::isKeyReleased(platform::Button::X)) { blockTypeToPlace++; }

		blockTypeToPlace = glm::clamp(blockTypeToPlace, 1, BlocksCount - 1);

		if (platform::isRMouseReleased())
		{
			if (blockToPlace)
				gameData.chunkSystem.placeBlockByClient(*blockToPlace, blockTypeToPlace,
				gameData.undoQueue, gameData.c.position, gameData.lightSystem);
		}
		else if (platform::isLMouseReleased())
		{
			gameData.chunkSystem.placeBlockByClient(rayCastPos, BlockTypes::air,
				gameData.undoQueue, gameData.c.position, gameData.lightSystem);
		}
	};



	static glm::dvec3 point;
	static bool renderBox = 1;

	programData.pointDebugRenderer.renderCubePoint(gameData.c, point);

	if (renderBox)
	{
		programData.gyzmosRenderer.drawCube(from3DPointToBlock(point));
	}


	programData.gyzmosRenderer.render(gameData.c, posInt, posFloat);


#pragma region imgui

	//if (gameData.escapePressed)
	{
		bool terminate = false;
		//if (ImGui::Begin("camera controll", &gameData.escapePressed))
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {26/255.f,26/255.f,26/255.f,0.5f});
		if (ImGui::Begin("camera controll"))
		{
			ImGui::DragScalarN("camera pos", ImGuiDataType_Double, &gameData.c.position[0], 3, 0.01);

			ImGui::Text("camera float: %f, %f, %f", posFloat.x, posFloat.y, posFloat.z);
			ImGui::Text("camera int: %d, %d, %d", posInt.x, posInt.y, posInt.z);

			ImGui::DragScalarN("Point pos", ImGuiDataType_Double, &point[0], 3, 1);
			ImGui::Checkbox("Render Box", &renderBox);

			auto b = gameData.chunkSystem.getBlockSafe(point);
			if (b) ImGui::Text("Box Light Value: %d", b->getSkyLight());


			ImGui::DragFloat("camera speed", &moveSpeed);
			ImGui::Text("fps: %d", programData.currentFps);
			if (ImGui::Button("Exit game"))
			{
				terminate = true;
			}
		}
		ImGui::End();
		ImGui::PopStyleColor();

		if (terminate)
		{
			return false;
		}
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

	for (auto &i : gameData.chunkSystem.loadedChunks)
	{
		delete i;
	}


	gameData = GameData(); //free all resources
}