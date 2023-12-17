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
#include <rendering/sunShadow.h>
#include <multiPlot.h>
#include <profiler.h>
#include <thread>

struct GameData
{
	Camera c;
	ChunkSystem chunkSystem;
	bool escapePressed = 0;
	bool showLightLevels = 0;
	UndoQueue undoQueue;
	LightSystem lightSystem;
	int skyLightIntensity = 15;

	SunShadow sunShadow;


	Profiler gameplayFrameProfiler;

	//debug stuff
	glm::ivec3 point = {};
	glm::ivec3 pointSize = {};//todo move
	bool renderBox = 1;

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

	gameData.chunkSystem.createChunks(16);

	gameData.sunShadow.init();

	return true;
}


bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData)
{
	gameData.gameplayFrameProfiler.endSubProfile("swap chain and others");

	gameData.c.aspectRatio = (float)w / h;
	glViewport(0, 0, w, h);


#pragma region server stuff
	{
		gameData.gameplayFrameProfiler.startSubProfile("server messages");

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

		//send player position
		{

			static float timer = 1;
			timer -= deltaTime;
			if (timer <= 0)
			{
				timer = 1;
					
				Packer_SendPlayerData data;
				data.playerData.position = gameData.c.position;
				data.playerData.chunkDistance = gameData.chunkSystem.squareSize;

				//todo enum for channel
				sendPacket(getServer(), formatPacket(headerSendPlayerData), (char*)&data, sizeof(data), 0, 1);
			}

		}

		gameData.gameplayFrameProfiler.endSubProfile("server messages");
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
		gameData.c.rotateFPS(platform::getRelMousePosition(), 0.25f * deltaTime, rotate);

		if (!gameData.escapePressed)
		{
			platform::setRelMousePosition(w / 2, h / 2);
			gameData.c.lastMousePos = {w / 2, h / 2};
		}

	}
#pragma endregion


#pragma region place blocks

	glm::ivec3 rayCastPos = {};
	std::optional<glm::ivec3> blockToPlace = std::nullopt;
	glm::dvec3 cameraRayPos = gameData.c.position;

	if (gameData.chunkSystem.rayCast(cameraRayPos, gameData.c.viewDirection, rayCastPos, 20, blockToPlace))
	{
		programData.gyzmosRenderer.drawCube(rayCastPos);

	}

	if (!gameData.escapePressed)
	{
		static int blockTypeToPlace = BlockTypes::glowstone;
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

		}
		else
			if (platform::isKeyHeld(platform::Button::LeftAlt))
			{
				if (platform::isRMouseReleased())
				{
					if (blockToPlace)
					{
						gameData.point = *blockToPlace;
					}
				}
				else if (platform::isLMouseReleased())
				{
					if (blockToPlace)
					{
						gameData.pointSize = *blockToPlace - gameData.point;
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

#pragma endregion

	
#pragma region get player positions and stuff

	glm::ivec3 blockPositionPlayer = from3DPointToBlock(gameData.c.position);
	bool underWater = 0;
	auto inBlock = gameData.chunkSystem.getBlockSafe(blockPositionPlayer);
	if (inBlock)
	{
		if (inBlock->type == BlockTypes::water)
		{
			underWater = 1;
		}
	}

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	gameData.c.decomposePosition(posFloat, posInt);

#pragma endregion

#pragma region update lights

	gameData.gameplayFrameProfiler.startSubProfile("lightsSystem");
	gameData.lightSystem.update(gameData.chunkSystem);
	gameData.gameplayFrameProfiler.endSubProfile("lightsSystem");

#pragma endregion


#pragma region chunks and rendering
	{

		//static std::vector<int> data;
		
		
		programData.renderer.updateDynamicBlocks();
		
		gameData.gameplayFrameProfiler.startSubProfile("chunkSystem");
		gameData.chunkSystem.update(blockPositionPlayer, deltaTime, gameData.undoQueue,
			gameData.lightSystem);
		gameData.gameplayFrameProfiler.endSubProfile("chunkSystem");



		gameData.gameplayFrameProfiler.startSubProfile("rendering");

		gameData.sunShadow.update();

		programData.renderer.renderShadow(gameData.sunShadow,
			gameData.chunkSystem, gameData.c, programData);

		gameData.sunShadow.renderShadowIntoTexture(gameData.c);

		//programData.renderer.render(data, gameData.c, programData.texture);
		programData.renderer.renderFromBakedData(gameData.sunShadow,gameData.chunkSystem, 
			gameData.c, programData, gameData.showLightLevels, 
			gameData.skyLightIntensity, gameData.point, underWater, w, h, deltaTime);

		gameData.gameplayFrameProfiler.endSubProfile("rendering");
	}
#pragma endregion

	
	
#pragma region debug and gyzmos stuff
	programData.pointDebugRenderer.renderCubePoint(gameData.c, gameData.point);

	if (gameData.renderBox)
	{
		//programData.gyzmosRenderer.drawCube(from3DPointToBlock(point));
		programData.gyzmosRenderer.drawCube(gameData.point);

		if (gameData.pointSize != glm::ivec3{})
		{
			programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize)));
			programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(1,0,0)));
			programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(0,1,0)));
			programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(0,0,1)));
		}
	}

	programData.gyzmosRenderer.render(gameData.c, posInt, posFloat);
