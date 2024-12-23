#pragma once
#include <gl2d/gl2d.h>
#include <glui/glui.h>
#include <profiler.h>
#include "rendering/renderer.h"
#include "blocksLoader.h"
#include <structure.h>
#include <rendering/model.h>
#include <rendering/UiEngine.h>

struct ProgramData
{

	UiENgine ui;

	Renderer renderer;
	GyzmosRenderer gyzmosRenderer;
	PointDebugRenderer pointDebugRenderer;
	BlocksLoader blocksLoader;
	ModelsManager modelsManager;
	SkyBoxLoaderAndDrawer skyBoxLoaderAndDrawer;
	SunRenderer sunRenderer;

	Profiler GPUProfiler;

	gl2d::Texture numbersTexture;
	gl2d::Texture causticsTexture;
	gl2d::Texture dudv;
	gl2d::Texture dudvNormal;
	gl2d::Texture aoTexture;
	gl2d::Texture brdfTexture;
	gl2d::Texture defaultCover;
	gl2d::Texture crackTexture;
	gl2d::Texture lensDirtTexture;
	gl2d::Texture hitDirtTexture;
	gl2d::Texture waterDirtTexture;
	gl2d::Texture heartsTexture;
	gl2d::TextureAtlasPadding heartsAtlas;
	std::vector<gl2d::Texture> lensFlare;
	float maxFlareSize = 0;
	int currentFps = 0;
	bool showImgui = 1;


};


bool initGameplay(ProgramData &programData, const char *c);

bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData);

void closeGameLogic();
