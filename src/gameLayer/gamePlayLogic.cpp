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
#include <gameplay/mapEngine.h>


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

	MapEngine mapEngine;
	bool isInsideMapView;

	//debug stuff
	glm::ivec3 point = {};
	glm::ivec3 pointSize = {};
	glm::dvec3 entityTest = {-4, 113, 3};
	bool renderBox = 1;
	bool renderPlayerPos = 0;
	bool renderColliders = 0;
	
	bool colidable = 1;

	ClientEntityManager entityManager;

	std::uint64_t serverTimer = 0;
	float serverTimerCounter = 0;

	glm::dvec3 lastSendPos = {-INFINITY,0,0};

	int currentItemSelected = 0;
	
	InteractionData interaction;
	unsigned char currentBlockInteractionRevisionNumber = 0;

	AdaptiveExposure adaptiveExposure;

	bool insideInventoryMenu = 0;
	int currentInventoryTab = 0;

	struct
	{
		bool breaking = 0;
		glm::ivec3 pos = {};
		float timer = 0;
		float totalTime = 0;
		int tool = 0;
	}currentBlockBreaking;

	std::string currentSkinName = "";
	gl2d::Texture currentSkinTexture = {};
	GLuint64 currentSkinBindlessTexture = 0;

	 
	BoneTransform playerFOVHandTransform{
		glm::vec3{glm::radians(120.f),0.f,0.f},
		glm::vec3{0.2,-2.0,-0.5}
	};

	bool handHit = 0;
	bool killed = 0;

	//water drops blur
	float dropsStrength = 0;
	bool lastFrameInWater = 0;

	int craftingSlider = 0;
	bool showUI = 1;

}gameData;

void loadCurrentSkin()
{

	if (gameData.currentSkinTexture.id)
	{
		gameData.currentSkinTexture.cleanup();
	}

	gameData.currentSkinName = getSkinName();
	if (gameData.currentSkinName == "")
	{

	}
	else
	{
		gameData.currentSkinTexture 
			= loadPlayerSkin((RESOURCES_PATH "skins/" + gameData.currentSkinName + ".png").c_str());
	}

	if (!gameData.currentSkinTexture.id)
	{
		gameData.currentSkinTexture
			= loadPlayerSkin(RESOURCES_PATH "assets/models/steve.png");
	}

	//todo repeating code
	gameData.currentSkinTexture.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2.f);

	glGenerateMipmap(GL_TEXTURE_2D);


	gameData.currentSkinBindlessTexture = glGetTextureHandleARB(gameData.currentSkinTexture.id);
	glMakeTextureHandleResidentARB(gameData.currentSkinBindlessTexture);

	sendPlayerSkinPacket(getServer(), getConnectionData().cid, gameData.currentSkinTexture);

}

bool ShowOpenFileDialog(HWND hwnd, char *filePath, DWORD filePathSize, const char *initialDir,
	const char *filter)
{
	// Initialize the OPENFILENAME structure
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	// Zero out the file path buffer
	ZeroMemory((void*)filePath, filePathSize);


	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = filePathSize;
	ofn.lpstrInitialDir = initialDir;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	// Show the file open dialog
	return GetOpenFileNameA(&ofn);
}

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


	gameData.chunkSystem.init(getShadingSettings().viewDistance * 2);

	//TODO, MOVE TO PROGRAM DATA!!!!!!!!!!!
	gameData.sunShadow.init();

	loadCurrentSkin();

	//-5359
	//6348
	//todo clear history stuff here
	//programData.GPUProfiler.;

	//gameData.inventory.heldInMouse = Item(BlockTypes::glass);


	//we started the game!
	AudioEngine::stopAllMusicAndSounds();
	return true;
}

//todo: bug: use the same revision for inventory and block placement and also resend block pos or something

void dealDamageToLocalPlayer(int damage)
{
	if (damage < 0) { return; }
	if (damage == 0) { AudioEngine::playHurtSound(); return; }
	damage = std::min(damage, MAXSHORT - 10);


	auto &p = gameData.entityManager.localPlayer;

	Packet_ClientDamageLocally packetData;
	packetData.damage = damage;

	p.life.life -= damage;
	if (p.life.life <= 0) 
	{
		p.life.life = 0; //kill
		gameData.killed = true;

		sendPacket(getServer(), formatPacket(headerClientDamageLocallyAndDied),
			0, 0,
			true, channelChunksAndBlocks);
	}
	else
	{
		sendPacket(getServer(), formatPacket(headerClientDamageLocally),
			(char *)&packetData, sizeof(Packet_ClientDamageLocally),
			true, channelChunksAndBlocks);

		AudioEngine::playHurtSound();

	}

}

