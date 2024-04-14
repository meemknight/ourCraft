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
#include <gameplay/physics.h>
#include <gameplay/entityManagerClient.h>

struct GameData
{
	Camera c;
	ChunkSystem chunkSystem;
	bool escapePressed = 0;
	bool showImgui = 1;
	bool showLightLevels = 0;
	UndoQueue undoQueue;
	LightSystem lightSystem;
	int skyLightIntensity = 15;

	SunShadow sunShadow;

	Profiler gameplayFrameProfiler;

	//debug stuff
	glm::ivec3 point = {};
	glm::ivec3 pointSize = {};//todo move
	glm::dvec3 entityTest = {-4, 113, 3};
	bool renderBox = 0;
	bool renderPlayerPos = 0;
	
	bool colidable = 1;

	ClientEntityManager entityManager;

	std::uint64_t serverTimer = 0;
	float serverTimerCounter = 0;

	glm::dvec3 lastSendPos = {-INFINITY,0,0};



}gameData;


bool initGameplay(ProgramData &programData, const char *c)
{

	Packet_ReceiveCIDAndData playerData;

	if (!createConnection(playerData, c))
	{
		reportError("Problem joining server");
		return false;
	}
	
	gameData = GameData();
	gameData.c.position = glm::vec3(0, 65, 0);

	gameData.entityManager.localPlayer.body.pos = playerData.playersPosition;
	gameData.entityManager.localPlayer.body.lastPos = playerData.playersPosition;
	gameData.entityManager.localPlayer.entityId = playerData.yourPlayerEntityId;


	gameData.chunkSystem.init(30);

	gameData.sunShadow.init();

	return true;
}


bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData)
{
	gameData.gameplayFrameProfiler.endSubProfile("swap chain and others");

	if (h != 0)
	{
		gameData.c.aspectRatio = (float)w / h;
	}
	glViewport(0, 0, w, h);


#pragma region server stuff
	{
		gameData.gameplayFrameProfiler.startSubProfile("server messages");

		EventCounter validateEvent = 0;
		RevisionNumber inValidateRevision = 0;
		bool disconnect = 0;

		//todo the server should sent only one validate event message that is the latest, if possible
		//todo timeout maybe? if the server doesn't give you stuff.
		clientMessageLoop(validateEvent, inValidateRevision,
			gameData.entityManager.localPlayer.body.pos, gameData.chunkSystem.squareSize,
			gameData.entityManager, gameData.undoQueue, gameData.serverTimer, disconnect);

		if (disconnect) { return 0; }

		//todo timeout here? and request the server for a hard reset?
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
				permaAssert(0 && "inconsistency between the server's revision and mine"); //inconsistency between the server's revision and mine
			}

			for (int i = gameData.undoQueue.events.size() - 1; i >= 0; i--)
			{

				auto &e = gameData.undoQueue.events[i];

				if (e.type == Event::iPlacedBlock)
				{
					gameData.chunkSystem.placeBlockNoClient(e.blockPos, e.originalBlock, gameData.lightSystem);
				}
				else if (e.type == Event::iDroppedItemFromInventory)
				{
					gameData.entityManager.removeDroppedItem(e.entityId);
				}


			}

			gameData.entityManager.localPlayer.body.pos = gameData.undoQueue.events[0].playerPos;
			gameData.entityManager.localPlayer.body.lastPos = gameData.undoQueue.events[0].playerPos;

			gameData.undoQueue.events.clear();

			gameData.undoQueue.currentEventId.revision++;
		}


		{

			static float timer = 0.016;

			if (gameData.entityManager.localPlayer.body.pos != gameData.lastSendPos)
			{
				timer -= deltaTime;
			}
			else
			{
				timer -= deltaTime * 0.1f;
			}

			if (timer <= 0)
			{
				timer = 0.016 * 3.f;
				//timer = 0.316;

				Packer_SendPlayerData data;
				data.playerData.position = gameData.entityManager.localPlayer.body.pos;
				data.playerData.chunkDistance = gameData.chunkSystem.squareSize;
				data.timer = gameData.serverTimer;

				sendPacket(getServer(),
					formatPacket(headerSendPlayerData), (char *)&data, sizeof(data), 0,
					channelPlayerPositions);

				gameData.lastSendPos = gameData.entityManager.localPlayer.body.pos;
			}



		}

		gameData.gameplayFrameProfiler.endSubProfile("server messages");
	}
