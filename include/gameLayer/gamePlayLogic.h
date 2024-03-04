#pragma once
#include <gl2d/gl2d.h>
#include "rendering/renderer.h"
#include "blocksLoader.h"
#include <structure.h>

struct ProgramData
{
	gl2d::Renderer2D renderer2d;
	Renderer renderer;
	GyzmosRenderer gyzmosRenderer;
	PointDebugRenderer pointDebugRenderer;
	BlocksLoader blocksLoader;
	

	gl2d::Font font;
	gl2d::Texture texture;
	gl2d::Texture uiTexture;
	gl2d::Texture numbersTexture;
	gl2d::Texture causticsTexture;
	gl2d::Texture dudv;
	gl2d::Texture dudvNormal;
	gl2d::TextureAtlas uiAtlas{6, 1};
	int currentFps = 0;
};

struct PlayerData 
{
	int chunkDistance = 10; //remove this from here
	glm::dvec3 position = {};
	//...
};

bool initGameplay(ProgramData &programData, const char *c);

bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData);

void closeGameLogic();