void exitInventoryMenu()
{

	if (gameData.interaction.blockInteractionType)
	{

		Packet_RecieveExitBlockInteraction packetData;
		packetData.revisionNumber = gameData.currentBlockInteractionRevisionNumber;

		sendPacket(getServer(), formatPacket(headerRecieveExitBlockInteraction),
			(char *)&packetData, sizeof(Packet_RecieveExitBlockInteraction),
			true, channelChunksAndBlocks);
	}

	gameData.interaction = {};

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
		bool respawned = 0;

		clientMessageLoop(validateEvent, inValidateRevision,
			from3DPointToBlock(gameData.c.position), gameData.chunkSystem.squareSize,
			gameData.entityManager, gameData.undoQueue,
			gameData.chunkSystem, gameData.lightSystem,
			gameData.serverTimer, disconnect, 
			gameData.currentBlockInteractionRevisionNumber, shouldExitBlockInteraction,
			gameData.killed, respawned);

		if (disconnect) { return 0; }

		if (respawned)
		{
			gameData.killed = false;
		}

		if (shouldExitBlockInteraction)
		{
			//exit block interaction
			gameData.interaction = {};
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

		//undo stuff
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

				if (e.type == UndoQueueEvent::iPlacedBlock)
				{

					gameData.chunkSystem.placeBlockNoClient(e.blockPos, e.originalBlock, gameData.lightSystem,
						&e.blockData, gameData.interaction);

					if (e.blockPos == gameData.currentBlockBreaking.pos)
					{
						gameData.currentBlockBreaking = {};
					}

				}
				else if (e.type == UndoQueueEvent::iDroppedItemFromInventory)
				{
					gameData.entityManager.removeDroppedItem(e.entityId);
				}
				else if (e.type == UndoQueueEvent::changedBlockData)
				{

					if (e.originalBlock.getType() == BlockTypes::structureBase)
					{

						BaseBlock block;
						size_t _ = 0;
						if (block.readFromBuffer(e.blockData.data(), e.blockData.size(), _))
						{
							
							auto *c = gameData.chunkSystem.getChunkSafeFromBlockPos(e.blockPos.x, e.blockPos.z);

							if (c)
							{

								auto blockData = c->blockData.getBaseBlock(modBlockToChunk(e.blockPos.x), e.blockPos.y,
									modBlockToChunk(e.blockPos.z));

								if (blockData)
								{
									*blockData = block;
								}

							}

						}


					}


				}


			}

			gameData.undoQueue.events.clear();

			gameData.undoQueue.currentEventId.revision++;
		}

		//player sends updates to server
		{

			static float timer = 0.020;

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
				timer = 0.020 * 3.f;
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

#pragma region reload skin

	if (gameData.currentSkinName != getSkinName())
	{
		loadCurrentSkin();
		//todo signal to others
	}

#pragma endregion


#pragma region input

	bool stopMainInput = gameData.escapePressed || gameData.killed || gameData.insideInventoryMenu ||
		gameData.isInsideMapView;

	static float moveSpeed = 7.f;
	float isPlayerMovingSpeed = 0;


	if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL)
	{
		player.entity.fly = 0;
	}

	if (gameData.killed)
	{
		gameData.insideInventoryMenu = false;
		gameData.interaction = {};
	}

	if (platform::isKeyReleased(platform::Button::F9))
	{
		programData.showImgui = !programData.showImgui;
	}

	if (platform::isKeyReleased(platform::Button::F10))
	{
		gameData.showUI = !gameData.showUI;
	}

	if(!stopMainInput || gameData.insideInventoryMenu)
	if (platform::isKeyReleased(platform::Button::E))
	{
		if (gameData.insideInventoryMenu)
		{
			exitInventoryMenu();
		}
		else
		{
			gameData.insideInventoryMenu = true;
			gameData.interaction = {};
		}
	}

	//inventory and menu stuff
	if (!stopMainInput)
	{

		if (platform::isKeyPressedOn(platform::Button::R))
		{
			programData.renderer.reloadShaders();
			programData.skyBoxLoaderAndDrawer.clearOnlyGPUdata();
			programData.skyBoxLoaderAndDrawer.createGpuData();

			programData.sunRenderer.clear();
			programData.sunRenderer.create();

		}


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

			if (player.entity.fly)
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

			static float jumpTimer = 0;
			if (platform::isKeyPressedOn(platform::Button::Space)
				|| platform::getControllerButtons().buttons[platform::ControllerButtons::LBumper].pressed)
			{
				if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE)
				{
					if (jumpTimer > 0)
					{
						gameData.entityManager.localPlayer.entity.fly =
							!gameData.entityManager.localPlayer.entity.fly;

						gameData.entityManager.localPlayer.entity.forces = {};
					}
					else
					{
						jumpTimer = 0.2;
					}
				}
			}
			jumpTimer -= deltaTime;
			jumpTimer = std::max(jumpTimer, 0.f);

			if (player.entity.fly)
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
			gameData.c.rotateFPS(platform::getRelMousePosition(), 0.22f * 0.02f, rotate);

			if (!gameData.escapePressed)
			{
				platform::setRelMousePosition(w / 2, h / 2);
				gameData.c.lastMousePos = {w / 2, h / 2};
			}

			if (glm::length(glm::vec2{moveDir.x, moveDir.z}))
			{
				isPlayerMovingSpeed = 1;
			}

		}


		//keyPad
		{

			for (int i = 0; i < 9; i++)
			{
				if (platform::isKeyPressedOn(platform::Button::NR1 + i))
				{
					if (gameData.currentItemSelected != i)
					{
						AudioEngine::playSound(AudioEngine::sounds::uiSlider, UI_SOUND_VOLUME);
					}

					gameData.currentItemSelected = i;

					
				}
			}

			auto scroll = platform::getScroll();
			if (scroll < -0.5)
			{	
				if (gameData.currentItemSelected < 8)
				{
					AudioEngine::playSound(AudioEngine::sounds::uiSlider, UI_SOUND_VOLUME);
					gameData.currentItemSelected++;
				}

			}
			else if (scroll > 0.5)
			{
				if (gameData.currentItemSelected > 0)
				{
					AudioEngine::playSound(AudioEngine::sounds::uiSlider, UI_SOUND_VOLUME);
					gameData.currentItemSelected--;
				}
			}

			gameData.currentItemSelected = std::clamp(gameData.currentItemSelected, 0, 8);

		}


	#pragma region drop items

			if (platform::isKeyPressedOn(platform::Button::Q))
			{
				gameData.entityManager.dropItemByClient(
					gameData.entityManager.localPlayer.entity.position,
					gameData.currentItemSelected, gameData.undoQueue, gameData.c.viewDirection * 5.f,
					gameData.serverTimer, player.inventory, !platform::isKeyHeld(platform::Button::LeftCtrl));

				gameData.currentBlockBreaking = {};
			}

	#pragma endregion
	}