#pragma endregion

#pragma region timer
	{
		gameData.serverTimerCounter += deltaTime;
		while (gameData.serverTimerCounter > 0.001)
		{
			gameData.serverTimerCounter -= 0.001;
			gameData.serverTimer++;
		}
		//std::cout << gameData.serverTimer << "\n";
	}
#pragma endregion


#pragma region input

	static float moveSpeed = 20.f;

	if (platform::isKeyReleased(platform::Button::Escape))
	{
		gameData.escapePressed = !gameData.escapePressed;
	}

	if (platform::isKeyReleased(platform::Button::I))
	{
		gameData.showImgui = !gameData.showImgui;
	}

	if (platform::isKeyPressedOn(platform::Button::R))
	{
		programData.renderer.reloadShaders();
	}

	platform::showMouse(gameData.escapePressed);

	//move
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
		if (platform::isKeyHeld(platform::Button::LeftShift)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::RBumper].held
			)
		{
			moveDir.y -= speed;
		}
		if (platform::isKeyHeld(platform::Button::Space)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::LBumper].held
			)
		{
			moveDir.y += speed;
		}

		//gameData.c.moveFPS(moveDir);
		gameData.entityManager.localPlayer.moveFPS(moveDir);


		bool rotate = !gameData.escapePressed;
		if (platform::isRMouseHeld()) { rotate = true; }
		gameData.c.rotateFPS(platform::getRelMousePosition(), 0.25f * deltaTime, rotate);
		gameData.entityManager.localPlayer.lookDirection = gameData.c.viewDirection;
		
		if (!gameData.escapePressed)
		{
			platform::setRelMousePosition(w / 2, h / 2);
			gameData.c.lastMousePos = {w / 2, h / 2};
		}

	}
#pragma endregion


#pragma region block collisions
	{
			
		auto chunkGetter = [](glm::ivec2 pos) -> ChunkData*
		{
			auto c = gameData.chunkSystem.getChunkSafeFromChunkPos(pos.x, pos.y);
			if (c)
			{
				return &c->data;
			}
			else
			{
				return nullptr;
			}
		};

		if (gameData.colidable)
		{
			gameData.entityManager.localPlayer.body.resolveConstrains(chunkGetter, nullptr, deltaTime, 
				glm::vec3(0.8, 1.8, 0.8));
		}

		gameData.entityManager.localPlayer.body.updateMove();
		gameData.c.position = gameData.entityManager.localPlayer.body.pos + glm::dvec3(0,1.5,0);

		gameData.entityManager.doAllUpdates(deltaTime, chunkGetter);



	}
#pragma endregion


#pragma region drop items
	
	if (platform::isKeyPressedOn(platform::Button::Q))
	{
		gameData.entityManager.dropItemByClient(gameData.entityManager.localPlayer.body.pos,
			BlockTypes::diamond_ore, gameData.undoQueue, gameData.c.viewDirection * 5.f,
			gameData.serverTimer);
	}

#pragma endregion


