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
#include <structure.h>
#include <safeSave.h>

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

	gameData.chunkSystem.createChunks(32);

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

	glm::ivec3 blockPositionPlayer = from3DPointToBlock(gameData.c.position);

	gameData.lightSystem.update(gameData.chunkSystem);

	{
		//static std::vector<int> data;

		gameData.c.decomposePosition(posFloat, posInt);
		
		programData.renderer.updateDynamicBlocks();
		
		gameData.chunkSystem.update(blockPositionPlayer, deltaTime, gameData.undoQueue,
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

	static glm::ivec3 point;
	static glm::ivec3 pointSize;
	static bool renderBox = 1;

	if (!gameData.escapePressed)
	{
		static int blockTypeToPlace = 1;
		if (platform::isKeyReleased(platform::Button::Z)) { blockTypeToPlace--; }
		if (platform::isKeyReleased(platform::Button::X)) { blockTypeToPlace++; }

		blockTypeToPlace = glm::clamp(blockTypeToPlace, 1, BlocksCount - 1);

		if (platform::isKeyHeld(platform::Button::LeftCtrl))
		{
			if (blockToPlace)
			{
				auto b = gameData.chunkSystem.getBlockSafe(rayCastPos);
				if (b)
				{
					blockTypeToPlace = b->type;
				}
			}
			
		}else
		if (platform::isKeyHeld(platform::Button::LeftAlt))
		{
			if (platform::isRMouseReleased())
			{
				if (blockToPlace)
				{
					point = *blockToPlace;
				}
			}
			else if (platform::isLMouseReleased())
			{
				if (blockToPlace)
				{
					pointSize = *blockToPlace - point;
				}
			}
		}
		else
		{
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
		}

		
	};



	

	programData.pointDebugRenderer.renderCubePoint(gameData.c, point);

	if (renderBox)
	{
		//programData.gyzmosRenderer.drawCube(from3DPointToBlock(point));
		programData.gyzmosRenderer.drawCube(point);

		if (pointSize != glm::ivec3{})
		{
			programData.gyzmosRenderer.drawCube(from3DPointToBlock(point + glm::ivec3(pointSize)));
			programData.gyzmosRenderer.drawCube(from3DPointToBlock(point + glm::ivec3(pointSize) - glm::ivec3(1,0,0)));
			programData.gyzmosRenderer.drawCube(from3DPointToBlock(point + glm::ivec3(pointSize) - glm::ivec3(0,1,0)));
			programData.gyzmosRenderer.drawCube(from3DPointToBlock(point + glm::ivec3(pointSize) - glm::ivec3(0,0,1)));
		}
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

			//ImGui::DragScalarN("Point pos", ImGuiDataType_Double, &point[0], 3, 1);
			ImGui::DragInt3("Point pos", &point[0]);
			ImGui::DragInt3("Point size",  &pointSize[0]);
			ImGui::Checkbox("Render Box", &renderBox);

			pointSize = glm::clamp(pointSize, glm::ivec3(0, 0, 0), glm::ivec3(64, 64, 64));

			auto b = gameData.chunkSystem.getBlockSafe(point);
			if (b) ImGui::Text("Box Light Value: %d", b->getSkyLight());


			ImGui::DragFloat("camera speed", &moveSpeed);
			ImGui::Text("fps: %d", programData.currentFps);
			if (ImGui::Button("Exit game"))
			{
				terminate = true;
			}


			ImGui::NewLine();
			
			static char fileBuff[256] = RESOURCES_PATH "gameData/structures/test.structure";
			ImGui::InputText("File:", fileBuff, sizeof(fileBuff));

			if (ImGui::Button("Save structure"))
			{
				std::vector<unsigned char> data;
				data.resize(sizeof(StructureData) + sizeof(BlockType) * pointSize.x * pointSize.y * pointSize.z);

				StructureData *s = (StructureData*)data.data();

				s->size = pointSize;
				s->unused = 0;
				
				for (int x = 0; x < pointSize.x; x++)
					for (int z = 0; z < pointSize.z; z++)
						for (int y = 0; y < pointSize.y; y++)
						{
							glm::ivec3 pos = point + glm::ivec3(x, y, z);

							auto rez = gameData.chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);

							if (rez)
							{
								s->unsafeGet(x, y, z) = rez->type;
							}
							else
							{
								s->unsafeGet(x, y, z) = BlockTypes::air;
							}

						}
				sfs::writeEntireFile(s, data.size(), fileBuff);
			}

			if (ImGui::Button("Load structure"))
			{
				std::vector<char> data;
				if (sfs::readEntireFile(data, fileBuff) ==
					sfs::noError)
				{
					StructureData *s = (StructureData *)data.data();

					for (int x = 0; x < s->size.x; x++)
						for (int z = 0; z < s->size.z; z++)
							for (int y = 0; y < s->size.y; y++)
							{
								glm::ivec3 pos = point + glm::ivec3(x, y, z);

								gameData.chunkSystem.placeBlockByClient(pos, s->unsafeGet(x, y, z),
									gameData.undoQueue, gameData.c.position, gameData.lightSystem);
							}

					pointSize = s->size;
				}


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