#pragma endregion


#pragma region block collisions and entity updates!
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

		
		if (gameData.killed)
		{
			gameData.entityManager.localPlayer.entity.forces = {};
			gameData.entityManager.localPlayer.entity.updatePositions();
		}
		else
		{
			gameData.entityManager.localPlayer.entity.updateForces(deltaTime, !player.entity.fly
				, settings);

			if (gameData.colidable)
			{
				auto forcesBackup = gameData.entityManager.localPlayer.entity.forces.velocity;

				gameData.entityManager.localPlayer
					.entity.resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime,
					gameData.entityManager.localPlayer.entity.getColliderSize(), settings);

				if (gameData.entityManager.localPlayer.entity.forces.colidesBottom())
				{
					gameData.entityManager.localPlayer.entity.fly = false;
				}

				auto newForces = gameData.entityManager.localPlayer.entity.forces.velocity;

				//todo no spawn damage!!!!
				{
					//fall damage
					float rez = glm::length(forcesBackup) - glm::length(newForces);

					//if (rez > 0.2)
					//{
					//	std::cout << "rez: " << rez << "\n";
					//}

					//auto b = 
					auto blockPos = from3DPointToBlock(player.entity.position - glm::dvec3(0, 0.1, 0));
					auto block = gameData.chunkSystem.getBlockSafe(blockPos.x, blockPos.y, blockPos.z);
					int sound = 0;
					if (block)
					{
						sound = getSoundForBlockStepping(block->getType());
					}

					if (rez > 17)
					{
						//high impact
						AudioEngine::playSound(AudioEngine::fallHigh, FALL_SOUND_VOLUME);

						if (sound)
						{
							AudioEngine::playSound(sound, 1);
						}
					}
					if (rez > 14)
					{
						//medium impact
						AudioEngine::playSound(AudioEngine::fallMedium, FALL_SOUND_VOLUME);

						if (sound)
						{
							AudioEngine::playSound(sound, 1);
						}
					}else
					if (rez > 10)
					{
						//low impact
						AudioEngine::playSound(AudioEngine::fallLow, FALL_SOUND_VOLUME);

						if (sound)
						{
							AudioEngine::playSound(sound, 1);
						}
					}
					else if (rez > 8)
					{
						if (sound)
						{
							AudioEngine::playSound(sound, 1);
						}
					}


					if (player.otherPlayerSettings.gameMode ==
						OtherPlayerSettings::SURVIVAL && rez > 13.2)
					{
						int fallDamage = (rez - 12.2);
						//std::cout << "fallDamage: " << fallDamage << "\n";

						dealDamageToLocalPlayer(fallDamage);
					}
				};

			}
			else
			{
				gameData.entityManager.localPlayer.entity.updatePositions();
			}
		}
		

		gameData.c.position = gameData.entityManager.localPlayer.entity.position
			+ glm::dvec3(0,1.5,0);

		gameData.entityManager.doAllUpdates(deltaTime, chunkGetter);



	}
#pragma endregion



