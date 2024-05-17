#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image/stb_image.h>
#include <stb_truetype/stb_truetype.h>
#include "gl2d/gl2d.h"
#include <iostream>
#include <ctime>
#include "platformTools.h"
#include "config.h"
#include <raudio.h>
#include "platformInput.h"
#include "otherPlatformFunctions.h"
#include "gameLayer.h"
#include <fstream>
#include <chrono>
#include <profilerLib/include/profilerLib.h>

#ifdef _WIN32
#define GPU_ENGINE 1
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = GPU_ENGINE;
}
#endif


#if REMOVE_IMGUI == 0
	#include "imgui.h"
	#include "backends/imgui_impl_glfw.h"
	#include "backends/imgui_impl_opengl3.h"
	#include "imguiThemes.h"
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

#undef min
#undef max

#pragma region globals 
bool currentFullScreen = 0;
bool fullScreen = 0;

#pragma endregion


void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{

	if ((action == GLFW_REPEAT || action == GLFW_PRESS) && key == GLFW_KEY_BACKSPACE)
	{
		platform::internal::addToTypedInput(8);
	}

	bool state = 0;

	if(action == GLFW_PRESS)
	{
		state = 1;
	}else if(action == GLFW_RELEASE)
	{
		state = 0;
	}else
	{
		return;
	}

	if(key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
	{
		int index = key - GLFW_KEY_A;
		platform::internal::setButtonState(platform::Button::A + index, state);
	}else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
	{
		int index = key - GLFW_KEY_0;
		platform::internal::setButtonState(platform::Button::NR0 + index, state);
	}else
	{
	//special keys
		//GLFW_KEY_SPACE, GLFW_KEY_ENTER, GLFW_KEY_ESCAPE, GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT

		if (key == GLFW_KEY_SPACE)
		{
			platform::internal::setButtonState(platform::Button::Space, state);
		}
		else
		if (key == GLFW_KEY_ENTER)
		{
			platform::internal::setButtonState(platform::Button::Enter, state);
		}
		else
		if (key == GLFW_KEY_ESCAPE)
		{
			platform::internal::setButtonState(platform::Button::Escape, state);
		}
		else
		if (key == GLFW_KEY_UP)
		{
			platform::internal::setButtonState(platform::Button::Up, state);
		}
		else
		if (key == GLFW_KEY_DOWN)
		{
			platform::internal::setButtonState(platform::Button::Down, state);
		}
		else
		if (key == GLFW_KEY_LEFT)
		{
			platform::internal::setButtonState(platform::Button::Left, state);
		}
		else
		if (key == GLFW_KEY_RIGHT)
		{
			platform::internal::setButtonState(platform::Button::Right, state);
		}
		else
		if (key == GLFW_KEY_LEFT_CONTROL)
		{
			platform::internal::setButtonState(platform::Button::LeftCtrl, state);
		}
		else
		if (key == GLFW_KEY_LEFT_ALT)
		{
			platform::internal::setButtonState(platform::Button::LeftAlt, state);
		}
		else
		if (key == GLFW_KEY_LEFT_SHIFT)
		{
			platform::internal::setButtonState(platform::Button::LeftShift, state);
		}
	}
	
};

void mouseCallback(GLFWwindow *window, int key, int action, int mods)
{
	bool state = 0;

	if (action == GLFW_PRESS)
	{
		state = 1;
	}
	else if (action == GLFW_RELEASE)
	{
		state = 0;
	}
	else
	{
		return;
	}

	if(key == GLFW_MOUSE_BUTTON_LEFT)
	{
		platform::internal::setLeftMouseState(state);
	}else
	if (key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		platform::internal::setRightMouseState(state);
	}
	

}

bool windowFocus = 1;

void windowFocusCallback(GLFWwindow *window, int focused)
{
	if (focused)
	{
		windowFocus = 1;
	}
	else
	{
		windowFocus = 0;
		//if you not capture the release event when the window loses focus,
		//the buttons will stay pressed
		platform::internal::resetInputsToZero();
	}
}

void windowSizeCallback(GLFWwindow *window, int x, int y)
{
	platform::internal::resetInputsToZero();
}

int mouseMovedFlag = 0;

void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
{
	mouseMovedFlag = 1;
}

void characterCallback(GLFWwindow *window, unsigned int codepoint)
{
	if (codepoint < 127)
	{
		platform::internal::addToTypedInput(codepoint);
	}
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);


#pragma region platform functions

GLFWwindow *wind = 0;

namespace platform
{

	void setRelMousePosition(int x, int y)
	{
		glfwSetCursorPos(wind, x, y);
	}

	bool isFullScreen()
	{
		return fullScreen;
	}

	void setFullScreen(bool f)
	{
		fullScreen = f;
	}

	glm::ivec2 getRelMousePosition()
	{
		double x = 0, y = 0;
		glfwGetCursorPos(wind, &x, &y);
		return { x, y };
	}

	glm::ivec2 getWindowSize()
	{
		int x = 0; int y = 0;
		glfwGetWindowSize(wind, &x, &y);
		return { x, y };
	}

	void showMouse(bool show)
	{
		static bool lastValue = true;
		if(show)
		{
			if (!lastValue)
			{
			
				glfwSetInputMode(wind, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				glfwSetInputMode(wind, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);

			}
			lastValue = true;
		}else
		{
			if (lastValue)
			{
				auto size = getWindowSize();
				glfwSetCursorPos(wind, size.x / 2, size.y / 2);
				//glfwSetInputMode(wind, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				glfwSetInputMode(wind, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetInputMode(wind, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

			}

			lastValue = false;
		}
	}

	bool isFocused()
	{
		return windowFocus;
	}

	bool mouseMoved()
	{
		return mouseMovedFlag;
	}

	bool writeEntireFile(const char *name, void *buffer, size_t size)
	{
		std::ofstream f(name, std::ios::binary);

		if(!f.is_open())
		{
			return 0;
		}

		f.write((char*)buffer, size);

		f.close();

		return 1;
	}


	bool readEntireFile(const char *name, void *buffer, size_t size)
	{
		std::ifstream f(name, std::ios::binary);

		if (!f.is_open())
		{
			return 0;
		}

		f.read((char *)buffer, size);

		f.close();

		return 1;
	}


};
#pragma endregion


int main()
{

#ifdef _WIN32
#ifdef _MSC_VER 
#if INTERNAL_BUILD
	AllocConsole();
	(void)freopen("conin$", "r", stdin);
	(void)freopen("conout$", "w", stdout);
	(void)freopen("conout$", "w", stderr);
	std::cout.sync_with_stdio();
#endif
#endif
#endif


//#ifdef _WIN32
//	timeBeginPeriod(1);
//	atexit([]() { timeEndPeriod(1); });
//#endif


#pragma region window and opengl

	permaAssertComment(glfwInit(), "err initializing glfw");
	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	int w = 500;
	int h = 500;
	wind = glfwCreateWindow(w, h, "geam", nullptr, nullptr);
	glfwMakeContextCurrent(wind);
	//glfwSwapInterval(1);

	glfwSetKeyCallback(wind, keyCallback);
	glfwSetMouseButtonCallback(wind, mouseCallback);
	glfwSetWindowFocusCallback(wind, windowFocusCallback);
	glfwSetWindowSizeCallback(wind, windowSizeCallback);
	glfwSetCursorPosCallback(wind, cursorPositionCallback);
	glfwSetCharCallback(wind, characterCallback);
	glfwSetScrollCallback(wind, scrollCallback);

	permaAssertComment(gladLoadGL(), "err initializing glad");

	//enableReportGlErrors();

#pragma endregion

#pragma region gl2d
	gl2d::init();
#pragma endregion


#pragma region imgui
	#if REMOVE_IMGUI == 0
		ImGui::CreateContext();
		//ImGui::StyleColorsDark();
		imguiThemes::embraceTheDarkness();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;
	
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			//style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 0.f; //always
			style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f; //always
		}
	
		ImGui_ImplGlfw_InitForOpenGL(wind, true);
		ImGui_ImplOpenGL3_Init("#version 330");

		//ImGuiIO &io = ImGui::GetIO();
		//io.Fonts->AddFontFromFileTTF("path/to/your/font.ttf", 24.0f);

		ImFont *font = io.Fonts->AddFontDefault(&ImFontConfig());
		font->Scale = 1.5f; // Scale factor to make the default font larger
		io.FontDefault = font;
	#endif
#pragma endregion

#pragma region audio
	InitAudioDevice();

	//Music m = LoadMusicStream(RESOURCES_PATH "target.ogg");
	Music m = {};
	UpdateMusicStream(m);
	//StopMusicStream(m);
	PlayMusicStream(m);

#pragma endregion

#pragma region initGame
	if (!initGame())
	{
		return 0;
	}
#pragma endregion


	//long lastTime = clock();
	
	auto stop = std::chrono::high_resolution_clock::now();

	while (!glfwWindowShouldClose(wind))
	{
		UpdateMusicStream(m);
		PlayMusicStream(m);

	#pragma region deltaTime

		//long newTime = clock();
		//float deltaTime = (float)(newTime - lastTime) / CLOCKS_PER_SEC;
		//lastTime = clock();
		auto start = std::chrono::high_resolution_clock::now();

		float deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(start - stop)).count() / 1000000.0f;
		stop = std::chrono::high_resolution_clock::now();

		float augmentedDeltaTime = deltaTime;
		if (augmentedDeltaTime > 1.f / 5) { augmentedDeltaTime = 1.f / 5; }
	
	#pragma endregion

	#pragma region imgui
		#if REMOVE_IMGUI == 0
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		#endif
	#pragma endregion

	#pragma region game logic

		if (!gameLogic(augmentedDeltaTime))
		{
			closeGame();
			return 0;
		}

	#pragma endregion


	#pragma region fullscreen 

		if (platform::isFocused() && currentFullScreen != fullScreen)
		{
			static int lastW = w;
			static int lastH = w;
			static int lastPosX = 0;
			static int lastPosY = 0;

			if (fullScreen)
			{
				lastW = w;
				lastH = h;

				//glfwWindowHint(GLFW_DECORATED, NULL); // Remove the border and titlebar..  
				glfwGetWindowPos(wind, &lastPosX, &lastPosY);


				//auto monitor = glfwGetPrimaryMonitor();
				auto monitor = getCurrentMonitor(wind);


				const GLFWvidmode* mode = glfwGetVideoMode(monitor);

				// switch to full screen
				glfwSetWindowMonitor(wind, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

				currentFullScreen = 1;

			}
			else
			{
				//glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // 
				glfwSetWindowMonitor(wind, nullptr, lastPosX, lastPosY, lastW, lastH, 0);

				currentFullScreen = 0;
			}

		}

	#pragma endregion

	#pragma region reset flags

		mouseMovedFlag = 0;
		platform::internal::updateAllButtons(deltaTime);
		platform::internal::resetTypedInput();

	#pragma endregion

	#pragma region window stuff


		PL::Profiler pl;
		pl.start();

		#pragma region imgui
				#if REMOVE_IMGUI == 0
				ImGui::Render();
				int display_w, display_h;
				glfwGetFramebufferSize(wind, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

				// Update and Render additional Platform Windows
				// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
				//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
				if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					GLFWwindow* backup_current_context = glfwGetCurrentContext();
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
					glfwMakeContextCurrent(backup_current_context);
				}
			#endif
		#pragma endregion

		pl.end();
		//if (pl.rezult.timeSeconds * 1000 > (17))
		//{
		//	std::cout << "IMGUI PROBLEM! " << pl.rezult.timeSeconds * 1000 << "ms!!!!\n";
		//}

		pl.start();
		glfwSwapBuffers(wind);
		pl.end();

		//if (pl.rezult.timeSeconds * 1000 > (17))
		//{
		//	std::cout << "SWAP BUFFERS PROBLEM! " << pl.rezult.timeSeconds * 1000 << "ms!!!!\n";
		//}
		

		glfwPollEvents();

	#pragma endregion

	}

	closeGame();

	//if you want the console to stay after closing the window
	//std::cin.clear();
	//std::cin.get();
}