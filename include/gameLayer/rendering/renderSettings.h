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
	int workerThreadsForBaking = 2;
	int lodStrength = 1;

	glm::vec3 waterColor = (glm::vec3(6, 42, 52) / 255.f);
	glm::vec3 underWaterColor = glm::vec3(0, 17, 25) / 255.f;

	float underwaterDarkenStrength = 0.94;
	float underwaterDarkenDistance = 29;
	float fogGradientUnderWater = 1.9;
	
	float bloomTresshold = 0.5;
	float bloomMultiplier = 0.5;

	float exposure = 0;
	float fogGradient = 16.f;

	void normalize();

	// Equality operator
	bool operator==(const ShadingSettings &other) const
	{
		return viewDistance == other.viewDistance &&
			tonemapper == other.tonemapper &&
			shadows == other.shadows &&
			waterColor == other.waterColor &&
			underWaterColor == other.underWaterColor &&
			underwaterDarkenStrength == other.underwaterDarkenStrength &&
			underwaterDarkenDistance == other.underwaterDarkenDistance &&
			fogGradientUnderWater == other.fogGradientUnderWater &&
			exposure == other.exposure &&
			lodStrength == other.lodStrength &&
			workerThreadsForBaking == other.workerThreadsForBaking &&
			bloomTresshold == other.bloomTresshold &&
			bloomMultiplier == other.bloomMultiplier &&
			waterType == other.waterType;
		
	}

	// Inequality operator
	bool operator!=(const ShadingSettings &other) const
	{
		return !(*this == other);
	}
};

ShadingSettings &getShadingSettings();

void saveShadingSettings();

void loadShadingSettings();