#pragma endregion


#pragma region imgui

	//if (gameData.escapePressed)
	{
		gameData.gameplayFrameProfiler.startSubProfile("imgui");


		bool terminate = false;
		//if (ImGui::Begin("camera controll", &gameData.escapePressed))
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {26/255.f,26/255.f,26/255.f,0.5f});
		if (ImGui::Begin("client controll"))
		{
			ImGui::DragScalarN("camera pos", ImGuiDataType_Double, &gameData.c.position[0], 3, 0.01);

			ImGui::Text("camera float: %f, %f, %f", posFloat.x, posFloat.y, posFloat.z);
			ImGui::Text("camera int: %d, %d, %d", posInt.x, posInt.y, posInt.z);
			ImGui::Text("Chunk: %d, %d" , divideChunk(posInt.x), divideChunk(posInt.z));


			//ImGui::DragScalarN("Point pos", ImGuiDataType_Double, &point[0], 3, 1);
			ImGui::DragInt3("Point pos", &gameData.point[0]);
			ImGui::DragInt3("Point size",  &gameData.pointSize[0]);
			ImGui::Checkbox("Render Box", &gameData.renderBox);
			gameData.pointSize = glm::clamp(gameData.pointSize, glm::ivec3(0, 0, 0), glm::ivec3(64, 64, 64));

			if (ImGui::CollapsingHeader("Light Stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Checkbox("showLightLevels", &gameData.showLightLevels);
				ImGui::SliderInt("skyLightIntensity", &gameData.skyLightIntensity, 0, 15);
				ImGui::SliderFloat("metallic", &programData.renderer.metallic, 0, 1);
				ImGui::SliderFloat("roughness", &programData.renderer.roughness, 0, 1);
				ImGui::SliderFloat("exposure", &programData.renderer.exposure, 0.001, 10);

				ImGui::SliderFloat3("Sky pos", &programData.renderer.skyBoxRenderer.sunPos[0], -1, 1);

				ImGui::ColorPicker3("Underwater color", &programData.renderer.skyBoxRenderer.waterColor[0]);

				if (glm::length(programData.renderer.skyBoxRenderer.sunPos[0]) != 0)
				{
					programData.renderer.skyBoxRenderer.sunPos = glm::normalize(programData.renderer.skyBoxRenderer.sunPos);
				}
				else
				{
					programData.renderer.skyBoxRenderer.sunPos = glm::vec3(0, -1, 0);
				}
			}

			auto b = gameData.chunkSystem.getBlockSafe(gameData.point);
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

			if (ImGui::CollapsingHeader("Load Save Stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				if (ImGui::Button("Save structure"))
				{
					std::vector<unsigned char> data;
					data.resize(sizeof(StructureData) + sizeof(BlockType) * gameData.pointSize.x * gameData.pointSize.y * gameData.pointSize.z);

					StructureData *s = (StructureData *)data.data();

					s->size = gameData.pointSize;
					s->unused = 0;

					for (int x = 0; x < gameData.pointSize.x; x++)
						for (int z = 0; z < gameData.pointSize.z; z++)
							for (int y = 0; y < gameData.pointSize.y; y++)
							{
								glm::ivec3 pos = gameData.point + glm::ivec3(x, y, z);

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
									glm::ivec3 pos = gameData.point + glm::ivec3(x, y, z);

									gameData.chunkSystem.placeBlockByClient(pos, s->unsafeGet(x, y, z),
										gameData.undoQueue, gameData.c.position, gameData.lightSystem);
								}

						gameData.pointSize = s->size;
					}


				}
			}


			ImGui::NewLine();

			if (ImGui::CollapsingHeader("Sun Shadow Map",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)gameData.sunShadow.shadowTexturePreview.color, {256, 256}, 
					{0, 1}, {1, 0});
			}

			ImGui::NewLine();

			if (ImGui::CollapsingHeader("Screen space pos",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboMain.secondaryColor, {256, 256},
					{0, 1}, {1, 0});
			}

		}
		ImGui::End();


		if (ImGui::Begin("Profiler"))
		{
			ImGui::Text("profiler");
			ImGui::Text("fps: %d", programData.currentFps);

			gameData.gameplayFrameProfiler.displayPlot("Gameplay Frame");

		}
		ImGui::End();

		ImGui::PopStyleColor();

		if (terminate)
		{
			return false;
		}

		gameData.gameplayFrameProfiler.endSubProfile("imgui");
	}

#pragma endregion


#pragma region ui
	if(1)
	{	
		gameData.gameplayFrameProfiler.startSubProfile("ui");

		Ui::Frame f({0,0, w, h});

		programData.renderer2d.renderRectangle(
			Ui::Box().xCenter().yCenter().xDimensionPixels(30).yAspectRatio(1.f), {}, 0,
			programData.uiTexture, programData.uiAtlas.get(2, 0)
		);
		gameData.gameplayFrameProfiler.endSubProfile("ui");
	}
#pragma endregion

	
	gameData.gameplayFrameProfiler.endFrame();
	gameData.gameplayFrameProfiler.startFrame();
	gameData.gameplayFrameProfiler.startSubProfile("swap chain and others");


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