#pragma region place blocks

	glm::ivec3 rayCastPos = {};
	std::optional<glm::ivec3> blockToPlace = std::nullopt;
	glm::dvec3 cameraRayPos = gameData.c.position;
	Block *raycastBlock = 0;
	glm::uint64 targetedEntity = 0;
	float raycastDist = 0;
	bool topPartForSlabs = 0;

	int facingDirection = gameData.c.getViewDirectionRotation();
	//std::cout << facingDirection << "\n";

	if (!stopMainInput)
	{

		if (platform::isLMouseHeld() || platform::isRMousePressed())
		{
			gameData.handHit = true;
		}

		constexpr static float TARGET_DIST = 20;

		raycastBlock = gameData.chunkSystem.rayCast(cameraRayPos, gameData.c.viewDirection,
			rayCastPos, TARGET_DIST, blockToPlace, raycastDist);
		float dist = TARGET_DIST;

		if (raycastBlock)
		{

			glm::dvec3 intersectPos = {};
			float intersectDist = 0;
			int intersectFace = 0;

			if (lineIntersectBoxGetPos(cameraRayPos, gameData.c.viewDirection, glm::dvec3(rayCastPos) -
				glm::dvec3(0, 0.5, 0), glm::dvec3(1.f), intersectPos, intersectDist, intersectFace))
			{

				//const char * names[] = {"front", "back", "top", "bottom", "left", "right"};
				//std::cout << names[intersectFace] << "\n";

				//top
				if (intersectFace == 2)
				{
					topPartForSlabs = 0;
				}
				else if (intersectFace == 3) //bottom
				{
					topPartForSlabs = 1;
				}
				else if (intersectPos.y - int(intersectPos.y) < 0.5)
				{
					topPartForSlabs = 1;
				}else
				{
					topPartForSlabs = 0;
				}
				//std::cout << (topPartForSlabs ? "top\n" : "bottom\n");

			}




			dist = raycastDist - 0.1;
		}


		float entityHitDistance = 0;
		targetedEntity = gameData.entityManager.intersectAllAttackableEntities(cameraRayPos,
			gameData.c.viewDirection, dist, entityHitDistance);

		//std::cout << targetedEntity << "\n";

		if (targetedEntity)
		{
			raycastBlock = nullptr;
			blockToPlace = {};
		}

		if (raycastBlock)
		{
			//todo special function here
			if (raycastBlock->getType() != BlockTypes::water)
			{
				programData.gyzmosRenderer.drawCube(rayCastPos);
			}
			else
			{
				gameData.currentBlockBreaking = {};
			}
		}
		else
		{
			gameData.currentBlockBreaking = {};
		}

		//place blocks and hit entities
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
						Item newItem(b->getType());

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

					static float soundTimer = 0;
					if (platform::isLMousePressed())
					{
						soundTimer = 0;
					}

					if (platform::isRMousePressed())
					{
						bool didAction = 0;
						Chunk *c = 0;
						auto b = gameData.chunkSystem.getBlockSafeAndChunk(rayCastPos.x, 
							rayCastPos.y, rayCastPos.z, c);
						if(b)
						{
							
							auto actionType = isInteractable(b->getType());

							if (actionType)
							{
								didAction = true;


								gameData.interaction = {};
								gameData.currentBlockInteractionRevisionNumber++;
								gameData.interaction.blockInteractionType = actionType;
								gameData.interaction.blockInteractionPosition = rayCastPos;

								if (actionType == InteractionTypes::structureBaseBlock)
								{
									auto baseBlock = c->blockData.getBaseBlock(
										modBlockToChunk(rayCastPos.x), rayCastPos.y, modBlockToChunk(rayCastPos.z));
									if (baseBlock)
									{
										gameData.interaction.baseBlockHolder = *baseBlock;
									}
								};


								sendBlockInteractionMessage(player.entityId, rayCastPos,
									b->getType(), gameData.currentBlockInteractionRevisionNumber);
								
								gameData.insideInventoryMenu = true;
								gameData.currentInventoryTab = 0;

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
								bool intersect = false;

								//todo intersect other entities
								if (boxColideBlock(
									player.entity.position,
									player.entity.getColliderSize(),
									*blockToPlace
									))
								{
									intersect = true;
								}

								if (!intersect)
								{
									//place block
									gameData.chunkSystem.placeBlockByClient(*blockToPlace,
										gameData.currentItemSelected,
										gameData.undoQueue,
										gameData.entityManager.localPlayer.entity.position,
										gameData.lightSystem,
										player.inventory,
										player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL,
										facingDirection, topPartForSlabs
									);

									AudioEngine::playSound(getSoundForBlockStepping(item.type),
										PLACED_BLOCK_SOUND_VOLUME);


								}
								
							}
						};

							
					}
					else if (platform::isLMouseHeld() && raycastBlock)
					{
						if (gameData.currentBlockBreaking.breaking &&
							gameData.currentBlockBreaking.tool != gameData.currentItemSelected)
						{
							gameData.currentBlockBreaking.breaking = false;
						}

						if (!gameData.currentBlockBreaking.breaking)
						{
							gameData.currentBlockBreaking.breaking = true;
							gameData.currentBlockBreaking.pos = rayCastPos;
							gameData.currentBlockBreaking.tool = gameData.currentItemSelected;

							if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL)
							{
								gameData.currentBlockBreaking.timer = computeMineDurationTime(raycastBlock->getType(),
									*player.inventory.getItemFromIndex(gameData.currentItemSelected));
								gameData.currentBlockBreaking.totalTime = gameData.currentBlockBreaking.timer;
							}
							else
							{
								gameData.currentBlockBreaking.totalTime = 0.3;
								gameData.currentBlockBreaking.timer = 0.3;

								if (platform::isLMousePressed())
								{
									gameData.currentBlockBreaking.totalTime = 0;
									gameData.currentBlockBreaking.timer = 0;
								}
							}
						}



						if (rayCastPos != gameData.currentBlockBreaking.pos)
						{
							gameData.currentBlockBreaking = {};
							soundTimer = 0;
						}
						else
						{
							gameData.currentBlockBreaking.timer -= deltaTime;
							
							if (gameData.currentBlockBreaking.timer <= 0)
							{
								auto sound = getSoundForBlockBreaking(raycastBlock->getType());
								AudioEngine::playSound(sound, BREAKED_BLOCK_SOUND_VOLUME);
								//AudioEngine::playSound(AudioEngine::crackStone, 0.3);

								//break block
								gameData.chunkSystem.breakBlockByClient(rayCastPos
									, gameData.undoQueue,
									gameData.entityManager.localPlayer.entity.position,
									gameData.lightSystem);
								gameData.currentBlockBreaking = {};
							}
							else
							{
								if (soundTimer <= 0)
								{
									soundTimer = 0.2;
									auto sound = getSoundForBlockStepping(raycastBlock->getType());
									AudioEngine::playSound(sound, MINING_BLOCK_SOUND_VOLUME);
								}
								soundTimer -= deltaTime;
							}

						}
					}
					else
					{
						soundTimer = 0;
						gameData.currentBlockBreaking = {};
					}
				}

			if (targetedEntity && platform::isLMousePressed())
			{
				
				auto weaponStats = player.inventory.getItemFromIndex(gameData.currentItemSelected)->
					getWeaponStats();
				
				if (entityHitDistance <= weaponStats.range)
				{

					//check if not creative player
					bool isCreativePlayer = 0;

					if (getEntityTypeFromEID(targetedEntity) == EntityType::player)
					{
						auto f = gameData.entityManager.players.find(targetedEntity);
						if (f != gameData.entityManager.players.end())
						{

						}

					}

					if (!isCreativePlayer)
					{
						attackEntity(targetedEntity, gameData.currentItemSelected,
							gameData.c.viewDirection);
					}

				};

			}

		};

	}
	else
	{
		gameData.currentBlockBreaking = {};
	}
	