#pragma region place blocks

	glm::ivec3 rayCastPos = {};
	std::optional<glm::ivec3> blockToPlace = std::nullopt;
	glm::dvec3 cameraRayPos = gameData.c.position;

	if (gameData.chunkSystem.rayCast(cameraRayPos, gameData.c.viewDirection, rayCastPos, 20, blockToPlace))
	{
		auto b = gameData.chunkSystem.getBlockSafe(rayCastPos);
		if (b && b->type != BlockTypes::water)
		{
			programData.gyzmosRenderer.drawCube(rayCastPos);
		}

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
						gameData.undoQueue, gameData.entityManager.localPlayer.body.pos, gameData.lightSystem);
				}
				else if (platform::isLMouseReleased())
				{
					gameData.chunkSystem.placeBlockByClient(rayCastPos, BlockTypes::air,
						gameData.undoQueue, gameData.entityManager.localPlayer.body.pos, gameData.lightSystem);
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

	programData.renderer.entityRenderer.itemEntitiesToRender.push_back({gameData.entityTest});

#pragma region chunks and rendering
	if(w != 0 && h != 0)
	{
		//static std::vector<int> data;
		
		programData.renderer.updateDynamicBlocks();
		
		gameData.gameplayFrameProfiler.startSubProfile("chunkSystem");
		gameData.chunkSystem.update(blockPositionPlayer, deltaTime, gameData.undoQueue,
			gameData.lightSystem);
		gameData.gameplayFrameProfiler.endSubProfile("chunkSystem");



		gameData.gameplayFrameProfiler.startSubProfile("rendering");

		gameData.sunShadow.update();

		if (programData.renderer.renderShadows)
		{
			programData.renderer.renderShadow(gameData.sunShadow,
				gameData.chunkSystem, gameData.c, programData);
		}

		gameData.sunShadow.renderShadowIntoTexture(gameData.c);

		//programData.renderer.render(data, gameData.c, programData.texture);
		programData.renderer.renderFromBakedData(gameData.sunShadow,gameData.chunkSystem, 
			gameData.c, programData, programData.blocksLoader, gameData.entityManager, gameData.showLightLevels, 
			gameData.skyLightIntensity, gameData.point, underWater, w, h, deltaTime);

		gameData.c.lastFrameViewProjMatrix =
			gameData.c.getProjectionMatrix() * gameData.c.getViewMatrix();

		gameData.gameplayFrameProfiler.endSubProfile("rendering");
	}
#pragma endregion

#pragma region drop entities that are too far

	gameData.entityManager.dropEntitiesThatAreTooFar({blockPositionPlayer.x,blockPositionPlayer.z},
		gameData.chunkSystem.squareSize);


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

	for (auto &p : gameData.entityManager.players)
	{
		
		programData.pointDebugRenderer.renderPoint(gameData.c, p.second.position);

		//todo this will be refactored
		auto boxSize = glm::vec3(0.8, 1.8, 0.8);
		auto pos = p.second.position;

		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2));

		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2));

		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2),
			pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2));
		programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2),
			pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2));

	}

	
	//programData.gyzmosRenderer.drawLine(
	//	gameData.point,
	//	glm::vec3(gameData.point) + glm::vec3(gameData.pointSize));

	if (gameData.renderPlayerPos)
	{
		programData.gyzmosRenderer.drawCube(blockPositionPlayer);
	}

	programData.gyzmosRenderer.render(gameData.c, posInt, posFloat);

#pragma endregion


