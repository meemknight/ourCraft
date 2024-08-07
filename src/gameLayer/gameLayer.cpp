#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include <iostream>
#include <sstream>
#include "rendering/camera.h"
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
#include "gamePlayLogic.h"
#include "multyPlayer/splitUpdatesLogic.h"
#include <rendering/renderSettings.h>
#include <ourJson.h>
#include <filesystem>

#include <platformTools.h>

#if REMOVE_IMGUI == 0
#include "imgui.h"
#endif


ProgramData programData;


inline void stubErrorFunc(const char *msg, void *userDefinedData)
{}

void clearOtherTextures()
{
	programData.numbersTexture.cleanup();
	programData.dudv.cleanup();
	programData.causticsTexture.cleanup();
	programData.dudvNormal.cleanup();
	programData.aoTexture.cleanup();
	programData.brdfTexture.cleanup();
}

void clearAllTexturePacks()
{
	clearOtherTextures();
	programData.skyBoxLoaderAndDrawer.clearOnlyTextures();
	programData.ui.clearOnlyTextures();
	programData.modelsManager.clearAllModels();
}

void loadOtherTextures(const char *basePath, bool noErrorReport)
{
	if (noErrorReport)
	{
		gl2d::setErrorFuncCallback(stubErrorFunc);
	}

	std::string p(basePath);

	if (!programData.numbersTexture.id)
	{
		programData.numbersTexture.loadFromFile((p + "numbers.png").c_str(), true, true);
		//programData.dudv.loadFromFile(RESOURCES_PATH "assets/otherTextures/test.jpg", true, true);
	};

	if (!programData.dudv.id)
	{
		programData.dudv.loadFromFile((p + "waterDUDV.png").c_str(), false, true);
		//programData.dudv.loadFromFile(RESOURCES_PATH "assets/otherTextures/wdudv.jpg", false, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	};

	if (!programData.causticsTexture.id)
	{
		programData.causticsTexture.loadFromFile((p + "caustics.jpg").c_str(), false, true);
		//programData.causticsTexture.loadFromFile(RESOURCES_PATH "assets/otherTextures/caustics3.png", false, true);
		//programData.causticsTexture.loadFromFile(RESOURCES_PATH "assets/otherTextures/test.jpg", false, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	};

	if (!programData.dudvNormal.id)
	{
		//programData.dudvNormal.loadFromFile(RESOURCES_PATH "assets/otherTextures/normal.png", false, true);
		//programData.dudvNormal.loadFromFile(RESOURCES_PATH "assets/otherTextures/normal2.png", false, true);
		//programData.dudvNormal.loadFromFile(RESOURCES_PATH "assets/otherTextures/normal.jpg", false, true);
		//programData.dudvNormal.loadFromFile(RESOURCES_PATH "assets/otherTextures/normal2.jpg", false, true);
		programData.dudvNormal.loadFromFile((p+"normal3.png").c_str(), false, true); //best
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	};


	if (!programData.aoTexture.id)
	{
		programData.aoTexture.loadFromFile((p+"ao.png").c_str(), false, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	};

	if (!programData.brdfTexture.id)
	{
		programData.brdfTexture.loadFromFile((p + "brdf.png").c_str(), false, false);
	}

	if (noErrorReport)
	{
		gl2d::setErrorFuncCallback(gl2d::defaultErrorFunc);
	}
}

void loadAllDefaultTexturePacks()
{
	loadOtherTextures(RESOURCES_PATH "assets/otherTextures/", false); //load the defaults

	programData.skyBoxLoaderAndDrawer.loadAllTextures(RESOURCES_PATH "assets/sky/");

	programData.ui.loadTextures(RESOURCES_PATH "assets/ui/");

	programData.modelsManager.loadAllModels(RESOURCES_PATH "assets/models/", false);
}

bool loadTexturePack(const char *basePath)
{

	std::filesystem::path root(basePath);

	if (std::filesystem::exists(root) &&
		std::filesystem::is_directory(root))
	{


		std::filesystem::path otherTexturesPath = root;
		otherTexturesPath /= "otherTextures/";

		if (std::filesystem::exists(otherTexturesPath) &&
			std::filesystem::is_directory(otherTexturesPath))
		{

			loadOtherTextures(otherTexturesPath.string().c_str(), true);
		}


		std::filesystem::path skyPath = root;
		skyPath /= "sky/";

		if (std::filesystem::exists(skyPath) &&
			std::filesystem::is_directory(skyPath))
		{
			gl2d::setErrorFuncCallback(stubErrorFunc);
			programData.skyBoxLoaderAndDrawer.loadAllTextures(skyPath.string().c_str());
			gl2d::setErrorFuncCallback(gl2d::defaultErrorFunc);
		}

		std::filesystem::path uiPath = root;
		uiPath /= "ui/";

		if (std::filesystem::exists(uiPath) &&
			std::filesystem::is_directory(uiPath))
		{
			gl2d::setErrorFuncCallback(stubErrorFunc);
			programData.ui.loadTextures(uiPath.string().c_str());
			gl2d::setErrorFuncCallback(gl2d::defaultErrorFunc);
		}

		std::filesystem::path modelsPath = root;
		modelsPath /= "models/";

		if (std::filesystem::exists(modelsPath) &&
			std::filesystem::is_directory(modelsPath))
		{
			gl2d::setErrorFuncCallback(stubErrorFunc);
			programData.modelsManager.loadAllModels(modelsPath.string().c_str(), false);
			gl2d::setErrorFuncCallback(gl2d::defaultErrorFunc);
		}
	}
	else
	{
		return 0;
	}

	return true;
}

bool initGame()
{

	createErrorFile();

	programData.GPUProfiler.initGPUProfiler();


	gl2d::setVsync(false);



	programData.ui.init();
	programData.blocksLoader.loadAllTextures();
	programData.renderer.create(programData.blocksLoader);
	programData.gyzmosRenderer.create();
	programData.pointDebugRenderer.create();
	programData.skyBoxLoaderAndDrawer.createGpuData();


	loadAllDefaultTexturePacks();


	programData.defaultCover.loadFromFile(RESOURCES_PATH "defaultCover.png");


	if (enet_initialize() != 0)
	{
		reportError("problem starting ENET");
		return false;
	}
	
	//programData.facesCount = blockData.size() / 4;

	//glNamedBufferData(programData.renderer.vertexBuffer, 0, 0, GL_DYNAMIC_DRAW);

	//glEnable(GL_LINE_WIDTH);
	glLineWidth(4);


	KeyValuePair settings;
	settings.loadElementsFromFile(RESOURCES_PATH "test.txt");
	settings.printAll();
	settings.writeIntoFile(RESOURCES_PATH "test2.txt");

	return true;
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getWindowSizeX();
	h = platform::getWindowSizeY();

	programData.ui.renderer2d.updateWindowMetrics(w, h);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#pragma endregion

#pragma region fps
	static float timeCounter = 0;
	static int frameCounter = 0;

	timeCounter += deltaTime;
	if(timeCounter >= 1.f)
	{
		timeCounter -= 1;
		programData.currentFps = frameCounter;
		frameCounter = 0;
	}
	frameCounter++;

#pragma endregion
	
	if (shouldReloadTexturePacks())
	{
		clearAllTexturePacks();

		auto tp = getUsedTexturePacksAndResetFlag();

		for (auto &t : tp)
		{
			std::string path = RESOURCES_PATH;
			path += "/texturePacks/";
			path += t.filename().string();
			loadTexturePack(path.c_str());
		}

		loadAllDefaultTexturePacks();
	}

	if (platform::isKeyPressedOn(platform::Button::P))
	{
		platform::setFullScreen(!platform::isFullScreen());
	}

	static bool gameStarted = 0;
	static std::string lastError = "";
	static char ipString[50] = {};

	if (!gameStarted)
	{
		programData.ui.menuRenderer.Begin(1);
		programData.ui.menuRenderer.SetAlignModeFixedSizeWidgets({0,150});

		if (programData.ui.menuRenderer.Button("Host game", Colors_Gray, programData.ui.buttonTexture))
		{
			if (!startServer())
			{
				lastError = "Problem starting server";
			}
			else
			{
				gameStarted = true;
				if (!initGameplay(programData, nullptr))
				{
					closeServer();
					lastError = "Coultn't join, closing server";
					gameStarted = false;
				}
			}

		}
		if (programData.ui.menuRenderer.Button("Join game", Colors_Gray,
			programData.ui.buttonTexture))
		{
			if (initGameplay(programData, ipString))
			{
				gameStarted = true;
			}
			else
			{
				lastError = "Couldn't join server";
			}
		}
		
		programData.ui.menuRenderer.InputText("IP: ", ipString, sizeof(ipString),
			Colors_Gray, programData.ui.buttonTexture, false);


		displayRenderSettingsMenuButton(programData);

		if (!lastError.empty())
		{
			programData.ui.menuRenderer.Text(lastError, glm::vec4(1, 0, 0, 1));
		}

		programData.ui.menuRenderer.End();
	}
	else
	{
		if (isServerRunning())
		{

		#if REMOVE_IMGUI == 0

			auto s = getServerSettingsCopy();

			ImGui::PushStyleColor(ImGuiCol_WindowBg, {26 / 255.f,26 / 255.f,26 / 255.f,0.5f});
			ImGui::Begin("Server window");

			ImGui::Text("Server Chunk Capacity: %d", getChunkCapacity());
			ImGui::Text("Server Ticke per seccond: %d", getServerTicksPerSeccond());
			ImGui::Text("Server Worker tick threads: %d", getThredPoolSize());
			
			for (auto &c : s.perClientSettings)
			{
				ImGui::PushID(c.first);
				ImGui::Text("%d", c.first);
				ImGui::Text("Position: %lf %lf %lf", c.second.outPlayerPos.x,
					c.second.outPlayerPos.y, c.second.outPlayerPos.z);
				ImGui::Checkbox("Allow validate moves", &c.second.validateStuff);
				ImGui::Checkbox("Spawn zombie", &c.second.spawnZombie);
				ImGui::Checkbox("Spawn pig", &c.second.spawnPig);
				ImGui::Checkbox("Resend inventory", &c.second.resendInventory);
				ImGui::Separator();
				ImGui::PopID();
			}


			ImGui::End();
			ImGui::PopStyleColor();

			setServerSettings(s);

		#endif

		}

		if (!gameplayFrame(deltaTime, w, h, programData))
		{
			closeGameLogic();
			closeConnection();
			closeServer();		//this will do something only if the server is on
			gameStarted = false;
			platform::showMouse(true);
		}

	}



#pragma region set finishing stuff
	gl2d::enableNecessaryGLFeatures();

	programData.ui.menuRenderer.renderFrame(programData.ui.renderer2d, programData.ui.font, platform::getRelMousePosition(),
		platform::isLMousePressed(), platform::isLMouseHeld(), platform::isLMouseReleased(),
		platform::isKeyReleased(platform::Button::Escape), platform::getTypedInput(), deltaTime);

	programData.ui.renderer2d.flush();

	return true;
#pragma endregion

}

void closeGame()
{
	closeGameLogic();
	closeConnection();
	closeServer();

}