#pragma endregion

#pragma region get player positions and stuff

	glm::ivec3 blockPositionPlayer = from3DPointToBlock(gameData.c.position);
	bool underWater = 0;
	auto inBlock = gameData.chunkSystem.getBlockSafe(blockPositionPlayer);
	if (inBlock)
	{
		if (inBlock->getType() == BlockTypes::water)
		{
			underWater = 1;
		}
	}

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	gameData.c.decomposePosition(posFloat, posInt);

#pragma endregion


	if (!stopMainInput || gameData.isInsideMapView)
	{
		if (platform::isKeyReleased(platform::Button::M))
		{
			if (gameData.isInsideMapView)
			{
				gameData.isInsideMapView = false;
				gameData.mapEngine.close();
			}
			else
			{
				gameData.isInsideMapView = true;
				gameData.mapEngine.open(programData,
					{posInt.x, posInt.z}, gameData.chunkSystem);
			}
		}
	}


#pragma region update lights

	gameData.gameplayFrameProfiler.startSubProfile("lightsSystem");
	gameData.lightSystem.update(gameData.chunkSystem);
	gameData.gameplayFrameProfiler.endSubProfile("lightsSystem");

#pragma endregion

	programData.renderer.entityRenderer.itemEntitiesToRender.push_back({gameData.entityTest});


	//
#pragma region weather and time

	static float dayTime = 0.25;
	programData.renderer.sunPos = calculateSunPosition(dayTime);
	
	//dayTime += deltaTime * 0.05f;
	//if (dayTime > 0) { dayTime -= (int)dayTime; }

#pragma endregion


#pragma region chunks and rendering

	{
		gameData.gameplayFrameProfiler.startSubProfile("chunkSystem");
		gameData.chunkSystem.update(blockPositionPlayer, deltaTime, gameData.undoQueue,
			gameData.lightSystem, gameData.interaction);
		gameData.gameplayFrameProfiler.endSubProfile("chunkSystem");
	}


#pragma region underwater water drops
	if (gameData.lastFrameInWater)
	{
		if (!underWater)
		{
			gameData.dropsStrength = 4;
		}
	}
	
	if (underWater)
	{
		gameData.dropsStrength = 0;
	}
	gameData.lastFrameInWater = underWater;

	gameData.dropsStrength -= deltaTime;
	if (gameData.dropsStrength < 0) { gameData.dropsStrength = 0; }

	float finalDropStrength = std::min(1.f, gameData.dropsStrength/3.f);
#pragma endregion


	if(w != 0 && h != 0 && !gameData.isInsideMapView)
	{

		gameData.gameplayFrameProfiler.startSubProfile("rendering");

		//programData.renderer.render(data, gameData.c, programData.texture);
		programData.renderer.renderFromBakedData(gameData.sunShadow,gameData.chunkSystem, 
			gameData.c, programData, programData.blocksLoader, gameData.entityManager,
			programData.modelsManager, 
			gameData.adaptiveExposure, gameData.showLightLevels,
			gameData.point, underWater, w, h, deltaTime, dayTime, gameData.currentSkinBindlessTexture,
			gameData.handHit, isPlayerMovingSpeed, gameData.playerFOVHandTransform,
			gameData.currentItemSelected, finalDropStrength, 
			gameData.showUI
			);


		if (gameData.currentBlockBreaking.breaking)
		{
			programData.renderer.renderDecal(rayCastPos, gameData.c, *raycastBlock,
				programData, 1-(gameData.currentBlockBreaking.timer / gameData.currentBlockBreaking.totalTime));
		}


		gameData.c.lastFrameViewProjMatrix =
			gameData.c.getProjectionMatrix() * gameData.c.getViewMatrix();

		gameData.gameplayFrameProfiler.endSubProfile("rendering");
	}
