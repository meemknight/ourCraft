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
#include "rendering/UiEngine.h"
#include "glui/glui.h"
#include <platformTools.h>
#if REMOVE_IMGUI == 0
#include <imgui.h>
#endif
#include <iostream>
#include "multyPlayer/undoQueue.h"
#include <lightSystem.h>
#include <structure.h>
#include <safeSave.h>
#include <rendering/sunShadow.h>
#include <multiPlot.h>
#include <profiler.h>
#include <thread>
#include <gameplay/physics.h>
#include <gameplay/entityManagerClient.h>
#include <gameplay/items.h>
#include <gameplay/crafting.h>
#include <rendering/renderSettings.h>
#include <audioEngine.h>

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
	glm::ivec3 pointSize = {};
	glm::dvec3 entityTest = {-4, 113, 3};
	bool renderBox = 0;
	bool renderPlayerPos = 0;
	bool renderColliders = 0;
	bool fly = 1;
	
	bool colidable = 1;

	ClientEntityManager entityManager;

	std::uint64_t serverTimer = 0;
	float serverTimerCounter = 0;

	glm::dvec3 lastSendPos = {-INFINITY,0,0};

	int currentItemSelected = 0;

	unsigned char currentBlockInteractionRevisionNumber = 0;
	unsigned char blockInteractionType = 0;
	glm::ivec3 blockInteractionPosition = {0, -1, 0};

	bool insideInventoryMenu = 0;

}gameData;


bool initGameplay(ProgramData &programData, const char *c) //GAME STUFF!
{

	Packet_ReceiveCIDAndData playerData;

	if (!createConnection(playerData, c))
	{
		reportError("Problem joining server");
		return false;
	}
	



	gameData = GameData();
	gameData.c.position = glm::vec3(0, 65, 0);

	gameData.entityManager.localPlayer.entity = playerData.entity;
	gameData.entityManager.localPlayer.entityId = playerData.yourPlayerEntityId;
	gameData.entityManager.localPlayer.otherPlayerSettings = playerData.otherSettings;

	//todo restant timer here ...
	//playerData.timer;


	gameData.chunkSystem.init(programData.otherSettings.viewDistance * 2);

	//TODO, MOVE TO PROGRAM DATA!!!!!!!!!!!
	gameData.sunShadow.init();

	//-5359
	//6348
	//todo clear history stuff here
	//programData.GPUProfiler.;

	//gameData.inventory.heldInMouse = Item(BlockTypes::glass);


	//we started the game!
	AudioEngine::stopAllMusicAndSounds();
	return true;
}


void exitInventoryMenu()
{

	if (gameData.blockInteractionType)
	{

		Packet_RecieveExitBlockInteraction packetData;
		packetData.revisionNumber = gameData.currentBlockInteractionRevisionNumber;

		sendPacket(getServer(), formatPacket(headerRecieveExitBlockInteraction),
			(char *)&packetData, sizeof(Packet_RecieveExitBlockInteraction),
			true, channelChunksAndBlocks);
	}

	gameData.blockInteractionType = 0;
	gameData.blockInteractionPosition = {0, -1, 0};

	gameData.insideInventoryMenu = false;
}

bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData)
{
	gameData.gameplayFrameProfiler.endSubProfile("swap chain and others");

	if (h != 0)
	{
		gameData.c.aspectRatio = (float)w / h;
	}
	glViewport(0, 0, w, h);

	auto &player = gameData.entityManager.localPlayer;


#pragma region server stuff
	{
		gameData.gameplayFrameProfiler.startSubProfile("server messages");

		EventCounter validateEvent = 0;
		RevisionNumber inValidateRevision = 0;
		bool disconnect = 0;

		if (!gameData.undoQueue.events.empty())
		{
			auto time = gameData.undoQueue.events[0].createTime;

			//todo Request the server for a hard reset rather than a timeout?
			if ((getTimer() - time) > 10'000)
			{
				std::cout << "Client timeouted because of validate events!\n";
				return 0;
			}
		}

		bool shouldExitBlockInteraction = 0;

		clientMessageLoop(validateEvent, inValidateRevision,
			gameData.entityManager.localPlayer.entity.position, gameData.chunkSystem.squareSize,
			gameData.entityManager, gameData.undoQueue, gameData.serverTimer, disconnect, 
			gameData.currentBlockInteractionRevisionNumber, shouldExitBlockInteraction);

		if (disconnect) { return 0; }

		if (shouldExitBlockInteraction)
		{
			//exit block interaction
			gameData.blockInteractionPosition = {0,-1,0};
			gameData.blockInteractionType = 0;
			gameData.insideInventoryMenu = 0;
		}

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

			gameData.entityManager.localPlayer.entity.position = gameData.undoQueue.events[0].playerPos;
			gameData.entityManager.localPlayer.entity.lastPosition = gameData.undoQueue.events[0].playerPos;

			gameData.undoQueue.events.clear();

			gameData.undoQueue.currentEventId.revision++;
		}

		//player sends updates to server
		{

			static float timer = 0.016;

			///todo a common method to check if data was modified
			if (gameData.entityManager.localPlayer.entity.position != gameData.lastSendPos)
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
				data.timer = gameData.serverTimer;

				//todo SET THIS ALSO SOMEWHERE ELSE LOL!!!!
				gameData.entityManager.localPlayer.entity.chunkDistance = gameData.chunkSystem.squareSize;
				data.playerData = gameData.entityManager.localPlayer.entity;

				sendPacket(getServer(),
					formatPacket(headerSendPlayerData), (char *)&data, sizeof(data), 0,
					channelPlayerPositions);

				gameData.lastSendPos = gameData.entityManager.localPlayer.entity.position;
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

	//inventory and menu stuff
	if (!gameData.escapePressed)
	{

		if (platform::isKeyReleased(platform::Button::E))
		{
			if (gameData.insideInventoryMenu)
			{
				exitInventoryMenu();
			}
			else
			{
				gameData.insideInventoryMenu = true;
				gameData.blockInteractionType = 0;
			}

		}



		if (platform::isKeyReleased(platform::Button::I))
		{
			gameData.showImgui = !gameData.showImgui;
		}

		if (platform::isKeyPressedOn(platform::Button::R))
		{
			programData.renderer.reloadShaders();
		}


		//move
		if(!gameData.insideInventoryMenu)
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

		if (gameData.fly)
		{
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
		}
		else
		{
			if (platform::isKeyPressedOn(platform::Button::Space)
				|| platform::getControllerButtons().buttons[platform::ControllerButtons::LBumper].held
				)
			{
				gameData.entityManager.localPlayer.entity.jump();
			}
		}


		if (gameData.fly)
		{
			gameData.entityManager.localPlayer.entity.flyFPS(moveDir, gameData.c.viewDirection);
		}
		else
		{
			gameData.entityManager.localPlayer.entity.moveFPS(moveDir, gameData.c.viewDirection);
		}

		//gameData.c.moveFPS(moveDir);

		setBodyAndLookOrientation(gameData.entityManager.localPlayer.entity.bodyOrientation,
			gameData.entityManager.localPlayer.entity.lookDirectionAnimation, moveDir, gameData.c.viewDirection);

		//gameData.entityManager.localPlayer.bodyOrientation = 
		//gameData.entityManager.localPlayer.lookDirection = 

		bool rotate = !gameData.escapePressed;
		if (platform::isRMouseHeld()) { rotate = true; }
		gameData.c.rotateFPS(platform::getRelMousePosition(), 0.22f * deltaTime, rotate);

		if (!gameData.escapePressed)
		{
			platform::setRelMousePosition(w / 2, h / 2);
			gameData.c.lastMousePos = {w / 2, h / 2};
		}

	}


		//keyPad
		if (!gameData.insideInventoryMenu)
	{

		for (int i = 0; i < 9; i++)
		{
			if (platform::isKeyPressedOn(platform::Button::NR1 + i))
			{
				gameData.currentItemSelected = i;
			}
		}

		auto scroll = platform::getScroll();
		if (scroll < -0.5)
		{
			gameData.currentItemSelected++;
		}
		else if (scroll > 0.5)
		{
			gameData.currentItemSelected--;
		}

		gameData.currentItemSelected = std::clamp(gameData.currentItemSelected, 0, 8);

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


		PhysicalSettings settings;
		//settings.sideFriction = 2.f;

		gameData.entityManager.localPlayer.entity.updateForces(deltaTime, !gameData.fly, settings);


		if (gameData.colidable)
		{
			gameData.entityManager.localPlayer
				.entity.resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime,
				gameData.entityManager.localPlayer.entity.getColliderSize(), settings);
		}
		else
		{
			gameData.entityManager.localPlayer.entity.updatePositions();
		}

		gameData.c.position = gameData.entityManager.localPlayer.entity.position
			+ glm::dvec3(0,1.5,0);

		gameData.entityManager.doAllUpdates(deltaTime, chunkGetter);



	}
#pragma endregion


#pragma region drop items
	
	if (!gameData.insideInventoryMenu && !gameData.escapePressed)
	if (platform::isKeyPressedOn(platform::Button::Q))
	{
		gameData.entityManager.dropItemByClient(
			gameData.entityManager.localPlayer.entity.position,
			gameData.currentItemSelected, gameData.undoQueue, gameData.c.viewDirection * 5.f,
			gameData.serverTimer, player.inventory, !platform::isKeyHeld(platform::Button::LeftCtrl));
	}

#pragma endregion


#pragma region place blocks

	if (!gameData.insideInventoryMenu)
	{

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

			auto &item = player.inventory.items[gameData.currentItemSelected];

			if (item.isBlock())
			{
				if (platform::isKeyReleased(platform::Button::Z)) { item.type--; }
				if (platform::isKeyReleased(platform::Button::X)) { item.type++; }

				item.type = glm::clamp(item.type, (unsigned short)1u,
					(unsigned short)(BlocksCount - 1u));
			}



			if (platform::isKeyHeld(platform::Button::LeftCtrl)
				&& (platform::isLMousePressed() || platform::isRMousePressed())
				)
			{
				if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE
					&&
					blockToPlace)
				{
					auto b = gameData.chunkSystem.getBlockSafe(rayCastPos);
					if (b)
					{
						Item newItem(b->type);

						item = newItem;

						forceOverWriteItem(player.inventory, gameData.currentItemSelected, item);

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
				else if (!platform::isKeyHeld(platform::Button::LeftCtrl))
				{


					if (platform::isRMousePressed())
					{
						bool didAction = 0;

						auto b = gameData.chunkSystem.getBlockSafe(rayCastPos);
						if(b)
						{
							
							auto actionType = isInteractable(b->type);

							if (actionType)
							{
								didAction = true;

								gameData.currentBlockInteractionRevisionNumber++;
								gameData.blockInteractionType = actionType;
								gameData.blockInteractionPosition = rayCastPos;

								sendBlockInteractionMessage(player.entityId, rayCastPos,
									b->type, gameData.currentBlockInteractionRevisionNumber);
								
								gameData.insideInventoryMenu = true;

								//reset crafting table
								// TODO!!! exit menu thingy here
								//gameData.craftingTableInventory = {};
							}

						}

						if (!didAction)
						{
							if (item.isItemThatCanBeUsed() && blockToPlace)
							{

								Packet_ClientUsedItem data;
								data.from = gameData.currentItemSelected;
								data.itemType = item.type;
								data.position = *blockToPlace;
								data.revisionNumber = player.inventory.revisionNumber;

								sendPacket(getServer(), headerClientUsedItem, player.entityId,
									&data, sizeof(data), true, channelChunksAndBlocks);

								if (item.isConsumedAfterUse() && player.otherPlayerSettings.gameMode ==
									OtherPlayerSettings::SURVIVAL)
								{
									item.counter--;
									if (item.counter <= 0)
									{
										item = {};
									}
								}

							}
							else if (blockToPlace && item.isBlock())
							{

								gameData.chunkSystem.placeBlockByClient(*blockToPlace,
									gameData.currentItemSelected,
									gameData.undoQueue,
									gameData.entityManager.localPlayer.entity.position,
									gameData.lightSystem,
									player.inventory);

							}
						};

							
					}
					else if (platform::isLMousePressed())
					{
						//break block
						gameData.chunkSystem.breakBlockByClient(rayCastPos
							, gameData.undoQueue,
							gameData.entityManager.localPlayer.entity.position,
							gameData.lightSystem);
					}
				}


		};

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

	static float dayTime = 0.25;

#pragma region chunks and rendering
	if(w != 0 && h != 0)
	{
		
		
		gameData.gameplayFrameProfiler.startSubProfile("chunkSystem");
		gameData.chunkSystem.update(blockPositionPlayer, deltaTime, gameData.undoQueue,
			gameData.lightSystem);
		gameData.gameplayFrameProfiler.endSubProfile("chunkSystem");



		gameData.gameplayFrameProfiler.startSubProfile("rendering");

		gameData.sunShadow.update();

		if (programData.renderer.defaultShader.shadingSettings.shadows)
		{
			programData.renderer.renderShadow(gameData.sunShadow,
				gameData.chunkSystem, gameData.c, programData);
		}

		gameData.sunShadow.renderShadowIntoTexture(gameData.c);

		//programData.renderer.render(data, gameData.c, programData.texture);
		programData.renderer.renderFromBakedData(gameData.sunShadow,gameData.chunkSystem, 
			gameData.c, programData, programData.blocksLoader, gameData.entityManager,
			programData.modelsManager, gameData.showLightLevels,
			gameData.point, underWater, w, h, deltaTime, dayTime);

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
	
	programData.GPUProfiler.startSubProfile("Debug rendering");


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


	auto drawPlayerBox = [&](glm::dvec3 pos, glm::vec3 boxSize)
	{
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
	};
	
	if (gameData.renderColliders)
	{

		for (auto &p : gameData.entityManager.pigs)
		{
			programData.pointDebugRenderer.
				renderPoint(gameData.c, p.second.getRubberBandPosition());
		
			auto boxSize = glm::vec3(0.8, 0.8, 0.8);
			auto pos = p.second.getRubberBandPosition();
		
			drawPlayerBox(pos, boxSize);
		}

		for (auto &p : gameData.entityManager.players)
		{
			programData.pointDebugRenderer.
				renderPoint(gameData.c, p.second.getRubberBandPosition());
		
			auto boxSize = p.second.entity.getColliderSize();
			auto pos = p.second.getRubberBandPosition();
		
			drawPlayerBox(pos, boxSize);
		}
		
		for (auto &p : gameData.entityManager.zombies)
		{
		
			programData.pointDebugRenderer.
				renderPoint(gameData.c, p.second.getRubberBandPosition());
		
			auto boxSize = p.second.entity.getColliderSize();
			auto pos = p.second.getRubberBandPosition();
		
			drawPlayerBox(pos, boxSize);
		}

		for (auto &p : gameData.entityManager.cats)
		{

			programData.pointDebugRenderer.
				renderPoint(gameData.c, p.second.getRubberBandPosition());

			auto boxSize = p.second.entity.getColliderSize();
			auto pos = p.second.getRubberBandPosition();

			drawPlayerBox(pos, boxSize);
		}

	}


	//programData.gyzmosRenderer.drawLine(
	//	gameData.point,
	//	glm::vec3(gameData.point) + glm::vec3(gameData.pointSize));

	if (gameData.renderPlayerPos)
	{
		programData.gyzmosRenderer.drawCube(blockPositionPlayer);
	}

	programData.gyzmosRenderer.render(gameData.c, posInt, posFloat);

	programData.GPUProfiler.endSubProfile("Debug rendering");

#pragma endregion


#pragma region imgui
	
	bool terminate = false;

#if REMOVE_IMGUI == 0

	if (gameData.showImgui)
	{
		gameData.gameplayFrameProfiler.startSubProfile("imgui");

		//if (ImGui::Begin("camera controll", &gameData.escapePressed))
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {26/255.f,26/255.f,26/255.f,0.5f});
		if (ImGui::Begin("client controll"))
		{

			ImGui::SliderFloat("Day time", &dayTime, 0, 1);

			if (ImGui::CollapsingHeader("Camera stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Checkbox("Colidable", &gameData.colidable);
				ImGui::Checkbox("Fly", &gameData.fly);

				ImGui::DragScalarN("Player Body pos", ImGuiDataType_Double,
					&gameData.entityManager.localPlayer.entity.position[0], 3, 0.01);
				gameData.entityManager.localPlayer.entity.lastPosition 
					= gameData.entityManager.localPlayer.entity.position;

				ImGui::Text("camera float: %f, %f, %f", posFloat.x, posFloat.y, posFloat.z);
				ImGui::Text("camera int: %d, %d, %d", posInt.x, posInt.y, posInt.z);
				ImGui::Text("camera view: %f, %f, %f", gameData.c.viewDirection.x, gameData.c.viewDirection.y, gameData.c.viewDirection.z);

				ImGui::Text("Chunk: %d, %d", divideChunk(posInt.x), divideChunk(posInt.z));

				//ImGui::DragScalarN("Point pos", ImGuiDataType_Double, &point[0], 3, 1);
				ImGui::DragInt3("Point pos", &gameData.point[0]);
				ImGui::DragInt3("Point size", &gameData.pointSize[0]);
				ImGui::Checkbox("Render Box", &gameData.renderBox);
				ImGui::Checkbox("Render Player Pos", &gameData.renderPlayerPos);
				ImGui::Checkbox("Render Coliders", &gameData.renderColliders);

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

									std::cout << "Not implemented!\n";

									//todo implement
									//gameData.chunkSystem.placeBlockByClient(pos, s->unsafeGet(x, y, z),
									//	gameData.undoQueue, 
									//	gameData.entityManager.localPlayer.entity.position, gameData.lightSystem);
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

			if (ImGui::CollapsingHeader("Sky Map",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboSkyBox.color, {256, 256},
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
				ImGui::Text("Requested.");

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
							if (c->isCulled())
							{
								currentColor = colCulled;
							}
							else if (c->isDirty())
							{
								currentColor = colLoadedButNotBaked;
							}
							else if(c->isDirtyTransparency())
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

			//ImGui::Checkbox("Unified geometry pool",
			//	&programData.renderer.unifiedGeometry);

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

			ImGui::Checkbox("SSAO",
				&programData.renderer.ssao);
			//ImGui::Checkbox("Water Refraction",
			//	&programData.renderer.waterRefraction);

			if (ImGui::CollapsingHeader("Music ",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				
				if (ImGui::Button("Play random night music"))
				{

					AudioEngine::playRandomNightMusic();

				}
			}
			
			
		}
		ImGui::End();


		if (ImGui::Begin("Profiler"))
		{
			ImGui::Text("profiler");
			ImGui::Text("fps: %d", programData.currentFps);

			gameData.gameplayFrameProfiler.displayPlot("Gameplay Frame");

			programData.GPUProfiler.displayPlot("GPU");

		}
		ImGui::End();


		ImGui::PopStyleColor();

		gameData.gameplayFrameProfiler.endSubProfile("imgui");
	}
#endif
#pragma endregion


#pragma region crafting

	Item itemToCraft;

	if (gameData.insideInventoryMenu)
	{
		if (gameData.blockInteractionType == InteractionTypes::craftingTable)
		{
			itemToCraft = craft9(player.inventory.crafting);
		}
		else
		{
			itemToCraft = craft4(player.inventory.crafting);
		}

	}


#pragma endregion



#pragma region ui

	int cursorSelected = -2;


	programData.ui.renderGameUI(deltaTime, w, h, gameData.currentItemSelected,
		player.inventory, programData.blocksLoader, gameData.insideInventoryMenu,
		cursorSelected, itemToCraft, 
		(gameData.blockInteractionType == InteractionTypes::craftingTable)
		);


#pragma endregion

	//std::cout << cursorSelected << "\n";

#pragma region move items in inventory
	if (gameData.insideInventoryMenu && !gameData.escapePressed)
	{
		
		static std::bitset<64> rightClickedThisClick = 0;

		if (platform::isLMousePressed())
		{

			if (cursorSelected == PlayerInventory::CRAFTING_RESULT_INDEX && itemToCraft.type)
			{
				Item *cursor = &player.inventory.heldInMouse;
				
				if (cursor->type == 0)
				{
					
					//craft one
					*cursor = itemToCraft;
					if (gameData.blockInteractionType == InteractionTypes::craftingTable)
					{
						player.inventory.craft9();
					}
					else
					{
						player.inventory.craft4();
					}
					cratedOneItem(player.inventory, itemToCraft, PlayerInventory::CURSOR_INDEX);

				}
				else if(cursor->type == itemToCraft.type)
				{
					//grab and craft one
					if (cursor->counter < cursor->getStackSize())
					{

						if (cursor->counter + itemToCraft.counter <= cursor->getStackSize())
						{
							cursor->counter += itemToCraft.counter;
							if (gameData.blockInteractionType == InteractionTypes::craftingTable)
							{
								player.inventory.craft9();
							}
							else
							{
								player.inventory.craft4();
							}
							cratedOneItem(player.inventory, itemToCraft, PlayerInventory::CURSOR_INDEX);
						}

					}

				}



			}
			else
			if (cursorSelected >= 0)
			{
				Item *selected = player.inventory.getItemFromIndex(cursorSelected);
				Item *cursor = &player.inventory.heldInMouse;

				if (selected && selected != cursor)
				{

					if (cursor->type == 0)
					{
						//grab
						grabItem(player.inventory, cursorSelected, PlayerInventory::CURSOR_INDEX);
					}
					else
					{
						//place
						if (!placeItem(player.inventory, PlayerInventory::CURSOR_INDEX, cursorSelected))
						{
							//swap
							swapItems(player.inventory, cursorSelected, PlayerInventory::CURSOR_INDEX);
						}

					}

				}
			}
		}
		else if (platform::isRMousePressed())
		{

			rightClickedThisClick.reset();


			if (cursorSelected >= 0)
			{
				Item *selected = player.inventory.getItemFromIndex(cursorSelected);
				Item *cursor = &player.inventory.heldInMouse;

				if (cursor->type != 0)
				{
					//place one
					if (placeItem(player.inventory, PlayerInventory::CURSOR_INDEX, cursorSelected, 1))
					{
						rightClickedThisClick[cursorSelected] = true;
					}

				}
				else
				{

					//grab
					grabItem(player.inventory, cursorSelected, PlayerInventory::CURSOR_INDEX, selected->counter/2);

					//don't place it again lol
					rightClickedThisClick[cursorSelected] = true;
				}


			}



		}
		else if (platform::isRMouseHeld())
		{

			//right click held place items
			if (cursorSelected >= 0 && !rightClickedThisClick[cursorSelected])
			{
				Item *selected = player.inventory.getItemFromIndex(cursorSelected);
				Item *cursor = &player.inventory.heldInMouse;

				if (cursor->type != 0)
				{
					//place one
					if (placeItem(player.inventory, PlayerInventory::CURSOR_INDEX, cursorSelected, 1))
					{
						rightClickedThisClick[cursorSelected] = true;
					}

				}
			}


		}

		//outside borders
		if (cursorSelected == -1)
		{

			if (platform::isLMousePressed())
			{
				gameData.entityManager.dropItemByClient(
					gameData.entityManager.localPlayer.entity.position,
					PlayerInventory::CURSOR_INDEX, gameData.undoQueue, gameData.c.viewDirection * 5.f,
					gameData.serverTimer, player.inventory, 0);
			}
			else if (platform::isRMousePressed())
			{
				gameData.entityManager.dropItemByClient(
					gameData.entityManager.localPlayer.entity.position,
					PlayerInventory::CURSOR_INDEX, gameData.undoQueue, gameData.c.viewDirection * 5.f,
					gameData.serverTimer, player.inventory, 1);
			}

		}

	}

	player.inventory.sanitize();

#pragma endregion


#pragma region esc manu

	bool justPressedEsc = 0;
	if (gameData.insideInventoryMenu)
	{
		if (platform::isKeyReleased(platform::Button::Escape))
		{
			exitInventoryMenu();
		}
	}
	else
	{
		if (platform::isKeyReleased(platform::Button::Escape) && !gameData.escapePressed)
		{
			gameData.escapePressed = !gameData.escapePressed;
			justPressedEsc = true;
		}
	}


	if (gameData.escapePressed)
	{

		programData.ui.renderer2d.renderRectangle({0,0,programData.ui.renderer2d.windowW,
			programData.ui.renderer2d.windowH}, {0,0,0,0.5});
	
		programData.ui.menuRenderer.Begin(2);
		programData.ui.menuRenderer.SetAlignModeFixedSizeWidgets({0,150});

		programData.ui.menuRenderer.Text("Game Menu", Colors_White);

		if (programData.ui.menuRenderer.Button("Back to Game", Colors_Gray, programData.ui.buttonTexture))
		{
			gameData.escapePressed = false;
		}

		displayRenderSettingsMenuButton(programData);


		programData.ui.menuRenderer.End();

		if (programData.ui.menuRenderer.internal.allMenuStacks[2].size() == 0 && 
			platform::isKeyReleased(platform::Button::Escape) && !justPressedEsc)
		{
			gameData.escapePressed = false;
		}

	}

	platform::showMouse(gameData.escapePressed || gameData.insideInventoryMenu);


#pragma endregion

	gameData.chunkSystem.changeRenderDistance(programData.otherSettings.viewDistance * 2);


	gameData.gameplayFrameProfiler.endFrame();
	programData.GPUProfiler.endFrame();
	gameData.gameplayFrameProfiler.startFrame();
	programData.GPUProfiler.startFrame();
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