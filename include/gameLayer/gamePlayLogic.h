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

	Profiler GPUProfiler;

	gl2d::Texture numbersTexture;
	gl2d::Texture causticsTexture;
	gl2d::Texture dudv;
	gl2d::Texture dudvNormal;
	int currentFps = 0;

	struct OtherSettings
	{

		int viewDistance = 15;

	}otherSettings;
};


bool initGameplay(ProgramData &programData, const char *c);

bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData);

void closeGameLogic();