#pragma region imgui

	bool terminate = false;

	if (gameData.showImgui)
	{
		gameData.gameplayFrameProfiler.startSubProfile("imgui");


		//if (ImGui::Begin("camera controll", &gameData.escapePressed))
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {26/255.f,26/255.f,26/255.f,0.5f});
		if (ImGui::Begin("client controll"))
		{
			if (ImGui::CollapsingHeader("Camera stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Checkbox("Colidable", &gameData.colidable);

				ImGui::DragScalarN("Player Body pos", ImGuiDataType_Double,
					&gameData.entityManager.localPlayer.body.pos[0], 3, 0.01);
				gameData.entityManager.localPlayer.body.lastPos = gameData.entityManager.localPlayer.body.pos;

				ImGui::Text("camera float: %f, %f, %f", posFloat.x, posFloat.y, posFloat.z);
				ImGui::Text("camera int: %d, %d, %d", posInt.x, posInt.y, posInt.z);
				ImGui::Text("camera view: %f, %f, %f", gameData.c.viewDirection.x, gameData.c.viewDirection.y, gameData.c.viewDirection.z);

				ImGui::Text("Chunk: %d, %d", divideChunk(posInt.x), divideChunk(posInt.z));

				//ImGui::DragScalarN("Point pos", ImGuiDataType_Double, &point[0], 3, 1);
				ImGui::DragInt3("Point pos", &gameData.point[0]);
				ImGui::DragInt3("Point size", &gameData.pointSize[0]);
				ImGui::Checkbox("Render Box", &gameData.renderBox);
				ImGui::Checkbox("Render Player Pos", &gameData.renderPlayerPos);

				ImGui::DragScalarN("Entity pos test", ImGuiDataType_Double,
					&gameData.entityTest[0], 3, 0.1);

				ImGui::DragFloat("camera speed", &moveSpeed);

			}

			ImGui::NewLine();

			gameData.pointSize = glm::clamp(gameData.pointSize, glm::ivec3(0, 0, 0), glm::ivec3(64, 64, 64));

			if (ImGui::CollapsingHeader("Light Stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Checkbox("showLightLevels", &gameData.showLightLevels);
				ImGui::SliderInt("skyLightIntensity", &gameData.skyLightIntensity, 0, 15);
				ImGui::SliderFloat("metallic", &programData.renderer.metallic, 0, 1);
				ImGui::SliderFloat("roughness", &programData.renderer.roughness, 0, 1);
				ImGui::SliderFloat("exposure", &programData.renderer.defaultShader
					.shadingSettings.exposure, 0.001, 10);
				ImGui::Combo("Tonemapper", &programData.renderer.defaultShader.
					shadingSettings.tonemapper, "ACES\0AgX\0ZCAM\0");

				ImGui::SliderFloat3("Sky pos", &programData.renderer.skyBoxRenderer.sunPos[0], -1, 1);

				ImGui::ColorPicker3("water color", 
					&programData.renderer.defaultShader.shadingSettings.waterColor[0]);

				ImGui::ColorPicker3("under water color",
					&programData.renderer.defaultShader.shadingSettings.underWaterColor[0]);

				ImGui::SliderFloat("underwaterDarkenStrength",
					&programData.renderer.defaultShader.shadingSettings.underwaterDarkenStrength,
					0, 1);

				ImGui::SliderFloat("underwaterDarkenDistance",
					&programData.renderer.defaultShader.shadingSettings.underwaterDarkenDistance,
					0, 40);

				ImGui::SliderFloat("fogGradientUnderWater",
					&programData.renderer.defaultShader.shadingSettings.fogGradientUnderWater,
					0, 10);


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

			
			ImGui::Text("fps: %d", programData.currentFps);
			if (ImGui::Button("Exit game"))
			{
				terminate = true;
			}

			ImGui::NewLine();

			
			if (ImGui::CollapsingHeader("Load Save Stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				static char fileBuff[256] = RESOURCES_PATH "gameData/structures/test.structure";
				ImGui::InputText("File:", fileBuff, sizeof(fileBuff));

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
										gameData.undoQueue, 
										gameData.entityManager.localPlayer.body.pos, gameData.lightSystem);
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

			if (ImGui::CollapsingHeader("HBAO Map",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboHBAO.color, {256, 256},
					{0, 1}, {1, 0});
			}

			ImGui::NewLine();

			if (ImGui::CollapsingHeader("Screen space pos",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboLastFramePositions.color, {256, 256},
					{0, 1}, {1, 0});
			}

			if (ImGui::CollapsingHeader("Last frame color",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboLastFrame.color, {256, 256},
					{0, 1}, {1, 0});
			}

			if (ImGui::CollapsingHeader("Chunk system",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{

				static int blockSize = 25;

				ImGui::SliderInt("Size", &blockSize, 10, 100);

				ImVec4 colNotLoaded = {0.2f,0.2f,0.2f,1.f};
				ImVec4 colLoaded = {0.2f,0.9f,0.2f,1.f};
				ImVec4 colCulled = {0.4f,0.1f,0.1f,1.f};
				ImVec4 colLoadedRebakingTransparency = {0.2f,0.4f,1.0f,1.f};
				ImVec4 colLoadedButNotBaked = {0.5f,0.5f,0.2f,1.f};
				ImVec4 colrequested = {0.2f,0.2f,0.9f,1.f};

				ImGui::Text("Gpu buffer entries count: %d",
					(int)gameData.chunkSystem.gpuBuffer.entriesMap.size());

				if (ImGui::Button("Drop all chunks"))
				{
					gameData.chunkSystem.dropAllChunks(&gameData.chunkSystem.gpuBuffer);
				}
				
				ImGui::ColorButton("##1", colNotLoaded, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Not loaded."); 

				ImGui::ColorButton("##2", colrequested, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Requested Loaded.");

				ImGui::ColorButton("##3", colLoadedButNotBaked, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Loaded not baked");

				ImGui::ColorButton("##4", colLoadedRebakingTransparency, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Rebaking transparency");

				ImGui::ColorButton("##5", colCulled, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Culled (but loaded)");

				ImGui::ColorButton("##6", colLoaded, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Loaded!");

				ImGui::Separator();

				for (int z = 0; z < gameData.chunkSystem.squareSize; z++)
					for (int x = 0; x < gameData.chunkSystem.squareSize; x++)
					{

						auto c = gameData.chunkSystem.getChunksInMatrixSpaceUnsafe(x, z);

						auto currentColor = colNotLoaded;

						if (c != nullptr)
						{
							if (c->culled)
							{
								currentColor = colCulled;
							}
							else if (c->dirty)
							{
								currentColor = colLoadedButNotBaked;
							}
							else if(c->dirtyTransparency)
							{
								currentColor = colLoadedRebakingTransparency;
							}
							else
							{
								currentColor = colLoaded;
							}

						}
						else
						{
							auto pos = gameData.chunkSystem.
								fromMatrixSpaceToChunkSpace(x, z);

							if (gameData.chunkSystem.recentlyRequestedChunks.find(pos)
								!= gameData.chunkSystem.recentlyRequestedChunks.end()
								)
							{
								currentColor = colrequested;
							}
						}

						if (x > 0)
							ImGui::SameLine();

						ImGui::PushID(z *gameData.chunkSystem.squareSize + x);
						if (ImGui::ColorButton("##chunkb", currentColor,
							ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip
							, ImVec2(blockSize, blockSize)))
						{

						}
						ImGui::PopID();


					}

			}

			ImGui::Checkbox("Unified geometry pool",
				&programData.renderer.unifiedGeometry);

			ImGui::Checkbox("Sort chunks",
				&programData.renderer.sortChunks);

			ImGui::Checkbox("Z pre pass",
				&programData.renderer.zprepass);

			bool shaders = programData.renderer.defaultShader.shadingSettings.shaders;
			ImGui::Checkbox("Shaders",
				&shaders);
			programData.renderer.defaultShader.shadingSettings.shaders = shaders;

			ImGui::Checkbox("Frustum culling",
				&programData.renderer.frustumCulling);

			ImGui::Checkbox("Shadows",
				&programData.renderer.renderShadows);
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

		gameData.gameplayFrameProfiler.endSubProfile("imgui");
	}

#pragma endregion


#pragma region ui
	if (w != 0 && h != 0)
	{	

		Ui::Frame f({0,0, w, h});

		programData.renderer2d.renderRectangle(
			Ui::Box().xCenter().yCenter().xDimensionPixels(30).yAspectRatio(1.f),
			programData.uiTexture, Colors_White, {}, 0, 
			programData.uiAtlas.get(2, 0)
		);
	}
#pragma endregion

	gameData.gameplayFrameProfiler.endFrame();
	gameData.gameplayFrameProfiler.startFrame();
	gameData.gameplayFrameProfiler.startSubProfile("swap chain and others");

	if (terminate)
	{
		return false;
	}

	return true;
}

void closeGameLogic()
{
	gameData.chunkSystem.cleanup();
	gameData = GameData(); //free all resources
}