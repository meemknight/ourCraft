#pragma once

#include "glui/glui.h"
#include <filesystem>

struct ProgramData;


void displayWorldSelectorMenuButton(ProgramData &programData);

void displayWorldSelectorMenu(ProgramData &programData);

void displayRenderSettingsMenuButton(ProgramData &programData);
void displayRenderSettingsMenu(ProgramData &programData);

void displaySettingsMenuButton(ProgramData &programData);
void displaySettingsMenu(ProgramData &programData);

bool shouldReloadTexturePacks();

std::vector<std::filesystem::path> getUsedTexturePacksAndResetFlag();

void displayTexturePacksSettingsMenuButton(ProgramData &programData);

void displayTexturePacksSettingsMenu(ProgramData &programData);

void displaySkinSelectorMenu(ProgramData &programData);

void displaySkinSelectorMenuButton(ProgramData &programData);

void displayVolumeMenu(ProgramData &programData);

void displayVolumeMenuButton(ProgramData &programData);


std::string getSkinName();

struct ShadingSettings
{

	int viewDistance = 15;
	int tonemapper = 0;
	int shadows = 0;
	int waterType = 1;
	int workerThreadsForBaking = 2; //MOVE TODO
	int lodStrength = 1; //MOVE TODO
	int PBR = 1;
	int maxLights = 40;
	int useLights = 1;
	float lightsStrength = 1.f;
	bool FXAA = 1;

	glm::vec3 waterColor = (glm::vec3(6, 42, 52) / 255.f);
	glm::vec3 underWaterColor = glm::vec3(0, 17, 25) / 255.f;

	float underwaterDarkenStrength = 0.94;
	float underwaterDarkenDistance = 29;
	float fogGradientUnderWater = 1.9;
	
	float bloomTresshold = 0.5;
	float bloomMultiplier = 0.5;

	float exposure = 0;
	float fogGradient = 16.f;
	int bloom = 1;

	int SSR = 1;

	float toneMapSaturation = 1;
	float toneMapVibrance = 1;
	float toneMapGamma = 1;
	float toneMapShadowBoost = 0;
	float toneMapHighlightBoost = 0;
	float vignette = 0.15;
	glm::vec3 toneMapLift = glm::vec3(0.5);
	glm::vec3 toneMapGain = glm::vec3(0.5);

	void normalize();

	// Equality operator
	bool operator==(const ShadingSettings &other) const
	{
		return std::memcmp(this, &other, sizeof(ShadingSettings)) == 0;
	}

	// Inequality operator
	bool operator!=(const ShadingSettings &other) const
	{
		return !(*this == other);
	}

	std::string formatIntoGLSLcode();
};

ShadingSettings &getShadingSettings();

bool checkIfShadingSettingsChangedForShaderReloads();

void saveShadingSettings();

void loadShadingSettings();