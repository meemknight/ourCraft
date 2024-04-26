#pragma once
#include <gl2d/gl2d.h>
#include <glui/glui.h>
#include "rendering/renderer.h"
#include "blocksLoader.h"
#include <structure.h>
#include <rendering/model.h>

struct ProgramData
{
	gl2d::Renderer2D renderer2d;
	glui::RendererUi menuRenderer;
	Renderer renderer;
	GyzmosRenderer gyzmosRenderer;
	PointDebugRenderer pointDebugRenderer;
	BlocksLoader blocksLoader;
	ModelsManager modelsManager;

	gl2d::Font font;
	gl2d::Texture uiTexture;
	gl2d::Texture buttonTexture;
	gl2d::Texture numbersTexture;
	gl2d::Texture causticsTexture;
	gl2d::Texture dudv;
	gl2d::Texture dudvNormal;
	gl2d::TextureAtlas uiAtlas{6, 1};
	int currentFps = 0;
};



bool initGameplay(ProgramData &programData, const char *c);

bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData);

void closeGameLogic();