#pragma endregion


#pragma region drop entities that are too far
	//todo we shouldn't drop players!!!!!!,
	//we should just not update/draw them instead
	gameData.entityManager.dropEntitiesThatAreTooFar({blockPositionPlayer.x,blockPositionPlayer.z},
		gameData.chunkSystem.squareSize);


#pragma endregion


//steppings sound
#pragma region steppings sounds

	{
		static float timer = 0;

		const float shortestTimer = 0.3;
		const float longestTimer = 0.7;

		//player.entity.forces.colidesBottom() && 
		if (isPlayerMovingSpeed)
		{
			auto blockPos = from3DPointToBlock(player.entity.position - glm::dvec3(0, 0.1, 0));
			auto block = gameData.chunkSystem.getBlockSafe(blockPos.x, blockPos.y, blockPos.z);

			if (block)
			{
				auto sound = getSoundForBlockStepping(block->getType());

				if (sound)
				{
					if (timer <= 0)
					{
						timer = glm::mix(longestTimer, shortestTimer, isPlayerMovingSpeed);
						AudioEngine::playSound(sound, STEPPING_SOUND_VOLUME);
					}
					timer -= deltaTime;
				}
				else
				{
					timer = 0;
				}
			}
			else
			{
				timer = 0;
			}
		}
		else
		{
			timer = 0;
		}
	}

#pragma endregion

	
#pragma region debug and gyzmos stuff
	
	if (!gameData.isInsideMapView)
	{

		programData.GPUProfiler.startSubProfile("Debug rendering");


		programData.pointDebugRenderer.renderCubePoint(gameData.c, gameData.point);

		if (gameData.renderBox)
		{
			//programData.gyzmosRenderer.drawCube(from3DPointToBlock(point));
			programData.gyzmosRenderer.drawCube(gameData.point);

			if (gameData.pointSize != glm::ivec3{})
			{
				programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize)));
				programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(1, 0, 0)));
				programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(0, 1, 0)));
				programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(0, 0, 1)));
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

		auto drawBox = [&](glm::dvec3 pos, glm::vec3 boxSize)
		{
			programData.gyzmosRenderer.drawLine(pos,
				pos + glm::dvec3(boxSize.x, 0, 0));

			programData.gyzmosRenderer.drawLine(pos,
				pos + glm::dvec3(0, 0, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x, 0, 0),
				pos + glm::dvec3(boxSize.x, 0, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0, 0, boxSize.z),
				pos + glm::dvec3(boxSize.x, 0, boxSize.z));



			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0,boxSize.y,0),
				pos + glm::dvec3(boxSize.x, boxSize.y, 0));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0, boxSize.y, 0),
				pos + glm::dvec3(0, boxSize.y, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x, boxSize.y, 0),
				pos + glm::dvec3(boxSize.x, boxSize.y, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0, boxSize.y, boxSize.z),
				pos + glm::dvec3(boxSize.x, boxSize.y, boxSize.z));


			programData.gyzmosRenderer.drawLine(pos,
				pos + glm::dvec3(0, boxSize.y, 0));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x, 0, 0),
				pos + glm::dvec3(boxSize.x, boxSize.y, 0));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0, 0, boxSize.z),
				pos + glm::dvec3(0, boxSize.y, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x, 0, boxSize.z),
				pos + glm::dvec3(boxSize.x, boxSize.y, boxSize.z));
		};

		if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE)
		{
			//todo sort the chunks in the chunk system once and than keep that vector because we need it
			int maxCount = 100;
			for (auto &c : gameData.chunkSystem.loadedChunks)
			{
				if (c)
				{

					for (auto &b : c->blockData.baseBlocks)
					{

						glm::ivec3 pos = fromHashValueToBlockPosinChunk(b.first);
						glm::vec3 size = {b.second.sizeX,b.second.sizeY,b.second.sizeZ};

						if (size.x != 0 && size.y != 0 && size.z != 0)
						{
							maxCount--;

							glm::dvec3 posD = pos + glm::ivec3(c->data.x * CHUNK_SIZE,0, c->data.z * CHUNK_SIZE);
							posD += glm::dvec3(0.5, -0.5, 0.5);
							posD += glm::ivec3(b.second.offsetX, b.second.offsetY, b.second.offsetZ);

							drawBox(posD, size);
						}

						if (maxCount <= 0)
						{
							break;
						}
					}


				}

				if (maxCount <= 0)
				{
					break;
				}
			}
		};


		if (gameData.renderColliders)
		{

			gameData.entityManager.renderColiders(programData.pointDebugRenderer, programData.gyzmosRenderer, 
				gameData.c);

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
	}

