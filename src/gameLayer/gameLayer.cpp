#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
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
#include "rendering/Ui.h"
#include "glui/glui.h"
#include "gamePlayLogic.h"

#define GPU_ENGINE 0
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = GPU_ENGINE;
}

ProgramData programData;



bool initGame()
{
	enableReportGlErrors();
	
	glui::gluiInit();

	gl2d::setVsync(false);
	programData.renderer2d.create();
	programData.font.createFromFile(RESOURCES_PATH "roboto_black.ttf");
	programData.texture.loadFromFile(RESOURCES_PATH "blocks.png", true);
	programData.uiTexture.loadFromFile(RESOURCES_PATH "ui0.png", true, true);

	programData.renderer.create();
	programData.gyzmosRenderer.create();
	

	//todo create error function
	if (enet_initialize() != 0)
	{
		std::cout << "problem starting enet\n";
		return false;
	}


	//programData.facesCount = blockData.size() / 4;

	//glNamedBufferData(programData.renderer.vertexBuffer, 0, 0, GL_DYNAMIC_DRAW);

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

	programData.renderer2d.updateWindowMetrics(w, h);


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
	

	if (platform::isKeyPressedOn(platform::Button::P))
	{
		platform::setFullScreen(!platform::isFullScreen());
	}

	static bool gameStarted = 0;
	static std::string lastError = "";
	
	if (!gameStarted)
	{
		glui::Begin(1);
		if (glui::Button("Host game", Colors_Gray))
		{
			if (!startServer())
			{
				lastError = "Problem starting server";
			}
			else
			{
				gameStarted = true;
				if (!initGameplay(programData))
				{
					closeServer();
					lastError = "Coultn't join, closing server";
					gameStarted = false;
				}
			}

		}
		if (glui::Button("Join game", Colors_Gray))
		{
			if (initGameplay(programData))
			{
				gameStarted = true;
			}
			else
			{
				lastError = "Couldn't join server";
			}
		}

		if (!lastError.empty())
		{
			glui::Text(lastError, glm::vec4(1, 0, 0, 1));
		}

		glui::End();
	}
	else
	{
		if (!gameplayFrame(deltaTime, w, h, programData))
		{
			closeGameLogic();
			closeConnection();
			closeServer();
			gameStarted = false;
		}
	}



#pragma region set finishing stuff
	gl2d::enableNecessaryGLFeatures();

	glui::renderFrame(programData.renderer2d, programData.font, platform::getRelMousePosition(),
		platform::isLMousePressed(), platform::isLMouseHeld(), platform::isLMouseReleased(),
		platform::isKeyReleased(platform::Button::Escape), platform::getTypedInput(), deltaTime);

	programData.renderer2d.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	closeGameLogic();

}
