#pragma once
#include <glm/vec2.hpp>

bool initGame();
bool gameLogic(float deltaTime);
void closeGame();

namespace platform
{
	///sets the mouse pos relative to the window's drawing area
	void setRelMousePosition(int x, int y);

	bool isFullScreen();
	void setFullScreen(bool f);

//gets the drawing region sizes
	glm::ivec2 getWindowSize();

	inline int getWindowSizeX() { return getWindowSize().x; }
	inline int getWindowSizeY() { return getWindowSize().y; }

///gets the mouse pos relative to the window's drawing area
	glm::ivec2 getRelMousePosition();

	void showMouse(bool show);
	bool isFocused();
	bool mouseMoved();

	bool writeEntireFile(const char *name, void *buffer, size_t size);
	bool readEntireFile(const char *name, void *buffer, size_t size);


};