#pragma endregion


	auto centerChunk = gameData.chunkSystem.getChunkSafeFromBlockPos(posInt.x, posInt.z);

#pragma region imgui
	
	bool terminate = false;

#if REMOVE_IMGUI == 0

	if (programData.showImgui)
	{
		gameData.gameplayFrameProfiler.startSubProfile("imgui");

		//if (ImGui::Begin("camera controll", &gameData.escapePressed))
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {26/255.f,26/255.f,26/255.f,0.5f});
		if (ImGui::Begin("client controll"))
		{

			ImGui::SliderFloat("Day time", &dayTime, 0, 1);

			if (centerChunk)
			{
				ImGui::Text("Vegetaion: %f", centerChunk->data.vegetation);
				ImGui::Text("CellValue X: %d", centerChunk->data.regionCenterX);
				ImGui::Text("CellValue Z: %d", centerChunk->data.regionCenterZ);
			}
	
			int l = player.life.life;
			ImGui::SliderInt("Player Life", &l, 0, 20);
			player.life.life = l;

			if (ImGui::CollapsingHeader("Camera stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Checkbox("Colidable", &gameData.colidable);

				ImGui::DragScalarN("Player Body pos", ImGuiDataType_Double,
					&gameData.entityManager.localPlayer.entity.position[0], 3, 0.01);
				gameData.entityManager.localPlayer.entity.lastPosition 
					= gameData.entityManager.localPlayer.entity.position;

				ImGui::Text("Entity pos: %lf, %lf, %lf", player.entity.position.x, player.entity.position.y, player.entity.position.z);
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
				ImGui::SliderFloat("exposure", &gameData.adaptiveExposure.currentExposure, 0.001, 10);
				ImGui::Combo("Tonemapper", &programData.renderer.defaultShader.
					shadingSettings.tonemapper, "ACES\0AgX\0ZCAM\0");

				ImGui::SliderFloat3("Sky pos", &programData.renderer.sunPos[0], -1, 1);

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

				if (glm::length(programData.renderer.sunPos[0]) != 0)
				{
					programData.renderer.sunPos = 
						glm::normalize(programData.renderer.sunPos);
				}
				else
				{
					programData.renderer.sunPos = glm::vec3(0, -1, 0);
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


				if (ImGui::Button("OPEN FILE DIALOGUE"))
				{
					ShowOpenFileDialog(0, fileBuff, sizeof(fileBuff),
						RESOURCES_PATH "gameData/structures/",
						".structure");
				}


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
									s->unsafeGet(x, y, z) = rez->getType();
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

									Block block;
									block.setType(s->unsafeGet(x, y, z));

									//todo implement the bulk version...
									gameData.chunkSystem.placeBlockByClientForce(pos,
										block, gameData.undoQueue,
										gameData.lightSystem);

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

			if (ImGui::CollapsingHeader("Filtered bloom color",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboMain.fourthColor, {256, 256},
					{0, 1}, {1, 0});

				ImGui::Image((void *)programData.renderer.bluredColorBuffer[0], {256, 256},
					{0, 1}, {1, 0});

				ImGui::Image((void *)programData.renderer.bluredColorBuffer[1], {256, 256},
					{0, 1}, {1, 0});

				ImGui::SliderFloat("Multiplier", &programData.renderer.bloomMultiplier, 0.0001, 3);
				ImGui::SliderFloat("Tresshold", &programData.renderer.bloomTresshold, 0.0001, 10);
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

			ImGui::Checkbox("Render Transparent",
				&programData.renderer.renderTransparent);

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

			ImGui::Checkbox("Bloom", &programData.renderer.bloom);


			if (ImGui::CollapsingHeader("Music ",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				
				if (ImGui::Button("Play random night music"))
				{
					AudioEngine::playRandomNightMusic();
				}

				if (ImGui::Button("Play sound"))
				{
					AudioEngine::playSound(AudioEngine::toolBreakingStone, 1);
				}

				if (ImGui::Button("Play sound2"))
				{
					AudioEngine::playSound(AudioEngine::grass, 1);
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


#pragma region ui

	int cursorSelected = -2;
	unsigned short selectedCreativeItem = 0;
	int craftedItemIndex = -1;

	if (!gameData.isInsideMapView)
	{

		if (gameData.interaction.blockInteractionType == InteractionTypes::structureBaseBlock)
		{
			if (programData.ui.renderBaseBlockUI(deltaTime, w, h, programData,
				gameData.interaction.baseBlockHolder))
			{

				auto block = gameData.chunkSystem.getBlockSafe(gameData.interaction.blockInteractionPosition.x, gameData.interaction.blockInteractionPosition.y,
					gameData.interaction.blockInteractionPosition.z);

				auto c = gameData.chunkSystem.
					getChunkSafeFromBlockPos(gameData.interaction.blockInteractionPosition.x,
					gameData.interaction.blockInteractionPosition.z);

				if (c && block)
				{
					auto blockData = c->blockData.getOrCreateBaseBlock(
						modBlockToChunk(gameData.interaction.blockInteractionPosition.x),
						gameData.interaction.blockInteractionPosition.y,
						modBlockToChunk(gameData.interaction.blockInteractionPosition.z));

					*blockData = gameData.interaction.baseBlockHolder;

					std::vector<unsigned char> vectorData;
					blockData->formatIntoData(vectorData);

					gameData.undoQueue.changedBlockDataEvent(gameData.interaction.blockInteractionPosition,
						*block, vectorData);

					//todo send packet
				}

			}
		}
		else
		{
			programData.ui.renderGameUI(deltaTime, w, h, gameData.currentItemSelected,
				player.inventory, programData.blocksLoader, gameData.insideInventoryMenu,
				cursorSelected, 
				(gameData.interaction.blockInteractionType == InteractionTypes::craftingTable),
				gameData.currentInventoryTab, player.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE,
				selectedCreativeItem, player.life, programData, player, 
				gameData.craftingSlider, craftedItemIndex, gameData.showUI
			);
		}


	}
	else
	{

		gameData.mapEngine.update(programData, deltaTime, {posInt.x, posInt.z},
			gameData.chunkSystem);

	}

#pragma endregion

#pragma region crafting

	if (gameData.insideInventoryMenu)
	{
		
		if (craftedItemIndex >= 0)
		{

			if (platform::isLMousePressed())
			{
				
				if (recepieExists(craftedItemIndex))
				{

					auto recepie = getRecepieFromIndexUnsafe(craftedItemIndex);


					if (canItemBeCrafted(recepie, player.inventory))
					{

						if (canItemBeMovedToAndMoveIt(recepie.result,
							*player.inventory.getItemFromIndex(PlayerInventory::CURSOR_INDEX)))
						{
							Packet_ClientCraftedItem packet;
							packet.recepieIndex = craftedItemIndex;
							packet.to = PlayerInventory::CURSOR_INDEX;
							packet.revisionNumber = player.inventory.revisionNumber;

							sendPacket(getServer(), headerClientCraftedItem, player.entityId,
								&packet, sizeof(packet), true, channelChunksAndBlocks);


							craftItemUnsafe(recepie, player.inventory);

						}

						

					};

				}

			}

		}


	}


#pragma endregion

	//std::cout << cursorSelected << "\n";

#pragma region move items in inventory
	if (gameData.insideInventoryMenu && !gameData.escapePressed)
	{

		//pickup blocks or items from the creative inventory
		if (selectedCreativeItem)
		{

			if (!player.inventory.heldInMouse.type)
			{
				if (platform::isLMousePressed())
				{
					Item item = itemCreator(selectedCreativeItem);
					
					if (platform::isKeyHeld(platform::Button::LeftCtrl))
					{
						item.counter = item.getStackSize();
					}

					//todo force overwrite item with metadata here!
					forceOverWriteItem(player.inventory, PlayerInventory::CURSOR_INDEX,
						item);

					player.inventory.heldInMouse = item;
				}
			}

		}
		else
		{
			static std::bitset<64> rightClickedThisClick = 0;

			if (platform::isLMousePressed())
			{
				//crafting
				//

				//grab items
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
						grabItem(player.inventory, cursorSelected, PlayerInventory::CURSOR_INDEX, selected->counter / 2);

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
	else if (gameData.isInsideMapView)
	{
		if (platform::isKeyReleased(platform::Button::Escape))
		{
			gameData.isInsideMapView = false;
			gameData.mapEngine.close();
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

	if (gameData.killed)
	{
		programData.ui.renderer2d.renderRectangle({0,0, w,h}, {0.9,0,0,0.5});
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

		displaySettingsMenuButton(programData);

		displaySkinSelectorMenuButton(programData);

		if (programData.ui.menuRenderer.Button("Exit", Colors_Gray, programData.ui.buttonTexture))
		{
			terminate = true;
		}

		programData.ui.menuRenderer.End();

		if (programData.ui.menuRenderer.internal.allMenuStacks[2].size() == 0 && 
			platform::isKeyReleased(platform::Button::Escape) && !justPressedEsc)
		{
			gameData.escapePressed = false;
		}

	}
	else if(gameData.killed)
	{
		programData.ui.menuRenderer.Begin(3);
		programData.ui.menuRenderer.SetAlignModeFixedSizeWidgets({0,150});

		programData.ui.menuRenderer.Text("You died :(", Colors_White);

		if (programData.ui.menuRenderer.Button("Respawn", Colors_Gray, programData.ui.buttonTexture))
		{
			sendPacket(getServer(), headerClientWantsToRespawn, player.entityId,
				0, 0, true, channelChunksAndBlocks);
		}

		if (programData.ui.menuRenderer.Button("Exit", Colors_Gray, programData.ui.buttonTexture))
		{
			terminate = true;
		}

		programData.ui.menuRenderer.End();
	}

	platform::showMouse(gameData.escapePressed || gameData.insideInventoryMenu ||
		gameData.killed || gameData.isInsideMapView);


#pragma endregion

	gameData.chunkSystem.changeRenderDistance(getShadingSettings().viewDistance * 2);


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
	gameData.currentSkinTexture.cleanup();
	gameData.entityManager.cleanup();
	
	gameData.mapEngine.close();

	gameData = GameData(); //free all resources
}